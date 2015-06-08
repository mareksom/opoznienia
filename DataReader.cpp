#include "DataReader.h"

#include <arpa/inet.h>

DataReader::DataReader(const uint8_t * data, unsigned length) : data(data), length(length)
{
}

DataReader::DataReader(const std::vector<uint8_t> & data) : DataReader(data.data(), data.size())
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
}

uint64_t DataReader::read64(unsigned & pos) const
{
	uint64_t value =
		((uint64_t) (*this)[pos + 0] << 56) |
		((uint64_t) (*this)[pos + 1] << 48) |
		((uint64_t) (*this)[pos + 2] << 40) |
		((uint64_t) (*this)[pos + 3] << 32) |
		((uint64_t) (*this)[pos + 4] << 24) |
		((uint64_t) (*this)[pos + 5] << 16) |
		((uint64_t) (*this)[pos + 6] <<  8) |
		((uint64_t) (*this)[pos + 7] <<  0);
	pos += 8;
	return value;
}

std::ostream & operator << (std::ostream & stream, const DataReader & reader)
{
	stream << "DataReader[" << reader.length << "] = {";
	for(unsigned i = 0; i < reader.length; i++)
	{
		if(i)
			stream << ", ";
		for(int j = 7; j >= 0; j--)
			if(reader.data[i] & (1 << j))
				stream << "1";
			else
				stream << "0";
		stream << " " << (int) reader.data[i];
	}
	stream << "}";
	return stream;
}
