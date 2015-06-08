#include "IPList.h"

#include <set>
#include <map>
#include <iostream>

using boost::asio::ip::address_v4;

namespace {

std::map<
	std::string,
	std::set<address_v4>
> ipList;

} // namespace

namespace IPList {

void addNewIP(address_v4 ip, const std::string & service)
{
	std::cout << "Got new IP: " << ip << " for service: " << service << "\n";
	ipList[service].insert(ip);
}

void removeIP(address_v4 ip, const std::string & service)
{
	auto serviceList = ipList[service];
	serviceList.erase(ip);
	if(serviceList.empty())
		ipList.erase(ipList.find(service));
}

std::vector<address_v4> getIPs()
{
	std::set<address_v4> IPSet;
	for(auto & it : ipList)
		for(auto & address : it.second)
			IPSet.insert(address);
	return std::vector<address_v4>(IPSet.begin(), IPSet.end());
}

std::vector<std::string> getServices(address_v4 ip)
{
	std::vector<std::string> ret;
	for(auto & it : ipList)
		if(it.second.find(ip) != it.second.end())
			ret.push_back(it.first);
	return ret;
}

} // namespace IPList
