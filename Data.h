#ifndef _DATA_H_
#define _DATA_H_

#include <vector>
#include <cstdint>
#include <iostream>

class Data
{
public:
	Data();
	Data(const std::vector<uint8_t> & data);
	Data(const uint8_t * msg, unsigned length);
	Data(const Data & data);

	uint8_t & getAt(unsigned pos);
	const uint8_t & getAt(unsigned pos) const;
	uint16_t & getAt16(unsigned pos);
	const uint16_t & getAt16(unsigned pos) const;
	uint32_t & getAt32(unsigned pos);
	const uint32_t & getAt32(unsigned pos) const;

	const uint8_t * getPtr() const;
	unsigned getLength() const;

	void append(const Data & data);
	void append(uint8_t value);
	void append(uint16_t value);
	void append(uint32_t value);
	void append(uint64_t value);

	template<typename Arg1, typename ...Args>
	void append(Arg1 value, Args ...args)
	{
		append(value);
		append(args...);
	}

	void prepend(const Data & data);

	void swap(Data & x) noexcept;

	friend std::ostream & operator << (std::ostream & stream, const Data & data);

private:
	std::vector<uint8_t> data;
};

#endif
