#ifndef _ICMPMETHOD_H_
#define _ICMPMETHOD_H_

#include "Ping.h"

#include <boost/asio.hpp>
#include <vector>
#include <cstdint>

class ICMPClient
{
public:
	ICMPClient(boost::asio::ip::icmp::endpoint remote_endpoint);

private:
	void setTimerTime();

	void startMeasurement();
	void handleStartOfMeasurement(const boost::system::error_code & error);
	void handleMessageSent(const boost::system::error_code & error);
	void startReceiving();
	void handleMessageReceived(const boost::system::error_code & error, std::size_t size);
	void handleTimeout(const boost::system::error_code & error);

	boost::asio::ip::icmp::socket socket;
	boost::asio::ip::icmp::endpoint remote_endpoint, rcv_endpoint;
	std::vector<uint8_t> buffer;
	boost::asio::deadline_timer timer;
	uint64_t time_sent;

	PingPacket request, response;
};

#endif
