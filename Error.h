#ifndef _ERR_H_
#define _ERR_H_

#include "Options.h"

#include <exception>
#include <iostream>

class Error
{
public:
	class ErrorEnd { };

	Error(bool connection);

	template<typename Arg>
	Error & operator << (const Arg & arg)
	{
		if(!connection or Options::PrintConnectionErrors())
			std::cerr << arg;
		return *this;
	}

	Error & operator << (const ErrorEnd & end);

private:
	const bool connection;
};

extern Error::ErrorEnd errend;
extern Error err;
extern Error connectionerr;

class ReturnException : public std::exception
{
public:
	ReturnException(int exitstatus);
	int getExitStatus() const;

private:
	int exitstatus;
};

#endif
