#include "Options.h"
#include "Error.h"
#include "Dns.h"
#include "IOService.h"
#include "UDPMethod.h"
#include "ICMPMethod.h"
#include "MDns.h"
#include "Utils.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>

using std::cout;

using boost::asio::ip::udp;
using boost::asio::ip::icmp;

/*
void programx()
{
	boost::asio::io_service io_service;
	udp::resolver resolver(io_service);
	//udp::resolver::query query(udp::v4(), "8.8.8.8", "53");
	udp::resolver::query query(udp::v4(), "198.41.0.4", "53");
	udp::endpoint reciever_endpoint = *resolver.resolve(query);

	udp::socket socket(io_service);
	socket.open(udp::v4());

	DNS_Packet packet;
	packet.setID(12345);
	packet.setQR(false);
	packet.setOPCode(DNS_OPCode::QUERY);
	packet.setAA(false);
	packet.setTC(false);
	packet.setRD(true);
	packet.setRA(false);
	packet.setZ(0);
	packet.setRCode(DNS_RCode::NO_ERROR);

	DNS_Question question;
	question.setQName(DNS_Name("www", "mimuw", "edu", "pl"));
	question.setQType(DNS_QType::A);
	question.setQClass(DNS_QClass::IN);

	packet.addQD(question);

	Data data = packet.getData();

	socket.send_to(boost::asio::buffer(data.getPtr(), data.getLength()), reciever_endpoint);

	std::vector<uint8_t> recv_buf(1024, 0);
	boost::system::error_code error;
	unsigned rozmiar = socket.receive_from(boost::asio::buffer(recv_buf), reciever_endpoint, 0, error);

	DNS_Packet pakiet(DataReader(recv_buf.data(), rozmiar), 0);

	cout << packet;
	cout << pakiet;
}
*/

void program()
{
	/*
	UDPServer serverUDP;

	udp::resolver resolver(IO);
	udp::resolver::query query(udp::v4(), "localhost", "12345");
	udp::endpoint receiver_endpoint = *resolver.resolve(query);

	UDPClient clientUDP(receiver_endpoint);
	*/

	/*
	icmp::endpoint receiver_endpoint_icmp(boost::asio::ip::address::from_string("8.8.8.8"), 0);

	ICMPClient clientICMP(receiver_endpoint_icmp);

	*/

	MDnsDiscoverer mDnsDiscoverer;

	cout << "przed run\n";
	IO.run();
	cout << "po run\n";
}

int main(int argc, char ** argv)
{
	//auto t = std::chrono::duration_cast< std::chrono::microseconds > (std::chrono::system_clock::now().time_since_epoch()).count();
	//cout << t << "\n";
	//cout << getTime() << "\n";
	try
	{
		Options::InitArguments(argc - 1, argv + 1);
		Options::PrintOptions();
		program();
	}
	catch(const ReturnException & re)
	{
		return re.getExitStatus();
	}
  return 0;
}
