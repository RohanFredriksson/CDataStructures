#include <stdint.h>

#ifndef HASH_H
#define HASH_H

uint64_t OAAT(const char* in);
uint64_t SIP64(const uint8_t* in, const size_t inlen, uint64_t seed0, uint64_t seed1);

#endif