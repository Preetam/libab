#include <memory>
#include <cstring>

#include "message.hpp"
#include "identity_message.hpp"
#include "ping_pong_message.hpp"
#include "leader_message.hpp"

/**
 * A message header has 4 fields:
 * - length (4 bytes)
 * - type (1 byte)
 * - flags (1 byte)
 * - id (8 bytes)
 * Total: 14 bytes
 */
#define MSG_HEADER_SIZE 14

int
Message :: pack(uint8_t* dest, int dest_len) const {
	int length = body_size() + MSG_HEADER_SIZE;
	if (dest_len < length) {
		return -1;
	}

	write32be(length, dest);
	dest += 4;
	dest_len -= 4;
	write8be(type, dest);
	dest++;
	dest_len--;
	write8be(flags, dest);
	dest++;
	dest_len--;
	write64be(message_id, dest);
	dest += 8;
	dest_len -= 8;

	int status = pack_body(dest, dest_len);
	if (status < 0) {
		return -2;
	}
	return length;
}

int
Message :: packed_size() const {
	return body_size() + MSG_HEADER_SIZE;
}

int
Message :: unpack(uint8_t* src, int src_len) {
	if (src_len < MSG_HEADER_SIZE) {
		return -1;
	}

	uint32_t length = read32be(src);
	src += 4;
	if (src_len < length) {
		return -2;
	}
	type = read8be(src);
	src++;
	flags = read8be(src);
	src++;
	message_id = read64be(src);
	src += 8;
	unpack_body(src, length);
	return 0;
}

int
decode_message(std::unique_ptr<Message>& m, uint8_t* src, int src_len) {
	assert(src_len >= 5);
	// Peek at the message type
	switch (src[4]) {
	case MSG_PING:
		m = std::make_unique<PingMessage>();
		break;
	case MSG_PONG:
		m = std::make_unique<PongMessage>();
		break;
	case MSG_IDENT_REQUEST:
		m = std::make_unique<IdentityRequest>();
		break;
	case MSG_IDENT:
		m = std::make_unique<IdentityMessage>();
		break;
	case MSG_LEADER_ACTIVE:
		m = std::make_unique<LeaderActiveMessage>();
		break;
	default:
		return -1;
	}

	return m->unpack(src, src_len);
}

int
decode_message_length(uint8_t* src, int src_len) {
	if (src_len < 4) {
		return -1;
	}
	uint32_t length = read32be(src);
	return (int)length;
}

#undef MSG_HEADER_SIZE
