#include "catch.hpp"
#include <cstring>

#include "message/encoding.h"

TEST_CASE("encoding", "[encoding]") {
	uint8_t data[4];
	write32be(0x12345678, data);
	uint8_t expected[4] = {0x78, 0x56, 0x34, 0x12};
	REQUIRE(memcmp(data, expected, 4) == 0);
	REQUIRE(read32be(data) == 0x12345678);
}
