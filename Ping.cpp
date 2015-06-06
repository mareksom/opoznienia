#include "Ping.h"
#include "Utils.h"

PingPacket::PingPacket()
{
}

PingPacket::PingPacket(DataReader reader, unsigned pos)
{
	try
	{
		auto wypisz = [] (uint8_t a) {
			for(int i = 7; i>= 0; i--)
				if(a & (1 << i))
					std::cout << "1";
				else
					std::cout << "0";
			std::cout << " ";
		};
		unsigned yolo = pos;
		for(unsigned i = 0; i < reader.getLength(); i++)
			wypisz(reader.read8(yolo));
		std::cout << "\n";

		unsigned position = pos;
		type = reader.read8(position);
		code = reader.read8(position);
		uint16_t checksum = reader.read16(position);
		id = reader.read16(position);
		sequence = reader.read16(position);
		uint32_t data = reader.read32(position);
		index = data & ((1 << 24) - 1);
		group = data >> 24;

		uint16_t sum = ~OnesComplementSum(
			reader.read16(pos),
			OnesComplementSum((uint16_t*) reader.getPtr() + 2, 4)
		);
		
		if(checksum != sum and false)
		{
			std::cout << "checksum = " << checksum << ", sum = " << sum << "\n";
			throw MalformedPacket();
		}
	}
	catch(DataReaderOutOfBounds & droob)
	{
		throw MalformedPacket();
	}
}

Data PingPacket::getData() const
{
	Data data;
	data.append(type);
	data.append(code);
	data.append((uint16_t) 0); // checksum is 0 now
	data.append(id);
	data.append(sequence);
	data.append(index | ((uint32_t) group << 24));

	data.getAt16(2) = ~OnesComplementSum((uint16_t*) data.getPtr(), data.getLength() / 2);

	return data;
}

void PingPacket::setType(uint8_t type) { this->type = type; }
void PingPacket::setCode(uint8_t code) { this->code = code; }
void PingPacket::setID(uint16_t id) { this->id = id; }
void PingPacket::setSequence(uint16_t sequence) { this->sequence = sequence; }
void PingPacket::setIndex(uint32_t index) { this->index = index; }
void PingPacket::setGroup(uint8_t group) { this->group = group; }

uint8_t PingPacket::getType() const { return type; }
uint8_t PingPacket::getCode() const { return code; }
uint16_t PingPacket::getID() const { return id; }
uint16_t PingPacket::getSequence() const { return sequence; }
uint32_t PingPacket::getIndex() const { return index; }
uint8_t PingPacket::getGroup() const { return group; }

std::ostream & operator << (std::ostream & stream, const PingPacket & packet)
{
	stream << "PingPacket(\n";

	stream << "  type = (" << (int) packet.type << ") ";
	if(packet.type == packet.ECHO_MESSAGE)
		stream << "ECHO_MESSAGE";
	else if(packet.type == packet.ECHO_REPLY_MESSAGE)
		stream << "ECHO_REPLY_MESSAGE";
	else
		stream << "UNKNOWN";
	stream << "\n";

	stream << "  code = " << (int) packet.code << "\n";
	stream << "  id = " << (int) packet.id << "\n";
	stream << "  sequence = " << (int) packet.sequence << "\n";
	stream << "  index = " << (int) packet.index << "\n";
	stream << "  group = " << (int) packet.group << "\n";

	stream << ")\n";
	return stream;
}
