#include "UDPMethod.h"
#include "IOService.h"
#include "Options.h"
#include "Error.h"
#include "Utils.h"
#include "Measurement.h"
#include "DataReader.h"
#include "IPList.h"

#include <boost/bind.hpp>

using boost::asio::ip::udp;

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
		uint64_t timeSent = reader.read64(pos);
		uint64_t myTime = getMicroTime();
		auto data = std::make_shared<Data>();
		data->append(timeSent);
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
				uint64_t myTime = getMicroTime();
				if(it->second == timeSent)
				{
					if(myTime > timeSent)
					{
						Measurement::addMeasurement(address, "udp", myTime - timeSent);
						IPtoTime.erase(it);
					}
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
	IPtoTime.clear();
	for(auto & address : IPList::getIPs("opoznienia"))
	{
		auto data = std::make_shared<Data>();
		uint64_t time = getMicroTime();
		data->append(time);
		IPtoTime[address] = time;
		sendData(udp::endpoint(address, Options::PortUDP()), data);
	}
}
