#pragma once

#include <chrono>
#include <unordered_map>

#include "message.hpp"

class MessageTimes
{
	using time_point = std::chrono::steady_clock::time_point;
	using map_type = std::unordered_map<uint8_t, time_point>;
public:
	MessageTimes()
	{
	}

	void
	mark(MESSAGE_TYPE m_type)
	{
		m[m_type] = std::chrono::steady_clock::now();
	}

	int ms_since(MESSAGE_TYPE m_type)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - m[m_type]
		).count();
	}


private:
	map_type m;
}; // MessageTimes
