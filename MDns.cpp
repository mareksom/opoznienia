#include "MDns.h"
#include "IOService.h"
#include "Error.h"
#include "Utils.h"
#include "GetIPAddress.h"
#include "Options.h"
#include "IPList.h"

#include <boost/bind.hpp>
#include <iostream>

using std::cout;
using boost::asio::ip::udp;
using boost::asio::error::operation_aborted;

MDnsDiscoverer::MDnsDiscoverer() :
	timer(IO),
	name(boost::asio::ip::host_name()),
	nameNumber(0),
	responseBuffer(1024),
	socket(IO, udp::v4()),
	multicast(boost::asio::ip::address::from_string("224.0.0.251"), 5353)
{
	socket.set_option(boost::asio::socket_base::reuse_address(true));
	socket.set_option(boost::asio::ip::multicast::join_group(multicast.address()));
	socket.set_option(boost::asio::ip::multicast::hops(255));
	socket.bind(udp::endpoint(udp::v4(), 5353));

	determineName();
}

/* Timer */

void MDnsDiscoverer::setTimer(int milliseconds)
{
	timer.cancel();
	timer.expires_from_now(
		boost::posix_time::milliseconds(milliseconds)
	);
}

/* Instance name */

std::string MDnsDiscoverer::getName() const
{
	if(nameNumber == 1)
		return name;
	return name + " (" + std::to_string(nameNumber) + ")";
}

void MDnsDiscoverer::determineName()
{
	if(nameNumber != 0)
		setTimer(1000);
	else
		setTimer(randomInt(0, 250));
	timer.async_wait(boost::bind(
		&MDnsDiscoverer::startProbes, this,
		boost::asio::placeholders::error
	));
}

void MDnsDiscoverer::startProbes(const boost::system::error_code & error)
{
	if(error)
		connectionerr << "MDnsDiscoverer::startProbes: " << error.message() << errend;
	else
	{
		nameNumber++;
		triesLeft = 3;
		nextTry();
	}
}

void MDnsDiscoverer::nextTry()
{
	if(triesLeft == 0)
	{
		cout << "This instance will be named: " << getName() << "\n";
		startDiscoveryServer();
		return;
	}
	triesLeft--;

	DNS_Packet packet;
	DNS_Question question;
	question.setUnicast(true);
	question.setQType(DNS_QType::ANY);
	question.setQClass(DNS_QClass::IN);
	question.setQName(DNS_Name(getName(), "local"));
	packet.addQD(question);

	auto data = std::make_shared<Data>(packet.getData());

	socket.async_send_to(
		boost::asio::buffer(data->getPtr(), data->getLength()),
		multicast,
		boost::bind(&MDnsDiscoverer::handleProbeSent, this,
			boost::asio::placeholders::error,
			data
		)
	);
}

void MDnsDiscoverer::handleProbeSent(const boost::system::error_code & error, std::shared_ptr<Data> data)
{
	if(error == operation_aborted)
		nextTry();
	else if(error)
		connectionerr << "MDnsDiscoverer::handleProbeSent: " << error.message() << errend;
	else
	{
		setTimer(250);
		timer.async_wait(boost::bind(
			&MDnsDiscoverer::handleProbeTimeout, this,
			boost::asio::placeholders::error
		));
		receiveProbe();
	}
}

void MDnsDiscoverer::receiveProbe()
{
	socket.async_receive_from(
		boost::asio::buffer(responseBuffer),
		rcv_endpoint,
		boost::bind(&MDnsDiscoverer::handleProbeResponse, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void MDnsDiscoverer::handleProbeResponse(const boost::system::error_code & error, std::size_t size)
{
	if(error == operation_aborted)
		nextTry();
	else if(error)
		connectionerr << "MDnsDiscoverer::handleProbeResponse: " << error.message() << errend;
	else
	{
		try
		{
			DNS_Packet packet(DataReader(responseBuffer.data(), size), 0);
			if(packet.getQR()) /* If it is a response */
			{
				DNS_Name myName(getName(), "local");
				for(auto & record : packet.getAN())
				{
					if(record.getName() == myName)
					{
						/* Name is already taken */
						determineName();
						return;
					}
				}
			}
		}
		catch(MalformedPacket & mp) { }

		receiveProbe();
	}
}

void MDnsDiscoverer::handleProbeTimeout(const boost::system::error_code & error)
{
	if(error == operation_aborted)
		;
	else if(error)
		connectionerr << "MDnsDiscoverer::handleProbeTimeout: " << error.message() << errend;
	else
		socket.cancel();
}

/* Discovery */

void MDnsDiscoverer::startDiscoveryServer()
{
	for(auto & adr : GetIPAddresses())
		myIPs.emplace_back(adr.second);

	cout << "Using those IP addresses: ";
	for(int i = 0; i < (int) myIPs.size(); i++)
	{
		if(i)
			cout << ", ";
		cout << myIPs[i];
	}
	cout << "\n";

	listenForResponse();
	runDiscovery();
}

void MDnsDiscoverer::listenForResponse()
{
	socket.async_receive_from(
		boost::asio::buffer(responseBuffer),
		rcv_endpoint,
		boost::bind(&MDnsDiscoverer::handleResponse, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void MDnsDiscoverer::handleResponse(const boost::system::error_code & error, std::size_t size)
{
	responseBufferSize = size;

	/* Check if it's my packet */
	for(auto & address : myIPs)
		if(rcv_endpoint.address() == address)
			return listenForResponse();

	try
	{
		DNS_Packet packet(DataReader(responseBuffer.data(), size), 0);
		if(packet.getQR()) /* It's a response */
			readPacketResponse(packet);
		else
			readPacketQuery(packet);
	}
	catch(MalformedPacket & mp)
	{
		// ignore packet
	}

	listenForResponse();
}

void MDnsDiscoverer::readPacketResponse(const DNS_Packet & packet)
{
	for(auto & an : packet.getAN())
		readPacketAnswer(an);
}

void MDnsDiscoverer::readPacketQuery(const DNS_Packet & packet)
{
	for(auto & question : packet.getQD())
		readPacketQuestion(question);
}

void MDnsDiscoverer::readPacketQuestion(const DNS_Question & question)
{
	if((question.getQType() == DNS_QType::ANY or question.getQType() == DNS_QType::A)
		and question.getQClass() == DNS_QClass::IN)
	{
		/* Answering probe questions */
		if(question.getQName() == DNS_Name(getName(), "local")
			or question.getQName() == DNS_Name(getName(), "_opoznienia", "_udp", "local")
			or question.getQName() == DNS_Name(getName(), "_ssh", "_tcp", "local"))
		{
			DNS_Packet packet;
			packet.setQR(true);
			packet.setAA(true);
			appendMyIPs(question.getQName(), packet);
			if(packet.getANCount())
				sendPacket(packet, question.getUnicast());
		}
	}
	else if(question.getQType() == DNS_QType::PTR and question.getQClass() == DNS_QClass::IN)
	{
		/* Answering PTR questions */
		if(question.getQName() == DNS_Name("_opoznienia", "_udp", "local")
			or (Options::ServerSSH() and question.getQName() == DNS_Name("_ssh", "_tcp", "local")))
		{
			DNS_Packet packet;
			packet.setQR(true);
			packet.setAA(true);
			DNS_RRecord ptrRecord;
			auto name = question.getQName();
			ptrRecord.setName(name);
			ptrRecord.setType(DNS_Type::PTR);
			ptrRecord.setClass(DNS_Class::IN);
			ptrRecord.setTTL(120);
			name.prepend(getName());
			ptrRecord.setData(name.getData());
			packet.addAN(ptrRecord);
			appendMyIPs(name, packet);
			sendPacket(packet, question.getUnicast());
		}
	}
}

void MDnsDiscoverer::readPacketAnswer(const DNS_RRecord & record)
{
	if(record.getType() == DNS_Type::PTR and record.getClass() == DNS_Class::IN)
	{
		if((record.getName() == DNS_Name("_opoznienia", "_udp", "local"))
			or (record.getName() == DNS_Name("_ssh", "_tcp", "local")))
		{
			DNS_Name name = record.getRDataName();
			if(name.empty())
				return; /* The PTR response must have a name */
			if(instanceToIP.insert(std::make_pair(
						name, boost::asio::ip::address_v4())).second)
			{
				/* Send question about the address */
				DNS_Packet packet;
				DNS_Question question;
				question.setQType(DNS_QType::A);
				question.setQClass(DNS_QClass::IN);
				question.setQName(record.getRDataName());
				packet.addQD(question);
				sendPacket(packet);
			}
		}
	}
	else if(record.getType() == DNS_Type::A and record.getClass() == DNS_Class::IN)
	{
		DNS_Name name = record.getName();
		Data rdata = record.getRData();
		if(rdata.getLength() != 4)
			return; /* The A response must contain an IPv4 address */
		DataReader reader(rdata.getPtr(), rdata.getLength());
		unsigned pos = 0;
		uint32_t ip = reader.read32(pos);
		auto it = instanceToIP.find(name);
		if(it == instanceToIP.end())
			return; /* I didn't ask for this address... */
		it->second = boost::asio::ip::address_v4(ip);
		if(name.endsWith(DNS_Name("_ssh", "_tcp", "local")))
			IPList::addNewIP(it->second, "ssh");
		else if(name.endsWith(DNS_Name("_opoznienia", "_udp", "local")))
			IPList::addNewIP(it->second, "opoznienia");
	}
}

void MDnsDiscoverer::appendMyIPs(const DNS_Name & name, DNS_Packet & packet)
{
	for(auto & address : myIPs)
	{
		if(!address.is_v4())
			continue;
		DNS_RRecord record;
		record.setName(name);
		record.setType(DNS_Type::A);
		record.setClass(DNS_Class::IN);
		record.setTTL(120); // two minutes
		Data data;
		data.append((uint32_t) address.to_v4().to_ulong());
		record.setData(data);
		packet.addAN(record);
	}
}

void MDnsDiscoverer::sendPacket(const DNS_Packet & packet, bool unicast)
{
	auto data = std::make_shared<Data>(packet.getData());
	auto handler = [data] (const boost::system::error_code & error, std::size_t) {
		if(error)
			connectionerr << "MDnsDiscoverer::sendPacket: " << error.message() << "\n";
	};

	socket.async_send_to(
		boost::asio::buffer(data->getPtr(), data->getLength()),
		unicast ? rcv_endpoint : multicast,
		handler
	);
}

void MDnsDiscoverer::runDiscovery()
{
	/* Set timer once again */
	setTimer(Options::DeltaSearch());
	timer.async_wait(boost::bind(
		&MDnsDiscoverer::handleDiscoveryTimeout, this,
		boost::asio::placeholders::error
	));

	/* Clear mappings */
	IPList::removeTrash();
	instanceToIP.clear();

	/* Send all the PTR questions */
	DNS_Packet packet;
	for(auto & service : {
			DNS_Name("_opoznienia", "_udp", "local"),
			DNS_Name("_ssh", "_tcp", "local"),
		})
	{
		DNS_Question question;
		question.setQName(service);
		question.setQType(DNS_QType::PTR);
		question.setQClass(DNS_QClass::IN);
		packet.addQD(question);
	}
	sendPacket(packet);
}

void MDnsDiscoverer::handleDiscoveryTimeout(const boost::system::error_code & error)
{
	if(error == operation_aborted)
		;
	else if(error)
		connectionerr << "MDnsDiscoverer::handleDiscoveryTimeout: " << error.message() << errend;
	else
		runDiscovery();
}
