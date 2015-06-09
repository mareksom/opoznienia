#ifndef _UISERVER_H_
#define _UISERVER_H_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <vector>
#include <string>
#include <cstdint>

class TCPConnection
{
	friend class UIServer;

public:
	TCPConnection(std::vector<std::string> & rows);
	void start();

private:
	void configureTelnet();
	void redraw();

	std::vector<uint8_t> buffer;
	void startListening();
	void handleReceive(const boost::system::error_code & error, std::size_t size);

	void setTimer();
	void handleTimeout(const boost::system::error_code & error);

	int countdownToSuicide;
	void IWannaDie();

	int lineNumber;
	boost::asio::ip::tcp::socket socket;
	boost::asio::deadline_timer timer;
	std::vector<std::string> & rows;
};

class UIServer
{
public:
	UIServer();

private:
	/* Accepting telnet clients */
	boost::asio::ip::tcp::acceptor acceptor;
	void startAccept();
	void handleAccept(const boost::system::error_code & error, TCPConnection * connection);

	/* Refreshing data */
	std::vector<std::string> rows;
	void refreshData();
	void startTimer();
	void handleTimeout(const boost::system::error_code & error);
	boost::asio::deadline_timer timer;
};

#endif
