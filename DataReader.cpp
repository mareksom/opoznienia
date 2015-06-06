#include "DataReader.h"

#include <arpa/inet.h>

DataReader::DataReader(const uint8_t * data, unsigned length) : data(data), length(length)
{
}

const uint8_t * DataReader::getPtr() const
{
	return data;
}

unsigned DataReader::getLength() const
{
	return length;
}

uint8_t DataReader::operator [] (unsigned index) const
{
	if(index >= length)
		throw DataReaderOutOfBounds();
	return data[index];
}

DataReader & DataReader::operator += (unsigned index)
{
	if(index > length)
		throw DataReaderOutOfBounds();
	data += index;
	length -= index;
	return *this;
}

const DataReader DataReader::operator + (unsigned index) const
{
	DataReader reader(*this);
	return reader += index;
}

uint8_t DataReader::read8(unsigned & pos) const
{
	uint8_t value =
		((uint8_t) (*this)[pos + 0] << 0);
	pos++;
	return value;
}

uint16_t DataReader::read16(unsigned & pos) const
{
	uint16_t value =
		((uint16_t) (*this)[pos + 0] << 8) |
		((uint16_t) (*this)[pos + 1] << 0);
	pos += 2;
	return value;
	//return ntohs(value);
}

uint32_t DataReader::read32(unsigned & pos) const
{
	uint32_t value =
		((uint32_t) (*this)[pos + 0] << 24) |
		((uint32_t) (*this)[pos + 1] << 16) |
		((uint32_t) (*this)[pos + 2] <<  8) |
		((uint32_t) (*this)[pos + 3] <<  0);
	pos += 4;
	return value;
	//return ntohl(value);
}
