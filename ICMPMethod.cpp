#include "ICMPMethod.h"
#include "IOService.h"
#include "Utils.h"
#include "Error.h"
#include "Options.h"
#include "Data.h"
#include "DataReader.h"
#include "IPList.h"
#include "IPv4Header.h"
#include "Measurement.h"

#include <boost/bind.hpp>
#include <iostream>

using boost::asio::ip::icmp;
using std::cout;

ICMPMethod::ICMPMethod() :
	sequenceNumber(0),
	socket(IO, icmp::v4()),
	buffer(120),
	timer(IO)
{
	startListening();
}

void ICMPMethod::startListening()
{
	socket.async_receive_from(
		boost::asio::buffer(buffer),
		rcv_endpoint,
		boost::bind(&ICMPMethod::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void ICMPMethod::handleReceive(const boost::system::error_code & error, std::size_t size)
{
	if(error)
		err << "ICMPMethod::handleReceive: " << error.message() << "\n";
	else
	{
		try
		{
			IPv4Header header(Data(buffer.data(), size));

			Data data = header.getInnerData();
			PingPacket packet(DataReader(data.getPtr(), data.getLength()), 0);

			if(rcv_endpoint.address().is_v4())
			{
				auto address = rcv_endpoint.address().to_v4();
				auto it = IPtoTime.find(address);
				if(it != IPtoTime.end())
				{
					if(packet.getSequence() == sequenceNumber)
					{
						uint64_t sendTime = it->second;
						uint64_t myTime = getMicroTime();
						if(sendTime < myTime)
						{
							Measurement::addMeasurement(address, "icmp", myTime - sendTime);
							IPtoTime.erase(it);
						}
					}
				}
			}
		}
		catch(MalformedPacket & mp)
		{
			// skip Malformed Packet
		}
	}
	startListening();
}

void ICMPMethod::sendData(icmp::endpoint where, std::shared_ptr<Data> data)
{
	socket.async_send_to(
		boost::asio::buffer(data->getPtr(), data->getLength()),
		where,
		[data] (const boost::system::error_code & error, std::size_t) {
			if(error)
				err << "ICMPMethod::sendData: " << error.message() << "\n";
		}
	);
}

void ICMPMethod::runMeasurement()
{
	sequenceNumber++;
	IPtoTime.clear();
	for(auto & address : IPList::getIPs())
	{
		PingPacket echo;
		echo.setType(PingPacket::ECHO_MESSAGE);
		echo.setCode(0);
		echo.setID(0x13);
		echo.setSequence(sequenceNumber);
		echo.setIndex(347247);
		echo.setGroup(3);
		std::shared_ptr<Data> data = std::make_shared<Data>(echo.getData());
		IPtoTime[address] = getMicroTime();
		sendData(icmp::endpoint(address, 0), data);
	}
}
