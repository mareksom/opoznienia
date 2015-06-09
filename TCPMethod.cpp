#include "TCPMethod.h"
#include "IOService.h"
#include "Utils.h"
#include "Measurement.h"
#include "IPList.h"
#include "Error.h"

using boost::asio::ip::tcp;

void TCPMethod::runMeasurement()
{
	sockets.clear();
	auto addressList = IPList::getIPs("ssh");
	sockets.reserve(addressList.size());
	for(unsigned i = 0; i < addressList.size(); i++)
	{
		sockets.emplace_back(IO);
		connectTo(addressList[i], sockets[i]);
	}
}

void TCPMethod::connectTo(boost::asio::ip::address_v4 address, tcp::socket & socket)
{
	uint64_t startTime = getMicroTime();
	socket.async_connect(
		tcp::endpoint(address, 22),
		[address, startTime] (const boost::system::error_code & error) {
			if(error)
				connectionerr << "TCPMethod::connectTo(" << address << "): " << error.message() << "\n";
			else
			{
				uint64_t endTime = getMicroTime();
				if(startTime < endTime)
					Measurement::addMeasurement(address, "tcp", endTime - startTime);
			}
		}
	);
}
