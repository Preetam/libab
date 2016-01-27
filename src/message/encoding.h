#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void write8be(uint8_t, uint8_t*);
void write16be(uint16_t, uint8_t*);
void write32be(uint32_t, uint8_t*);
void write64be(uint64_t, uint8_t*);

uint8_t read8be(uint8_t*);
uint16_t read16be(uint8_t*);
uint32_t read32be(uint8_t*);
uint64_t read64be(uint8_t*);

inline void
write8be(uint8_t v, uint8_t* dest) {
	*dest = v;
}

inline uint8_t
read8be(uint8_t* src) {
	return *src;
}

inline void
write16be(uint16_t v, uint8_t* dest) {
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
}

inline uint16_t
read16be(uint8_t* src) {
	uint16_t v = *src;
	src++;
	v |= uint64_t(*src) << (8*1);
	return v;
}

inline void
write32be(uint32_t v, uint8_t* dest) {
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
}

inline uint32_t
read32be(uint8_t* src) {
	uint32_t v = *src;
	src++;
	v |= uint32_t(*src) << 8;
	src++;
	v |= uint32_t(*src) << 16;
	src++;
	v |= uint32_t(*src) << 24;
	return v;
}

inline void
write64be(uint64_t v, uint8_t* dest) {
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
	dest++;
	v >>= 8;
	*dest = uint8_t(v);
}

inline uint64_t
read64be(uint8_t* src) {
	uint64_t v = *src;
	src++;
	v |= uint64_t(*src) << (8*1);
	src++;
	v |= uint64_t(*src) << (8*2);
	src++;
	v |= uint64_t(*src) << (8*3);
	src++;
	v |= uint64_t(*src) << (8*4);
	src++;
	v |= uint64_t(*src) << (8*5);
	src++;
	v |= uint64_t(*src) << (8*6);
	src++;
	v |= uint64_t(*src) << (8*7);
	return v;
}

#ifdef __cplusplus
}
#endif
