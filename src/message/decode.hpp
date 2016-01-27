#pragma once

#include <cassert>

#include "message.hpp"

int
decode_message(std::unique_ptr<Message>& m, uint8_t* src, int src_len);

int
decode_message_length(uint8_t* src, int src_len);
