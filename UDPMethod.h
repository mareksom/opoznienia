#ifndef _UDPMETHOD_H_
#define _UDPMETHOD_H_

#include "Method.h"
#include "Data.h"

#include <boost/asio.hpp>
#include <vector>
#include <cstdint>
#include <memory>

class UDPMethod : public Method
{
public:
	UDPMethod();

	virtual void runMeasurement();

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

#endif
