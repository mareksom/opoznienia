#ifndef _UDPMETHOD_H_
#define _UDPMETHOD_H_

#include "Data.h"

#include <boost/asio.hpp>
#include <vector>
#include <cstdint>
#include <memory>

class UDPMethod
{
public:
	UDPMethod();

	void runMeasurement();

private:
	void startListening();
	void handleReceive(const boost::system::error_code & error, std::size_t size);
	void sendData(boost::asio::ip::udp::endpoint where, std::shared_ptr<Data> data);

	/* Measurement stuff */
	std::map<boost::asio::ip::address_v4, uint64_t> IPtoTime;

	/* Network stuff */
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint rcv_endpoint;
	std::vector<uint8_t> buffer;
};

class UDPServer
{
public:
	UDPServer();

private:
	void startReceive();
	void handleReceive(const boost::system::error_code & error, std::size_t size);
	void handleSend(const boost::system::error_code & error);

	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint remote_endpoint;
	std::vector<uint8_t> buffer;
};

class UDPClient
{
public:
	UDPClient(boost::asio::ip::udp::endpoint remote_endpoint);

private:
	void setTimerTime();

	void startMeasurement();
	void handleStartOfMeasurement(const boost::system::error_code & error);
	void handleMessageSent(const boost::system::error_code & error);
	void handleResponseReceived(const boost::system::error_code & error, std::size_t size);
	void handleTimeout(const boost::system::error_code & error);

	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint remote_endpoint;
	std::vector<uint8_t> buffer;
	boost::asio::deadline_timer timer;
	uint64_t time_sent;
};

#endif
