#ifndef _UTILS_H_
#define _UTILS_H_

#include <cstdint>

uint64_t htonll(uint64_t x);
uint64_t ntohll(uint64_t x);

uint64_t getMicroTime();

uint16_t OnesComplementSum(uint16_t a, uint16_t b);
uint16_t OnesComplementSum(uint16_t * data, unsigned length);

int randomInt(int a, int b);

uint32_t BCD(uint32_t value);
uint32_t BCDrev(uint32_t value);

#endif
