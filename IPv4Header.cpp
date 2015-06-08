#include "IPv4Header.h"
#include "DataReader.h"
#include "MalformedPacket.h"

IPv4Header::IPv4Header(const Data & data) : data(data)
{
}

unsigned IPv4Header::getHeaderLength() const
{
	try
	{
		DataReader reader(data.getPtr(), data.getLength());
		unsigned pos = 0;
		return (reader.read8(pos) & 15) * 4;
	}
	catch(DataReaderOutOfBounds & droob)
	{
		throw MalformedPacket();
	}
}

Data IPv4Header::getInnerData() const
{
	try
	{
		DataReader reader(data.getPtr(), data.getLength());
		reader += getHeaderLength();
		return Data(reader.getPtr(), reader.getLength());
	}
	catch(DataReaderOutOfBounds & droob)
	{
		throw MalformedPacket();
	}
}
