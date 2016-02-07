#pragma once

#include <cstring>
#include <cassert>

#include "message.hpp"

class LeaderActiveMessage : public Message
{
public:
	LeaderActiveMessage()
	: Message(MSG_LEADER_ACTIVE), id(0)
	{
	}

	LeaderActiveMessage(uint64_t id)
	: Message(MSG_LEADER_ACTIVE), id(id)
	{
	}

	inline int
	body_size() const
	{
		return 8;
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) const
	{
		if (dest_len < 8) {
			return -1;
		}
		write64be(id, dest);
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		if (src_len < 8) {
			return -1;
		}
		id = read64be(src);
		return 0;
	}

public:
	uint64_t id;
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
