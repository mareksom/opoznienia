#include "ICMPMethod.h"
#include "IOService.h"
#include "Utils.h"
#include "Error.h"
#include "Options.h"
#include "Data.h"
#include "DataReader.h"

#include <boost/bind.hpp>
#include <iostream>

using boost::asio::ip::icmp;
using boost::asio::error::operation_aborted;
using std::cout;

// ICMPClient -----------------

ICMPClient::ICMPClient(icmp::endpoint remote_endpoint) :
	socket(IO, icmp::v4()),
	remote_endpoint(remote_endpoint),
	buffer(120),
	timer(IO)
{
	request.setType(PingPacket::ECHO_MESSAGE);
	request.setCode(0);
	request.setID(0x13);
	request.setSequence(0);
	request.setIndex(347247);
	request.setGroup(3);
	startMeasurement();
}

void ICMPClient::setTimerTime()
{
	timer.expires_from_now(
		boost::posix_time::milliseconds(Options::DeltaMeasurement())
	);
}

void ICMPClient::startMeasurement()
{
	setTimerTime();
	timer.async_wait(boost::bind(
		&ICMPClient::handleStartOfMeasurement, this,
		boost::asio::placeholders::error
	));
}

void ICMPClient::handleStartOfMeasurement(const boost::system::error_code & error)
{
	if(error)
		err << "ICMPClient.handleStartOfMeasurement: " << error << "\n";
	else
	{
		time_sent = getMicroTime();
		Data data = request.getData();
		socket.async_send_to(
			boost::asio::buffer(data.getPtr(), data.getLength()), remote_endpoint,
			boost::bind(&ICMPClient::handleMessageSent, this,
				boost::asio::placeholders::error
			)
		);
	}
}

void ICMPClient::handleMessageSent(const boost::system::error_code & error)
{
	if(error)
		err << "ICMPClient.handleMessageSent: " << error << "\n";
	else
	{
		setTimerTime();
		timer.async_wait(boost::bind(
			&ICMPClient::handleTimeout, this,
			boost::asio::placeholders::error
		));
		startReceiving();
	}
}

void ICMPClient::startReceiving()
{
	socket.async_receive_from(
		boost::asio::buffer(buffer), rcv_endpoint,
		boost::bind(&ICMPClient::handleMessageReceived, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void ICMPClient::handleMessageReceived(const boost::system::error_code & error, std::size_t size)
{
	if(error == operation_aborted)
		;
	else if(error)
		err << "ICMPClient.handleMessageReceived: " << error << "\n";
	else if(remote_endpoint != rcv_endpoint)
	{
		if(remote_endpoint != rcv_endpoint)
		{
			startReceiving();
			return;
		}
		try
		{
			cout << "ICMPClient: ";
			auto wypisz = [] (uint8_t a) {
				for(int i = 7; i >= 0; i--)
					if(a & (1 << i))
						cout << "1";
					else
						cout << "0";
				cout << " ";
			};
			for(unsigned i = 0; i < size; i++)
				wypisz(buffer[i]);
			cout << "\n";

			if(size != 12)
				throw MalformedPacket();
			response = PingPacket(DataReader(buffer), 0);
			cout << "got smth\n";
			cout << response;
		}
		catch (MalformedPacket & mp)
		{
			err << "ICMPClient.handleMessageReceived: Malformed packet\n";
		}
	}
}

void ICMPClient::handleTimeout(const boost::system::error_code & error)
{
}
