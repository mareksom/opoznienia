#ifndef _TCPMETHOD_H_
#define _TCPMETHOD_H_

#include "Method.h"

#include <boost/asio.hpp>
#include <vector>

class TCPMethod : public Method
{
public:
	virtual void runMeasurement();

private:
	void connectTo(boost::asio::ip::address_v4 address, boost::asio::ip::tcp::socket & socket);
	std::vector<boost::asio::ip::tcp::socket> sockets;
};

#endif
