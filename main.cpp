#include "Options.h"
#include "Error.h"
#include "Dns.h"
#include "IOService.h"
#include "UDPMethod.h"
#include "ICMPMethod.h"
#include "TCPMethod.h"
#include "Timeout.h"
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
	UDPMethod udpMethod;
	ICMPMethod icmpMethod;
	TCPMethod tcpMethod;

	Timeout timeout;
	timeout.addMethod(&udpMethod);
	timeout.addMethod(&icmpMethod);
	timeout.addMethod(&tcpMethod);

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
