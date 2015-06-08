#ifndef _GETIPADDRESS_H_
#define _GETIPADDRESS_H_

#include <boost/asio.hpp>
#include <vector>
#include <utility>
#include <string>

std::vector< std::pair< std::string, boost::asio::ip::address > > GetIPAddresses();

#endif
