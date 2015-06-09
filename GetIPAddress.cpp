#include "GetIPAddress.h"
#include "Error.h"

#include <sys/types.h>
#include <ifaddrs.h>
#include <string.h>
#include <errno.h>

#include <iostream>

using std::cout;

typedef std::vector< std::pair< std::string, boost::asio::ip::address > > AddressList;

namespace
{

void readAddressList(struct ifaddrs * addr, AddressList & result)
{
	if(addr == NULL)
		return;
	
	if((addr->ifa_addr->sa_family == AF_INET)
		and !(addr->ifa_flags & IFF_LOOPBACK)
		and (addr->ifa_flags & IFF_RUNNING)
		and (addr->ifa_flags & IFF_UP))
	{
		struct sockaddr_in * sa = (struct sockaddr_in*) addr->ifa_addr;
		result.emplace_back(
			std::string(addr->ifa_name),
			boost::asio::ip::address(
				boost::asio::ip::address_v4(
					ntohl(sa->sin_addr.s_addr)
			)
			)
		);
	}
	
	readAddressList(addr->ifa_next, result);
}

} // namespace

AddressList GetIPAddresses()
{
	struct ifaddrs * addr;
	if(getifaddrs(&addr) == -1)
		connectionerr << "GetIPAddresses::getifaddrs: " << strerror(errno) << errend;
	AddressList result;
	readAddressList(addr, result);
	freeifaddrs(addr);
	return result;
}
