#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <exception>

namespace Options {

	int PortUDP(); /* delay measurement with UDP */
	int PortInterface(); /* connect to the user interface */

	int DeltaMeasurement(); /* time delta between measurements (in milliseconds) */
	int DeltaSearch(); /* time delta between searching for another serwers (in milliseconds) */
	int DeltaRefresh(); /* time delta between refreshing user interface (in milliseconds) */

	int ServerSSH(); /* 0/1 -- annoucing _ssh._tcp */

	void InitArguments(int argc, char ** argv);
	void PrintOptions();

} // namespace Options

#endif
