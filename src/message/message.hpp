#pragma once

#include <string>
#include <cstdint>
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
	MSG_LEADER_ACTIVE
};

inline
const char* MSG_STR(uint8_t type) {
	switch (type) {
	case MSG_PING: return "MSG_PING";
	case MSG_PONG: return "MSG_PONG";
	case MSG_IDENT_REQUEST: return "MSG_IDENT_REQUEST";
	case MSG_IDENT: return "MSG_IDENT";
	case MSG_LEADER_ACTIVE: return "MSG_LEADER_ACTIVE";
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

	virtual std::unique_ptr<Message>
	clone() = 0;

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
