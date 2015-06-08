#ifndef _MDNS_H_
#define _MDNS_H_

#include "Dns.h"
#include "Data.h"

#include <boost/asio.hpp>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <cstdint>

class MDnsDiscoverer
{
public:
	MDnsDiscoverer();

private:
	/* Timer */
	boost::asio::deadline_timer timer;
	void setTimer(int milliseconds);

	/* Instance name */
	const std::string name; // host name
	int nameNumber; // pair (name, nameNumber) MUST be unique in the network
	std::string getName() const;

	void determineName();
	void startProbes(const boost::system::error_code & error);
	int triesLeft;
	void nextTry();
	void receiveProbe();
	void handleProbeSent(const boost::system::error_code & error, std::shared_ptr<Data> data);
	void handleProbeResponse(const boost::system::error_code & error, std::size_t size);
	void handleProbeTimeout(const boost::system::error_code & error);

	/* Discovery */
	std::map<DNS_Name, boost::asio::ip::address_v4 > instanceToIP;

	void startDiscoveryServer();
	void listenForResponse();
	void handleResponse(const boost::system::error_code & error, std::size_t size);
	void readPacketResponse(const DNS_Packet & packet);
	void readPacketQuery(const DNS_Packet & packet);
	void readPacketQuestion(const DNS_Question & question);
	void readPacketAnswer(const DNS_RRecord & record);
	void appendMyIPs(const DNS_Name & name, DNS_Packet & packet);
	void sendPacket(const DNS_Packet & packet, bool unicast = false);
	void runDiscovery();
	void handleDiscoveryTimeout(const boost::system::error_code & error);

	/* Network stuff */
	std::vector<boost::asio::ip::address> myIPs;
	std::vector<uint8_t> responseBuffer;
	std::size_t responseBufferSize;
	boost::asio::ip::udp::endpoint rcv_endpoint;
	boost::asio::ip::address my_address;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint multicast;
};

#endif
