#ifndef _PING_H_
#define _PING_H_

#include "Data.h"
#include "DataReader.h"
#include "MalformedPacket.h"

#include <cstdint>
#include <iostream>

class PingPacket
{
public:
	static const uint8_t ECHO_MESSAGE = 8;
	static const uint8_t ECHO_REPLY_MESSAGE = 0;

	PingPacket();
	PingPacket(DataReader reader, unsigned pos);

	Data getData() const;

	void setType(uint8_t type);
	void setCode(uint8_t code);
	void setID(uint16_t id);
	void setSequence(uint16_t sequence);
	void setIndex(uint32_t index);
	void setGroup(uint8_t group);

	uint8_t getType() const;
	uint8_t getCode() const;
	uint16_t getID() const;
	uint16_t getSequence() const;
	uint32_t getIndex() const;
	uint8_t getGroup() const;

	friend std::ostream & operator << (std::ostream & stream, const PingPacket & packet);

private:
	uint8_t type;
	uint8_t code;
	uint16_t id;
	uint16_t sequence;

	uint32_t index;
	uint8_t group;
};

#endif
