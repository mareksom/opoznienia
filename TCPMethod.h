#ifndef _TCPMETHOD_H_
#define _TCPMETHOD_H_

#include "Method.h"

#include <boost/asio.hpp>

class TCPMethod : public Method
{
public:
	virtual void runMeasurement();

private:
	void connectTo(boost::asio::ip::address_v4 address);
};

#endif
