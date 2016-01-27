#include "catch.hpp"
#include <cstring>
#include <string>

#include "message/message.hpp"

// TODO: fix this.
// TEST_CASE("message", "[message]") {
// 	std::string data("\x01\x02\x03\x04");
// 	Message m;
// 	m.type = 0x99;
// 	m.data = data;
// 	uint8_t packed[10];
// 	int status = m.pack(packed, 10);
// 	REQUIRE(status > 0);

// 	status = m.unpack(packed, 10);
// 	REQUIRE(status == 0);
// 	REQUIRE(m.type == 0x99);
// 	REQUIRE(m.data == data);
// }
