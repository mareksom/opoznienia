#ifndef _ERR_H_
#define _ERR_H_

#include <exception>
#include <iostream>

class Error
{
public:
	class ErrorEnd { };

	template<typename Arg>
	Error & operator << (const Arg & arg)
	{
		std::cerr << arg;
		return *this;
	}

	Error & operator << (const ErrorEnd & end);
};

extern Error::ErrorEnd errend;
extern Error err;

class ReturnException : public std::exception
{
public:
	ReturnException(int exitstatus);
	int getExitStatus() const;

private:
	int exitstatus;
};

#endif
