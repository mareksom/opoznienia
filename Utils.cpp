#include "Utils.h"

#include <chrono>
#include <arpa/inet.h>
#include <vector>
#include <random>
#include <algorithm>

uint64_t htonll(uint64_t x)
{
	return (1 == htonl(1)) ? x : ((uint64_t) htonl(x & 0xFFFFFFFF) << 32) | htonl(x >> 32);
}
uint64_t ntohll(uint64_t x)
{
	return (1 == ntohl(1)) ? x : ((uint64_t) ntohl(x & 0xffffffff) << 32) | ntohl(x >> 32);
}

uint64_t getMicroTime()
{
	return (uint64_t) std::chrono::duration_cast< std::chrono::microseconds > (
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}

uint16_t OnesComplementSum(uint16_t a, uint16_t b)
{
	uint32_t result = (uint32_t) a + (uint32_t) b;
	result = (uint16_t) (result >> 16) + (uint16_t) result;
	if(result == ((uint32_t) 1 << 17) - 1)
		result = 0;
	return result;
}

uint16_t OnesComplementSum(uint16_t * data, unsigned length)
{
	uint16_t sum = 0;
	for(unsigned i = 0; i < length; i++)
		sum = OnesComplementSum(sum, data[i]);
	return sum;
}

int randomInt(int a, int b)
{
	static std::random_device rd;
	static std::default_random_engine engine(rd());
	std::uniform_int_distribution<int> uniformDist(a, b);
	return uniformDist(engine);
}

uint32_t BCD(uint32_t value)
{
	std::vector<uint8_t> v;
	while(value)
	{
		v.push_back((uint8_t) (value % 10));
		value /= 10;
	}
	std::reverse(v.begin(), v.end());
	for(uint8_t i : v)
		value = (value << 4) | i;
	return value;
}

uint32_t BCDrev(uint32_t value)
{
	std::vector<uint8_t> v;
	while(value)
	{
		v.push_back((uint8_t) value & 15);
		value >>= 4;
	}
	std::reverse(v.begin(), v.end());
	for(uint8_t i : v)
		value = (value * 10) + i;
	return value;
}
