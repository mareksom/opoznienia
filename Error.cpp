#include "Error.h"

#include <cstdlib>

Error::Error(bool connection) : connection(connection)
{
}

Error & Error::operator << (const ErrorEnd & end)
{
	if(!connection or Options::PrintConnectionErrors())
		std::cerr << std::endl;
	throw ReturnException(EXIT_FAILURE);
}

Error::ErrorEnd errend;
Error err(false);
Error connectionerr(true);

ReturnException::ReturnException(int exitstatus) : exitstatus(exitstatus)
{
}

int ReturnException::getExitStatus() const
{
	return exitstatus;
}
