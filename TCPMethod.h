#ifndef _TCPMETHOD_H_
#define _TCPMETHOD_H_

#include <boost/asio.hpp>

class TCPMethod
{
public:
	void runMeasurement();

private:
	void connectTo(boost::asio::ip::address_v4 address);
};

#endif
