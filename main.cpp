#include "Options.h"
#include "Error.h"
#include "Dns.h"
#include "IOService.h"
#include "UDPMethod.h"
#include "ICMPMethod.h"
#include "MDns.h"
#include "Utils.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <functional>

using std::cout;

void program()
{
	MDnsDiscoverer mDnsDiscoverer;
	//UDPMethod udpMethod;
	ICMPMethod icmpMethod;

	boost::asio::deadline_timer timer(IO);

	std::function<void(const boost::system::error_code&)> timeout = [&] (const boost::system::error_code & error) {
		//udpMethod.runMeasurement();
		icmpMethod.runMeasurement();
		timer.expires_from_now(
			boost::posix_time::milliseconds(Options::DeltaMeasurement())
		);
		timer.async_wait(timeout);
	};

	timer.expires_from_now(
		boost::posix_time::milliseconds(Options::DeltaMeasurement())
	);
	timer.async_wait(timeout);

	cout << "przed run\n";
	IO.run();
	cout << "po run\n";
}

int main(int argc, char ** argv)
{
	try
	{
		Options::InitArguments(argc - 1, argv + 1);
		Options::PrintOptions();
		program();
	}
	catch(const ReturnException & re)
	{
		return re.getExitStatus();
	}
  return 0;
}
