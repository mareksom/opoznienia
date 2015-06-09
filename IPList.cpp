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

std::set<address_v4> newlyAddedIPs;

} // namespace

namespace IPList {

void addNewIP(address_v4 ip, const std::string & service)
{
	newlyAddedIPs.insert(ip);
	ipList[service].insert(ip);
}

void removeIP(address_v4 ip)
{
	for(auto & service : ipList)
	{
		auto it = service.second.find(ip);
		if(it != service.second.end())
			service.second.erase(it);
	}
}

void removeTrash()
{
	for(const auto & address : getIPs())
		if(newlyAddedIPs.find(address) == newlyAddedIPs.end())
			removeIP(address);
	newlyAddedIPs.clear();
}

std::vector<address_v4> getIPs()
{
	std::set<address_v4> IPSet;
	for(auto & it : ipList)
		for(auto & address : it.second)
			IPSet.insert(address);
	return std::vector<address_v4>(IPSet.begin(), IPSet.end());
}

std::vector<address_v4> getIPs(const std::string & service)
{
	auto it = ipList.find(service);
	if(it == ipList.end())
		return std::vector<address_v4>();
	return std::vector<address_v4>(it->second.begin(), it->second.end());
}

} // namespace IPList
