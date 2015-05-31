#include "Error.h"

#include <cstdlib>

Error & Error::operator << (const ErrorEnd & end)
{
	std::cerr << std::endl;
	throw ReturnException(EXIT_FAILURE);
}

Error::ErrorEnd errend;
Error err;

ReturnException::ReturnException(int exitstatus) : exitstatus(exitstatus)
{
}

int ReturnException::getExitStatus() const
{
	return exitstatus;
}
