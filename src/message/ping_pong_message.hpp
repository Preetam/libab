#pragma once

#include <cassert>

#include "message.hpp"

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

	std::unique_ptr<Message>
	clone()
	{
		return std::make_unique<PingMessage>();
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

	std::unique_ptr<Message>
	clone()
	{
		return std::make_unique<PongMessage>();
	}
};

