#include "Data.h"

#include <arpa/inet.h>

Data::Data()
{
}

Data::Data(const std::vector<uint8_t> & data) : data(data)
{
}

Data::Data(const uint8_t * msg, unsigned length) : data(msg, msg + length)
{
}

Data::Data(const Data & data) : data(data.data)
{
}

const uint8_t * Data::getPtr() const
{
	return data.data();
}

unsigned Data::getLength() const
{
	return (unsigned) data.size();
}

void Data::append(const Data & data)
{
	this->data.insert(this->data.end(), data.data.begin(), data.data.end());
}

void Data::append(uint8_t value)
{
	data.emplace_back(value);
}

void Data::append(uint16_t value)
{
	//value = htons(value);
	data.emplace_back((uint8_t) ((value >> 8) & ((1 << 8) - 1)));
	data.emplace_back((uint8_t) ((value >> 0) & ((1 << 8) - 1)));
}

void Data::append(uint32_t value)
{
	//value = htonl(value);
	data.emplace_back((uint8_t) ((value >> 24) & ((1 << 8) - 1)));
	data.emplace_back((uint8_t) ((value >> 16) & ((1 << 8) - 1)));
	data.emplace_back((uint8_t) ((value >>  8) & ((1 << 8) - 1)));
	data.emplace_back((uint8_t) ((value >>  0) & ((1 << 8) - 1)));
}

void Data::prepend(const Data & data)
{
	this->data.insert(this->data.begin(), data.data.begin(), data.data.end());
}

void Data::swap(Data & x) noexcept
{
	data.swap(x.data);
}

std::ostream & operator << (std::ostream & stream, const Data & data)
{
	const uint8_t * ptr = data.getPtr();
	int len = data.getLength();
	stream << "Data[" << len << "](";
	for(int i = 0; i < len; i++)
	{
		if(i)
			stream << ", ";
		stream << (int) ptr[i];
	}
	stream << ")";
	return stream;
}
