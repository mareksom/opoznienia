#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_

#include "Method.h"

#include <boost/asio.hpp>
#include <vector>

class Timeout
{
public:
	Timeout();
	void addMethod(Method * method);

private:
	void startTimer();
	void handleTimeout(const boost::system::error_code & error);

	boost::asio::deadline_timer timer;
	std::vector<Method*> methods;
};

#endif
