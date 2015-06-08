#include "TCPMethod.h"
#include "IOService.h"
#include "Utils.h"
#include "Measurement.h"
#include "IPList.h"
#include "Error.h"

#include <memory>

using boost::asio::ip::tcp;

void TCPMethod::runMeasurement()
{
	for(auto & address : IPList::getIPs())
		connectTo(address);
}

void TCPMethod::connectTo(boost::asio::ip::address_v4 address)
{
	std::shared_ptr<tcp::socket> socket = std::make_shared<tcp::socket>(IO);
	uint64_t startTime = getMicroTime();
	socket->async_connect(
		tcp::endpoint(address, 22),
		[socket, address, startTime] (const boost::system::error_code & error) {
			if(error)
				err << "TCPMethod::connectTo(" << address << "): " << error.message() << "\n";
			else
			{
				uint64_t endTime = getMicroTime();
				if(startTime < endTime)
					Measurement::addMeasurement(address, "tcp", endTime - startTime);
			}
		}
	);
}
