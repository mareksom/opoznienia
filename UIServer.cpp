#include "UIServer.h"
#include "IOService.h"
#include "Options.h"
#include "Error.h"
#include "IPList.h"
#include "Measurement.h"

#include <boost/bind.hpp>
#include <algorithm>
#include <sstream>

using boost::asio::ip::tcp;
using boost::asio::error::operation_aborted;

namespace {
const int screenWidth = 80 - 2;
const int screenHeight = 23;
const int IPWidth = 16;
} // namespace

UIServer::UIServer() :
	acceptor(IO, tcp::endpoint(tcp::v4(), Options::PortInterface())),
	timer(IO)
{
	refreshData();
	startTimer();
	startAccept();
}

/* Accepting telnet clients */

TCPConnection::TCPConnection(std::vector<std::string> & rows) :
	buffer(10),
	countdownToSuicide(0),
	lineNumber(0),
	socket(IO),
	timer(IO),
	rows(rows)
{
}

void TCPConnection::start()
{
	setTimer();
	startListening();
}

void TCPConnection::redraw()
{
	if(lineNumber < 0)
		lineNumber = 0;
	while(lineNumber > 0 and ((int) rows.size() - lineNumber) < screenHeight)
		lineNumber--;
	std::stringstream ss;
	ss << "\e[2J\e[H";

	for(int i = 0; i < screenHeight && lineNumber + i < (int) rows.size(); i++)
	{
		if(i)
			ss << '\n';
		ss << rows[lineNumber + i];
	}

	countdownToSuicide++;

	std::shared_ptr<std::string> message = std::make_shared<std::string>(ss.str());
	socket.async_send(
		boost::asio::buffer(message->data(), message->size()),
		[this, message] (const boost::system::error_code & error, std::size_t) {
			countdownToSuicide--;
			if(error)
			{
				if(error == operation_aborted)
					;
				else
					connectionerr << "TCPConnection::redraw: " << error.message() << " " << countdownToSuicide << "\n";
				IWannaDie();
			}
		}
	);
}

void TCPConnection::startListening()
{
	countdownToSuicide++;
	socket.async_receive(
		boost::asio::buffer(buffer),
		boost::bind(
			&TCPConnection::handleReceive, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void TCPConnection::handleReceive(const boost::system::error_code & error, std::size_t size)
{
	countdownToSuicide--;
	if(error)
	{
		if(error == operation_aborted)
			;
		else
			connectionerr << "TCPConnection::handleReceive: " << error.message() << "\n";
		IWannaDie();
	}
	else
	{
		for(int i = 0; i < (int) size; i++)
		{
			if(buffer[i] == 'A' or buffer[i] == 'a')
				lineNumber++;
			else if(buffer[i] == 'Q' or buffer[i] == 'q')
				lineNumber--;
		}
		redraw();
		startListening();
	}
}

void TCPConnection::setTimer()
{
	timer.expires_from_now(
		boost::posix_time::milliseconds(Options::DeltaRefresh())
	);
	countdownToSuicide++;
	timer.async_wait(boost::bind(
		&TCPConnection::handleTimeout, this,
		boost::asio::placeholders::error
	));
}

void TCPConnection::handleTimeout(const boost::system::error_code & error)
{
	countdownToSuicide--;
	if(error)
	{
		if(error == operation_aborted)
			;
		else
			connectionerr << "TCPConnection::handleTimeout: " << error.message() << "\n";
		IWannaDie();
	}
	else
	{
		redraw();
		setTimer();
	}
}

void TCPConnection::IWannaDie()
{
	if(countdownToSuicide == 0)
	{
		connectionerr << "TCPConnection::IWannaDie: closing connection\n";
		delete this;
	}
	else
	{
		socket.cancel();
		timer.cancel();
	}
}

void UIServer::startAccept()
{
	TCPConnection * connection = new TCPConnection(rows);
	acceptor.async_accept(
		connection->socket,
		boost::bind(
			&UIServer::handleAccept, this,
			boost::asio::placeholders::error,
			connection
		)
	);
}

void UIServer::handleAccept(const boost::system::error_code & error, TCPConnection * connection)
{
	if(error)
	{
		connectionerr << "UIServer::handleAccept: " << error.message() << "\n";
		delete connection;
	}
	else
		connection->start();

	startAccept();
}

/* Refreshing data */

namespace {

class Entry
{
public:
	Entry(boost::asio::ip::address_v4 ip, uint64_t udp, uint64_t icmp, uint64_t tcp) :
		ip(ip), udp(udp), icmp(icmp), tcp(tcp)
	{
	}

	boost::asio::ip::address_v4 getIP() const
	{
		return ip;
	}

	uint64_t getMean() const
	{
		uint64_t sum = 0;
		uint8_t count = 0;
		for(uint64_t i : {udp, icmp, tcp})
		{
			if(i)
			{
				sum += i;
				count++;
			}
		}
		if(count == 0)
			return 0;
		return (sum + count - 1) / count;
	}

	bool empty() const
	{
		return getMean() == 0;
	}

	bool operator < (const Entry & o) const
	{
		return getMean() > o.getMean();
	}

	std::string toString() const
	{
		std::stringstream ss;
		int it = 0;
		for(uint64_t i : {udp, tcp, icmp}) /* This order is required */
		{
			if(it)
				ss << " ";
			if(i == 0)
				ss << "N/A";
			else
				ss << (i + (1000 - 1)) / 1000; /* convert to milliseconds */
			it++;
		}
		return ss.str();
	}

private:
	boost::asio::ip::address_v4 ip;
	uint64_t udp, icmp, tcp;
};

std::string IPToString(boost::asio::ip::address_v4 address)
{
	std::string ip = address.to_string();
	while((int) ip.length() < IPWidth)
		ip += " ";
	return ip;
}

} // namespace

void UIServer::refreshData()
{
	rows.clear();

	std::vector<Entry> entries;
	for(auto & address : IPList::getIPs())
	{
		auto m = Measurement::getTimes(address);
		entries.emplace_back(
			address,
			m[std::string("udp")],
			m[std::string("icmp")],
			m[std::string("tcp")]
		);
	}

	std::sort(entries.begin(), entries.end());

	int spaceLeft = screenWidth - IPWidth;
	uint64_t highestTime = 0;
	for(auto & entry : entries)
	{
		spaceLeft = std::min(spaceLeft, screenWidth - IPWidth - (int) entry.toString().length());
		highestTime = std::max(highestTime, entry.getMean());
	}
	
	if(spaceLeft < 0)
	{
		connectionerr << "Results are too wide to show them on " << screenWidth << " width screen!\n";
		spaceLeft = 0;
	}

	std::string border = "+";
	for(int i = 0; i < screenWidth; i++)
		border += '-';
	border += "+";

	rows.push_back(border);

	for(auto & entry : entries)
	{
		int spaces;
		if(highestTime)
			spaces = (spaceLeft * entry.getMean() + highestTime - 1) / highestTime;
		else
			spaces = 0;
		std::string row = "|";
		row += IPToString(entry.getIP());
		for(int i = 0; i < spaces; i++)
			row += " ";
		std::string data = entry.toString();
		row += data;
		for(int i = spaces; i < screenWidth - IPWidth - (int) data.length(); i++)
			row += " ";
		row += "|";
		rows.push_back(row);
	}

	rows.push_back(border);
}

void UIServer::startTimer()
{
	timer.expires_from_now(
		boost::posix_time::milliseconds(Options::DeltaRefresh())
	);
	timer.async_wait(boost::bind(
		&UIServer::handleTimeout, this,
		boost::asio::placeholders::error
	));
}

void UIServer::handleTimeout(const boost::system::error_code & error)
{
	if(error)
		connectionerr << "UIServer::handleTimeout: " << error.message() << "\n";
	else
		refreshData();
	startTimer();
}
