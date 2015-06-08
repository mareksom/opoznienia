#ifndef _ICMPMETHOD_H_
#define _ICMPMETHOD_H_

#include "Method.h"
#include "Ping.h"
#include "Data.h"

#include <boost/asio.hpp>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

class ICMPMethod : public Method
{
public:
	ICMPMethod();

	virtual void runMeasurement();

private:
	void startListening();
	void handleReceive(const boost::system::error_code & error, std::size_t size);
	void sendData(boost::asio::ip::icmp::endpoint where, std::shared_ptr<Data> data);

	/* Measurement stuff */
	uint16_t sequenceNumber;
	std::map<boost::asio::ip::address_v4, uint64_t> IPtoTime;

	/* Network stuff */
	boost::asio::ip::icmp::socket socket;
	boost::asio::ip::icmp::endpoint rcv_endpoint;
	std::vector<uint8_t> buffer;
	boost::asio::deadline_timer timer;
};

#endif
