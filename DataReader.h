#ifndef _DATAREADER_H_
#define _DATAREADER_H_

#include <exception>
#include <cstdint>

class DataReader
{
public:
	DataReader(const uint8_t * data, unsigned length);

	const uint8_t * getPtr() const;
	unsigned getLength() const;

	uint8_t operator [] (unsigned index) const;

	DataReader & operator += (unsigned index);
	const DataReader operator + (unsigned index) const;

	uint8_t read8(unsigned & pos) const;
	uint16_t read16(unsigned & pos) const;
	uint32_t read32(unsigned & pos) const;

private:
	const uint8_t * data;
	unsigned length;
};

class DataReaderOutOfBounds : public std::exception { };

#endif
