#ifndef _IPV4HEADER_H_
#define _IPV4HEADER_H_

#include "Data.h"

class IPv4Header
{
public:
	IPv4Header(const Data & data);

	unsigned getHeaderLength() const;

	Data getInnerData() const;

private:
	Data data;
};

#endif
