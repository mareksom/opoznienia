#ifndef _IPLIST_H_
#define _IPLIST_H_

#include <boost/asio.hpp>
#include <vector>
#include <string>

namespace IPList {

void addNewIP(boost::asio::ip::address_v4 ip, const std::string & service);
void removeIP(boost::asio::ip::address_v4 ip, const std::string & service);

std::vector<boost::asio::ip::address_v4> getIPs();
std::vector<std::string> getServices(boost::asio::ip::address_v4 ip);

} // namespace IPList

#endif
