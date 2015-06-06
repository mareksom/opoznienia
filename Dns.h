#ifndef _DNS_H_
#define _DNS_H_

#include <vector>
#include <string>
#include <iostream>

#include "Data.h"
#include "DataReader.h"
#include "ConstIterable.h"
#include "MalformedPacket.h"

enum class DNS_Type : int
{
	NONE = 0,
	A = 1, NC = 2, MD = 3, MF = 4, CNAME = 5, SOA = 6, MB = 7, MG = 8,
	MR = 9, NULL_ = 10, WKS = 11, PTR = 12, HINFO = 13, MINFO = 14,
	MX = 15, TXT = 16
};

enum class DNS_QType : int
{
	NONE = 0,
	A = 1, NC = 2, MD = 3, MF = 4, CNAME = 5, SOA = 6, MB = 7, MG = 8,
	MR = 9, NULL_ = 10, WKS = 11, PTR = 12, HINFO = 13, MINFO = 14,
	MX = 15, TXT = 16,
	AXFR = 252, MAILB = 253, MAILA = 254, ALL = 255
};

enum class DNS_Class : int
{
	NONE = 0,
	IN = 1, CS = 2, CH = 3, HS = 4
};

enum class DNS_QClass : int
{
	NONE = 0,
	IN = 1, CS = 2, CH = 3, HS = 4,
	ALL = 255
};

class DNS_Name
{
public:
	DNS_Name();
	
	template<typename ...Args>
	DNS_Name(const std::string & label, const Args & ...args) : DNS_Name(args...)
	{
		labels.insert(labels.begin(), label);
	}

	DNS_Name(const std::vector< std::string > & labels);

	std::string getName() const;
	Data getData() const;

	unsigned readFromData(DataReader reader, unsigned pos);

	void swap(DNS_Name & x) noexcept;

	friend std::ostream & operator << (std::ostream & stream, const DNS_Name & name);

private:
	std::vector< std::string > labels;
};

class DNS_Question
{
public:
	DNS_QType getQType() const;
	DNS_QClass getQClass() const;
	DNS_Name getQName() const;

	void setQType(DNS_QType qtype);
	void setQClass(DNS_QClass qclass);
	void setQName(const DNS_Name & qname);

	Data getData() const;

	unsigned readFromData(DataReader reader, unsigned pos);

	friend std::ostream & operator << (std::ostream & stream, const DNS_Question & question);

private:
	DNS_Name qname;
	DNS_QType qtype;
	DNS_QClass qclass;
};

class DNS_RRecord
{
public:
	DNS_Name getName() const;
	DNS_Type getType() const;
	DNS_Class getClass() const;
	uint32_t getTTL() const;
	Data getRData() const;

	Data getData() const;

	unsigned readFromData(DataReader reader, unsigned pos);

	friend std::ostream & operator << (std::ostream & stream, const DNS_RRecord & record);

private:
	DNS_Name name;
	DNS_Type type;
	DNS_Class class_;
	uint32_t ttl;
	Data rdata;
};

enum class DNS_OPCode : int
{
	QUERY = 0, IQUERY = 1, STATUS = 2
};

enum class DNS_RCode : int
{
	NO_ERROR = 0, FORMAT_ERROR = 1, SERVER_FAILURE = 2,
	NAME_ERROR = 3, NOT_IMPLEMENTED = 4, REFUSED = 5
};

class DNS_Packet
{
public:
	DNS_Packet();
	DNS_Packet(DataReader reader, unsigned pos);

	Data getData() const;

	uint16_t getID() const;
	bool getQR() const;
	DNS_OPCode getOPCode() const;
	bool getAA() const;
	bool getTC() const;
	bool getRD() const;
	bool getRA() const;
	uint8_t getZ() const;
	DNS_RCode getRCode() const;
	ConstIterable< std::vector< DNS_Question > > getQD() const;
	ConstIterable< std::vector< DNS_RRecord > > getAN() const;
	ConstIterable< std::vector< DNS_RRecord > > getNS() const;
	ConstIterable< std::vector< DNS_RRecord > > getAR() const;
	unsigned getQDCount() const;
	unsigned getANCount() const;
	unsigned getNSCount() const;
	unsigned getARCount() const;

	void setID(uint16_t id);
	void setQR(bool qr);
	void setOPCode(DNS_OPCode opcode);
	void setAA(bool aa);
	void setTC(bool tc);
	void setRD(bool rd);
	void setRA(bool ra);
	void setZ(uint8_t);
	void setRCode(DNS_RCode rcode);
	void addQD(const DNS_Question & question);
	void addAN(const DNS_RRecord & record);
	void addNS(const DNS_RRecord & record);
	void addAR(const DNS_RRecord & record);

	friend std::ostream & operator << (std::ostream & stream, const DNS_Packet & packet);
	
private:
	unsigned id : 16;
	unsigned qr : 1;
	unsigned aa : 1;
	unsigned tc : 1;
	unsigned rd : 1;
	unsigned ra : 1;
	unsigned z : 3;
	DNS_OPCode opcode;
	DNS_RCode rcode;
	std::vector< DNS_Question > qd;
	std::vector< DNS_RRecord > an;
	std::vector< DNS_RRecord > ns;
	std::vector< DNS_RRecord > ar;
};

#endif
