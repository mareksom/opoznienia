#ifndef _MEASUREMENT_H_
#define _MEASUREMENT_H_

#include <boost/asio.hpp>
#include <string>
#include <map>
#include <utility>
#include <cstdint>

namespace Measurement {

void addMeasurement(boost::asio::ip::address_v4 ip, const std::string & method, uint64_t time);
void skipMeasurements();

std::map<std::string, uint64_t> getTimes(boost::asio::ip::address_v4 ip);

} // namespace Measurement

#endif
