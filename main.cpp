#include "Options.h"
#include "Error.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::udp;

void program()
{
	boost::asio::io_service io_service;
	udp::resolver resolver(io_service);
	udp::resolver::query query(udp::v4(), "8.8.8.8", "5353");
	udp::endpoint reciever_endpoint = *resolver.resolve(query);

	udp::socket socket(io_service);
	socket.open(udp::v4());

	boost::array<uint8_t, 10> send_buf = {{ 'n', 'a', 'p', 'i', 's', 's', 'i', 'p', 'a', 'n' }};
	socket.send_to(boost::asio::buffer(send_buf), reciever_endpoint);
}

int main(int argc, char ** argv)
{
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
