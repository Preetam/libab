#pragma once

#include <string>
#include <cassert>
#include <cstdlib>
#include <iostream>

#include "message.hpp"

const int KEY_SIZE = 32;

class Codec {
public:
	Codec()
	: m_key("")
	{
	}

	int
	set_key(const std::string& key)
	{
		if (key == "") {
			m_key = "";
			return 0;
		}
		if (key.size() != KEY_SIZE) {
			return -1;
		}
		m_key = key;
		return 0;
	}

	int
	pack_message(const Message* m, uint8_t* dest, int dest_len);

	int
	decode_message(std::unique_ptr<Message>& m, uint8_t* src, int src_len);

	int
	decode_message_length(uint8_t* src, int src_len);

private:
	std::string m_key;
}; // Codec
