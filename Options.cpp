#include "Options.h"
#include "Error.h"

#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <iostream>

using std::string;
using std::vector;
using std::pair;
using std::function;
using std::cout;
using std::endl;

namespace Options {

namespace {

int portUDP = 3382;
int portInterface = 3637;
int deltaMeasurement = 1 * 1000; /* milliseconds */
int deltaSearch = 10 * 1000; /* milliseconds */
int deltaRefresh = 1 * 1000; /* milliseconds */
int serverSSH = 0;

void SetPortIfValid(const string & option, const string & value, int & destination)
{
	long int port;
	port = strtol(value.c_str(), nullptr, 10);
	if(port >= 1 && port <= 65535)
		destination = port;
	else
		err << "Expected port number after " << option << ", got " << value << errend;
}

void SetTimeIfValid(const string & option, const string & value, int & destination)
{
	double time;
	time = strtod(value.c_str(), NULL);
	if(time < 0.01 || time > 60 * 60 * 24)
		err << "Expected time in seconds from range [0.01, " << 60 * 60 * 24 << "] after " << option << ", got " << value << errend;
	else
		destination = (int) (time * 1000);
}

void SetPortUDP(const string & option, const string & value)
{
	SetPortIfValid(option, value, portUDP);
}

void SetPortInterface(const string & option, const string & value)
{
	SetPortIfValid(option, value, portInterface);
}

void SetDeltaMeasurement(const string & option, const string & value)
{
	SetTimeIfValid(option, value, deltaMeasurement);
}

void SetDeltaSearch(const string & option, const string & value)
{
	SetTimeIfValid(option, value, deltaSearch);
}

void SetDeltaRefresh(const string & option, const string & value)
{
	SetTimeIfValid(option, value, deltaRefresh);
}

void SetServerSSH(const string & option)
{
	serverSSH = 1;
}

typedef function<void(const string &)> ZeroArgument;
typedef function<void(const string &, const string &)> OneArgument;

vector< pair< string, ZeroArgument > > ZeroArgumentFunctions = {
	{"-s", SetServerSSH}
};

vector< pair< string, OneArgument > > OneArgumentFunctions = {
	{"-u", SetPortUDP},
	{"-U", SetPortInterface},
	{"-t", SetDeltaMeasurement},
	{"-T", SetDeltaSearch},
	{"-v", SetDeltaRefresh},
};

} // namespace

void InitArguments(int argc, char ** argv)
{
	if(argc == 0)
		return;
	for(auto & f : ZeroArgumentFunctions)
	{
		if(f.first == argv[0])
		{
			f.second(argv[0]);
			InitArguments(argc - 1, argv + 1);
			return;
		}
	}
	for(auto & f : OneArgumentFunctions)
	{
		if(f.first == argv[0])
		{
			if(argc == 1)
				err << "Expected an argument after " << argv[0] << errend;
			f.second(argv[0], argv[1]);
			InitArguments(argc - 2, argv + 2);
			return;
		}
	}
	err << "Unknown option " << argv[0] << errend;
}

int PortUDP()          { return portUDP;          }
int PortInterface()    { return portInterface;    }
int DeltaMeasurement() { return deltaMeasurement; }
int DeltaSearch()      { return deltaSearch;      }
int DeltaRefresh()     { return deltaRefresh;     }
int ServerSSH()        { return serverSSH;        }

void PrintOptions()
{
	cout << "              UDP port:  " << PortUDP() << endl;
	cout << "        Interface port:  " <<  PortInterface() << endl;
	cout << "Measurement delta time:  " << (double) DeltaMeasurement() / 1000 << "s" << endl;
	cout << "mDNS-search delta time:  " << (double) DeltaSearch() / 1000 << "s" << endl;
	cout << "    Refresh delta time:  " << (double) DeltaRefresh() / 1000 << "s" << endl;
	cout << "   Annoucing _ssh._tcp:  " << (ServerSSH() ? "ON" : "OFF") << endl;
}

} // namespace Options
