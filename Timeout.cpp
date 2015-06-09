#include "Timeout.h"
#include "IOService.h"
#include "Options.h"
#include "Error.h"
#include "Measurement.h"

#include <boost/bind.hpp>

Timeout::Timeout() : timer(IO)
{
	startTimer();
}

void Timeout::addMethod(Method * method)
{
	methods.push_back(method);
}

void Timeout::startTimer()
{
	timer.expires_from_now(
		boost::posix_time::milliseconds(Options::DeltaMeasurement())
	);
	timer.async_wait(boost::bind(
		&Timeout::handleTimeout, this,
		boost::asio::placeholders::error
	));
}

void Timeout::handleTimeout(const boost::system::error_code & error)
{
	if(error)
		connectionerr << "Timeout::handleTimeout: " << error.message() << "\n";
	else
	{
		for(Method * method : methods)
			method->runMeasurement();
	}
	Measurement::skipMeasurements();
	startTimer();
}
