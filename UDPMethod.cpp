#include "UDPMethod.h"
#include "IOService.h"
#include "Options.h"
#include "Error.h"
#include "Utils.h"
#include "Measurement.h"
#include "DataReader.h"
#include "IPList.h"

#include <boost/bind.hpp>
#include <iostream>

using boost::asio::ip::udp;
using std::cout;
using std::endl;

UDPMethod::UDPMethod() :
	socket(IO, udp::endpoint(udp::v4(), Options::PortUDP())),
	buffer(16)
{
	startListening();
}

void UDPMethod::startListening()
{
	socket.async_receive_from(
		boost::asio::buffer(buffer),
		rcv_endpoint,
		boost::bind(
			&UDPMethod::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void UDPMethod::handleReceive(const boost::system::error_code & error, std::size_t size)
{
	if(error)
		err << "UDPMethod::handleReceive: " << error.message() << "\n";
	else if(size == 8) /* Request */
	{
		DataReader reader(buffer.data(), size);
		unsigned pos = 0;
		uint64_t timeReceived = reader.read64(pos);
		uint64_t myTime = getMicroTime();
		auto data = std::make_shared<Data>();
		data->append(timeReceived);
		data->append(myTime);
		sendData(rcv_endpoint, data);
	}
	else if(size == 16) /* Answer */
	{
		if(rcv_endpoint.address().is_v4())
		{
			boost::asio::ip::address_v4 address = rcv_endpoint.address().to_v4();
			auto it = IPtoTime.find(address);
			if(it != IPtoTime.end())
			{
				DataReader reader(buffer.data(), size);
				unsigned pos = 0;
				uint64_t timeSent = reader.read64(pos);
				uint64_t timeReSent = reader.read64(pos);
				if(it->second == timeSent && timeReSent > timeSent)
				{
					Measurement::addMeasurement(address, "udp", timeReSent - timeSent);
					IPtoTime.erase(it);
				}
			}
		}
	}
	startListening();
}

void UDPMethod::sendData(udp::endpoint where, std::shared_ptr<Data> data)
{
	socket.async_send_to(
		boost::asio::buffer(data->getPtr(), data->getLength()),
		where,
		[data] (const boost::system::error_code & error, std::size_t) {
			if(error)
				err << "UDPMethod::sendData: " << error.message() << "\n";
		}
	);
}

void UDPMethod::runMeasurement()
{
	cout << "runMeasurement()\n";
	IPtoTime.clear();
	for(auto & address : IPList::getIPs())
	{
		cout << "> address: " << address << "\n";
		auto data = std::make_shared<Data>();
		uint64_t time = getMicroTime();
		data->append(time);
		IPtoTime[address] = time;
		sendData(udp::endpoint(address, Options::PortUDP()), data);
	}
}


using boost::asio::error::operation_aborted;

// UDPServer --------------------------------

UDPServer::UDPServer() : socket(IO, udp::endpoint(udp::v4(), Options::PortUDP())), buffer(8)
{
	startReceive();
}

void UDPServer::startReceive()
{
	socket.async_receive_from(
		boost::asio::buffer(buffer), remote_endpoint,
		boost::bind(&UDPServer::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void UDPServer::handleReceive(const boost::system::error_code & error, std::size_t size)
{
	if(error)
		err << "UDPServer.receive: " << error.message() << "\n";
	else if(size != 8)
		err << "UDPServer.receive: Malformed packet\n";
	else
	{
		uint64_t times[2] = {
			/* Client time */
			ntohll(*((uint64_t*) buffer.data())),
			/* Server time */
			getMicroTime(),
		};
		cout << "UDPServer.receive: Got packet: times = {"
			<< times[0] << ", " << times[1] << "}" << endl;
		times[0] = htonll(times[0]);
		times[1] = htonll(times[1]);
		socket.async_send_to(
			boost::asio::buffer(times, 16),
			remote_endpoint,
			boost::bind(&UDPServer::handleSend, this,
				boost::asio::placeholders::error
			)
		);
	}
	startReceive();
}

void UDPServer::handleSend(const boost::system::error_code & error)
{
	if(error)
		err << "UDPServer.send: " << error.message() << "\n";
}

// UDPClient -------------------------

UDPClient::UDPClient(udp::endpoint remote_endpoint) :
	socket(IO, udp::v4()),
	remote_endpoint(remote_endpoint),
	buffer(16),
	timer(IO)
{
	startMeasurement();
}

void UDPClient::setTimerTime()
{
	timer.expires_from_now(
		boost::posix_time::milliseconds(Options::DeltaMeasurement())
	);
}

void UDPClient::startMeasurement()
{
	setTimerTime();
	timer.async_wait(boost::bind(
		&UDPClient::handleStartOfMeasurement, this,
		boost::asio::placeholders::error
	));
}

void UDPClient::handleStartOfMeasurement(const boost::system::error_code & error)
{
	if(error)
		err << "UDPClient.startOfMeasurement: " << error.message() << "\n";
	else
	{
		time_sent = htonll(getMicroTime());
		socket.async_send_to(
			boost::asio::buffer(&time_sent, 8),
			remote_endpoint,
			boost::bind(&UDPClient::handleMessageSent, this,
				boost::asio::placeholders::error
			)
		);
	}
}

void UDPClient::handleMessageSent(const boost::system::error_code & error)
{
	if(error)
		err << "UDPClient.messageSent: " << error << "\n";
	else
	{
		setTimerTime();
		timer.async_wait(boost::bind(
			&UDPClient::handleTimeout, this,
			boost::asio::placeholders::error
		));
		socket.async_receive_from(
			boost::asio::buffer(buffer), remote_endpoint,
			boost::bind(&UDPClient::handleResponseReceived, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}
}

void UDPClient::handleResponseReceived(const boost::system::error_code & error, std::size_t size)
{
	if(error == operation_aborted)
		;
	else if(error)
		err << "UDPClient.handleResponseReceived " << error << "\n";
	else if(size != 16)
		err << "UDPClient.handleResponseReceived: Malformed packet\n";
	else
	{
		uint64_t * times = (uint64_t*) buffer.data();
		times[0] = ntohll(times[0]);
		times[1] = ntohll(times[1]);
		time_sent = ntohll(time_sent);
		if(times[0] != time_sent)
			err << "UDPClient.handleResponseReceived: First time don't match\n";
		else
		{
			cout << "Opoznienie: " << times[1] - times[0] << " microseconds\n";
		}
		timer.cancel();
		startMeasurement();
	}
}

void UDPClient::handleTimeout(const boost::system::error_code & error)
{
	if(error == operation_aborted)
		;
	else if(error)
		err << "UDPClient.handleTimeout: " << error << "\n";
	else
	{
		cout << "Timeout...\n";
		socket.cancel();
		startMeasurement();
	}
}
