#pragma once

#include <cassert>
#include <cstring>
#include <cstdint>
#include <string>
#include <random>
#include <chrono>

#include "encoding.h"

enum MESSAGE_TYPE : uint8_t
{
	// Invalid
	MSG_INVALID,
	// Ping
	MSG_PING,
	// Ping response (pong)
	MSG_PONG,
	// Identification request
	MSG_IDENT_REQUEST,
	// Identification
	MSG_IDENT,
	// Active leader
	MSG_LEADER_ACTIVE,
	// Active leader acknowledgement
	MSG_LEADER_ACTIVE_ACK
};

inline
const char* MSG_STR(uint8_t type) {
	switch (type) {
	case MSG_PING: return "MSG_PING";
	case MSG_PONG: return "MSG_PONG";
	case MSG_IDENT_REQUEST: return "MSG_IDENT_REQUEST";
	case MSG_IDENT: return "MSG_IDENT";
	case MSG_LEADER_ACTIVE: return "MSG_LEADER_ACTIVE";
	case MSG_LEADER_ACTIVE_ACK: return "MSG_LEADER_ACTIVE_ACK";
	}

	return "MSG_INVALID";
}

enum MESSAGE_FLAG
{
	FLAG_BCAST  = 1 << 0,
	FLAG_RBCAST = 1 << 1
};

// Initialize RNG
static std::mt19937_64 rng(
	std::chrono::system_clock::now().time_since_epoch().count());

class Message
{
public:
	Message()
	: type(MSG_INVALID)
	{
	}

	Message(uint8_t type)
	: type(type), flags(0), message_id(rng())
	{
	}

	Message(uint8_t type, uint8_t flags)
	: type(type), flags(flags), message_id(rng())
	{
	}

	int
	pack(uint8_t* dest, int dest_len) const;

	int
	packed_size() const;

	int
	unpack(uint8_t* src, int src_len);

	inline bool
	broadcast()
	{
		return flags & (FLAG_BCAST|FLAG_RBCAST);
	}

	inline bool
	rbroadcast()
	{
		return flags & FLAG_RBCAST;
	}

	// Virtual methods to override
	virtual int
	body_size() const
	{
		return 0;
	}

	virtual int
	pack_body(uint8_t* dest, int dest_len) const = 0;

	virtual int
	unpack_body(uint8_t* src, int src_len) = 0;

public:
	int source;
	uint8_t type;
	uint8_t flags;
	uint64_t message_id;
};

class IdentityRequest : public Message
{
public:
	IdentityRequest()
	: Message(MSG_IDENT_REQUEST), id(0), address("")
	{
	}

	IdentityRequest(uint64_t id, std::string& address)
	: Message(MSG_IDENT_REQUEST), id(id), address(address)
	{
	}

	inline int
	body_size()
	{
		return 8 + 2 + address.size();
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) const
	{
		if (dest_len < 8 + 2 + address.size()) {
			return -1;
		}
		write64be(id, dest);
		dest += 8;
		write16be(address.size(), dest);
		dest += 2;
		memcpy(dest, address.c_str(), address.size());
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		if (src_len < 8 + 2) {
			return -1;
		}
		id = read64be(src);
		src += 8;
		uint16_t address_size = read16be(src);
		src += 2;
		if (src_len < 8 + 2 + address_size) {
			return -2;
		}
		address = std::string((const char*)src, address_size);
		return 0;
	}

public:
	uint64_t id;
	std::string address;
};

class IdentityMessage : public Message
{
public:
	IdentityMessage()
	: Message(MSG_IDENT), id(0), address("")
	{
	}

	IdentityMessage(uint64_t id, std::string& address)
	: Message(MSG_IDENT), id(id), address(address)
	{
	}

	inline int
	body_size() const
	{
		return 8 + 2 + address.size();
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) const {
		if (dest_len < 8 + 2 + address.size()) {
			return -1;
		}
		write64be(id, dest);
		dest += 8;
		write16be(address.size(), dest);
		dest += 2;
		memcpy(dest, address.c_str(), address.size());
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		if (src_len < 8 + 2) {
			return -1;
		}
		id = read64be(src);
		src += 8;
		uint16_t address_size = read16be(src);
		src += 2;
		if (src_len < 8 + 2 + address_size) {
			return -2;
		}
		address = std::string((const char*)src, address_size);
		return 0;
	}

public:
	uint64_t id;
	std::string address;
};

class PingMessage : public Message
{
public:
	PingMessage()
	: Message(MSG_PING)
	{
	}

	inline int
	body_size()
	{
		return 0;
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) const
	{
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		return 0;
	}
};

class PongMessage : public Message
{
public:
	PongMessage()
	: Message(MSG_PONG)
	{
	}

	inline int
	body_size()
	{
		return 0;
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) const
	{
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		return 0;
	}
};


class LeaderActiveMessage : public Message
{
public:
	LeaderActiveMessage()
	: Message(MSG_LEADER_ACTIVE)
	, id(0)
	, seq(0)
	{
	}

	LeaderActiveMessage(uint64_t id, uint64_t seq)
	: Message(MSG_LEADER_ACTIVE)
	, id(id)
	, seq(seq)
	{
	}

	inline int
	body_size() const
	{
		return 16;
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) const
	{
		if (dest_len < 8+8) {
			return -1;
		}
		write64be(id, dest);
		dest += 8;
		write64be(seq, dest);
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		if (src_len < 8+8) {
			return -1;
		}
		id = read64be(src);
		src += 8;
		seq = read64be(src);
		return 0;
	}

public:
	uint64_t id;
	uint64_t seq;
};

class LeaderActiveAck : public Message
{
public:
	LeaderActiveAck()
	: Message(MSG_LEADER_ACTIVE_ACK)
	{
	}

	inline int
	body_size() const
	{
		return 0;
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) const
	{
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		return 0;
	}
};

