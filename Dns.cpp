#include "Dns.h"

#include <algorithm>

using std::string;
using std::vector;

// DNS_Name ------------------------

DNS_Name::DNS_Name()
{
}

DNS_Name::DNS_Name(const vector< string > & labels) : labels(labels)
{
}

string DNS_Name::getName() const
{
	string name;
	for(const string & s : labels)
	{
		if(name != "")
			name += '.';
		name += s;
	}
	return name;
}

Data DNS_Name::getData() const
{
	Data data;
	for(const string & s : labels)
	{
		data.append((uint8_t) s.length());
		for(char c : s)
			data.append((uint8_t) c);
	}
	data.append((uint8_t) 0);
	return data;
}

unsigned DNS_Name::readFromData(DataReader reader, unsigned pos)
{
	try
	{
		vector< string > newLabels;
		const uint8_t twoHighBits = 3 << 6; // = 11000000
		unsigned maxPos = pos;
		unsigned start = pos;
		while(reader[pos] != 0)
		{
			if((reader[pos] & twoHighBits) == twoHighBits)
			{
				unsigned newPos = (((unsigned) (reader[pos] ^ twoHighBits)) << 8) | reader[pos + 1];
				if(newPos >= start)
					throw MalformedPacket();
				maxPos = std::max(maxPos, pos + 2);
				start = pos = newPos;
			}
			else
			{
				unsigned length = reader[pos];
				string label;
				for(unsigned i = 1; i <= length; i++)
					label += (char) reader[pos + i];
				pos += length + 1;
				newLabels.emplace_back(label);
				maxPos = std::max(maxPos, pos);
			}
		}
		labels.swap(newLabels);
		return std::max(maxPos, pos + 1);
	}
	catch(const DataReaderOutOfBounds & droob)
	{
		throw MalformedPacket();
	}
}

void DNS_Name::swap(DNS_Name & x) noexcept
{
	std::swap(labels, x.labels);
}

std::ostream & operator << (std::ostream & stream, const DNS_Name & name)
{
	for(int i = 0; i < (int) name.labels.size(); i++)
	{
		if(i)
			stream << ".";
		stream << "\"" << name.labels[i] << "\"";
	}
	return stream;
}

// DNS_Question ---------------

DNS_QType DNS_Question::getQType() const
{
	return qtype;
}

DNS_QClass DNS_Question::getQClass() const
{
	return qclass;
}

DNS_Name DNS_Question::getQName() const
{
	return qname;
}

void DNS_Question::setQType(DNS_QType qtype)
{
	this->qtype = qtype;
}

void DNS_Question::setQClass(DNS_QClass qclass)
{
	this->qclass = qclass;
}

void DNS_Question::setQName(const DNS_Name & qname)
{
	this->qname = qname;
}

Data DNS_Question::getData() const
{
	Data data = qname.getData();
	data.append((uint16_t) qtype);
	data.append((uint16_t) qclass);
	return data;
}

unsigned DNS_Question::readFromData(DataReader reader, unsigned pos)
{
	try
	{
		DNS_Name newName;
		pos = newName.readFromData(reader, pos);
		DNS_QType newType = (DNS_QType) reader.read16(pos);
		DNS_QClass newClass = (DNS_QClass) reader.read16(pos);
		std::swap(qname, newName);
		std::swap(qtype, newType);
		std::swap(qclass, newClass);
		return pos;
	}
	catch(const DataReaderOutOfBounds & droob)
	{
		throw MalformedPacket();
	}
}

std::ostream & operator << (std::ostream & stream, const DNS_Question & question)
{
	return stream << "Question(" << question.qname
		<< ", qtype = " << (int) question.qtype
		<< ", qclass = " << (int) question.qclass << ")";
}

// DNS_RRecord ------------------------------------

DNS_Name DNS_RRecord::getName() const
{
	return name;
}

DNS_Type DNS_RRecord::getType() const
{
	return type;
}

DNS_Class DNS_RRecord::getClass() const
{
	return class_;
}

uint32_t DNS_RRecord::getTTL() const
{
	return ttl;
}

Data DNS_RRecord::getRData() const
{
	return rdata;
}

Data DNS_RRecord::getData() const
{
	Data data = name.getData();
	data.append((uint16_t) type);
	data.append((uint16_t) class_);
	data.append((uint32_t) ttl);
	data.append((uint16_t) rdata.getLength());
	data.append(rdata);
	return data;
}

unsigned DNS_RRecord::readFromData(DataReader reader, unsigned pos)
{
	try
	{
		DNS_Name newName;
		unsigned oldpos = pos;
		pos = newName.readFromData(reader, pos);
		DNS_Type newType = (DNS_Type) reader.read16(pos);
		DNS_Class newClass = (DNS_Class) reader.read16(pos);
		uint32_t newTTL = reader.read32(pos);
		Data newRData;
		unsigned rdlength = reader.read16(pos);
		for(int i = 0; i < rdlength; i++)
			newRData.append(reader.read8(pos));
		std::swap(name, newName);
		std::swap(type, newType);
		std::swap(class_, newClass);
		std::swap(ttl, newTTL);
		std::swap(rdata, newRData);
		return pos;
	}
	catch(const DataReaderOutOfBounds & droob)
	{
		throw MalformedPacket();
	}
}

std::ostream & operator << (std::ostream & stream, const DNS_RRecord & record)
{
	return stream << "RRecord(" << record.name
		<< ", type = " << (int) record.type
		<< ", class = " << (int) record.class_
		<< ", ttl = " << record.ttl
		<< ", rdata = " << record.rdata << ")";
}

// DNS_Packet -----------------------------------

DNS_Packet::DNS_Packet()
{
}

DNS_Packet::DNS_Packet(DataReader reader, unsigned pos)
{
	try
	{
		id = reader.read16(pos);

		int flags = reader.read8(pos);
		qr = flags & (1 << 7);                                 // 10000000
		opcode = (DNS_OPCode) (flags & (((1 << 4) - 1) << 3)); // 01111000
		aa = flags & 8;                                        // 00000100
		tc = flags & 2;                                        // 00000010
		rd = flags & 1;                                        // 00000001

		flags = reader.read8(pos);
		ra = flags & (1 << 7);                        // 10000000
		z = flags & (((1 << 3) - 1) << 4);            // 01110000
		rcode = (DNS_RCode) (flags & ((1 << 4) - 1)); // 00001111

		unsigned qdcount = reader.read16(pos);
		unsigned ancount = reader.read16(pos);
		unsigned nscount = reader.read16(pos);
		unsigned arcount = reader.read16(pos);

		for(int i = 0; i < qdcount; i++)
		{
			qd.emplace_back();
			pos = qd.back().readFromData(reader, pos);
		}

		for(int i = 0; i < ancount; i++)
		{
			an.emplace_back();
			pos = an.back().readFromData(reader, pos);
		}

		for(int i = 0; i < nscount; i++)
		{
			ns.emplace_back();
			pos = ns.back().readFromData(reader, pos);
		}

		for(int i = 0; i < arcount; i++)
		{
			ar.emplace_back();
			pos = ar.back().readFromData(reader, pos);
		}
	}
	catch(const DataReaderOutOfBounds & droob)
	{
		throw MalformedPacket();
	}
}

Data DNS_Packet::getData() const
{
	Data data;
	data.append((uint16_t) id);

	uint8_t flags =
		((uint8_t) qr << 7) |
		((uint8_t) opcode << 3) |
		((uint8_t) aa << 2) |
		((uint8_t) tc << 1) |
		((uint8_t) rd << 0);
	data.append(flags);

	flags =
		((uint8_t) ra << 7) |
		((uint8_t) z << 4) |
		((uint8_t) rcode << 0);
	data.append(flags);

	data.append((uint16_t) getQDCount());
	data.append((uint16_t) getANCount());
	data.append((uint16_t) getNSCount());
	data.append((uint16_t) getARCount());

	for(const DNS_Question & q : qd)
		data.append(q.getData());
	for(const DNS_RRecord & r : an)
		data.append(r.getData());
	for(const DNS_RRecord & r : ns)
		data.append(r.getData());
	for(const DNS_RRecord & r : ar)
		data.append(r.getData());
	return data;
}

uint16_t DNS_Packet::getID() const { return id; }
bool DNS_Packet::getQR() const { return qr; }
DNS_OPCode DNS_Packet::getOPCode() const { return opcode; }
bool DNS_Packet::getAA() const { return aa; }
bool DNS_Packet::getTC() const { return tc; }
bool DNS_Packet::getRD() const { return rd; }
bool DNS_Packet::getRA() const { return ra; }
uint8_t DNS_Packet::getZ() const { return z; }
DNS_RCode DNS_Packet::getRCode() const { return rcode; }

ConstIterable< std::vector< DNS_Question > > DNS_Packet::getQD() const
{
	return ConstIterable< decltype(qd) >(qd);
}

ConstIterable< std::vector< DNS_RRecord > > DNS_Packet::getAN() const
{
	return ConstIterable< decltype(an) >(an);
}

ConstIterable< std::vector< DNS_RRecord > > DNS_Packet::getNS() const
{
	return ConstIterable< decltype(ns) >(ns);
}

ConstIterable< std::vector< DNS_RRecord > > DNS_Packet::getAR() const
{
	return ConstIterable< decltype(ar) >(ar);
}

unsigned DNS_Packet::getQDCount() const { return (unsigned) qd.size(); }
unsigned DNS_Packet::getANCount() const { return (unsigned) an.size(); }
unsigned DNS_Packet::getNSCount() const { return (unsigned) ns.size(); }
unsigned DNS_Packet::getARCount() const { return (unsigned) ar.size(); }

void DNS_Packet::setID(uint16_t id) { this->id = id; }
void DNS_Packet::setQR(bool qr) { this->qr = qr; }
void DNS_Packet::setOPCode(DNS_OPCode opcode) { this->opcode = opcode; }
void DNS_Packet::setAA(bool aa) { this->aa = aa; }
void DNS_Packet::setTC(bool tc) { this->tc = tc; }
void DNS_Packet::setRD(bool rd) { this->rd = rd; }
void DNS_Packet::setRA(bool ra) { this->ra = ra; }
void DNS_Packet::setZ(uint8_t z) { this->z = z; }
void DNS_Packet::setRCode(DNS_RCode rcode) { this->rcode = rcode; }
void DNS_Packet::addQD(const DNS_Question & question) { qd.emplace_back(question); }
void DNS_Packet::addAN(const DNS_RRecord & record) { an.emplace_back(record); }
void DNS_Packet::addNS(const DNS_RRecord & record) { ns.emplace_back(record); }
void DNS_Packet::addAR(const DNS_RRecord & record) { ar.emplace_back(record); }

std::ostream & operator << (std::ostream & stream, const DNS_Packet & packet)
{
	stream << "DNS_Packet (\n";
	stream << "  id = " << packet.id << ", qr = " << packet.qr
		<< ", opcode = " << (int) packet.opcode
		<< ", aa = " << packet.aa << ", tc = " << packet.tc
		<< ", rd = " << packet.rd << "\n";
	stream << "  ra = " << packet.ra << ", z = " << packet.z
		<< ", rcode = " << (int) packet.rcode << "\n";
	for(const DNS_Question & q : packet.qd)
		stream << "  QD: " << q << "\n";
	for(const DNS_RRecord & r : packet.an)
		stream << "  AN: " << r << "\n";
	for(const DNS_RRecord & r : packet.ns)
		stream << "  NS: " << r << "\n";
	for(const DNS_RRecord & r : packet.ar)
		stream << "  AR: " << r << "\n";
	stream << ")\n";
	return stream;
}
