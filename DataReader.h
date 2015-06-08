#ifndef _DATAREADER_H_
#define _DATAREADER_H_

#include <exception>
#include <iostream>
#include <vector>
#include <cstdint>

class DataReader
{
public:
	DataReader(const uint8_t * data, unsigned length);
	DataReader(const std::vector<uint8_t> & data);

	const uint8_t * getPtr() const;
	unsigned getLength() const;

	uint8_t operator [] (unsigned index) const;

	DataReader & operator += (unsigned index);
	const DataReader operator + (unsigned index) const;

	uint8_t read8(unsigned & pos) const;
	uint16_t read16(unsigned & pos) const;
	uint32_t read32(unsigned & pos) const;
	uint64_t read64(unsigned & pos) const;

	friend std::ostream & operator << (std::ostream & strem, const DataReader & reader);

private:
	const uint8_t * data;
	unsigned length;
};

class DataReaderOutOfBounds : public std::exception { };

#endif
