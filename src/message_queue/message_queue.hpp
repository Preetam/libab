// A thread-safe message queue.
#pragma once

#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "../message/message.hpp"

using unique_message = std::unique_ptr<Message>;

class Message_Queue
{
public:
	Message_Queue()
	: capacity(-1)
	{
	}

	Message_Queue(size_t capacity)
	: capacity(capacity)
	{
	}

	// Disable copying.
	Message_Queue(const Message_Queue&) = delete;
	Message_Queue& operator =(const Message_Queue&) = delete;

	// pop removes the first element of the queue.
	// If the queue is empty, this function blocks until
	// there is a new element.
	unique_message
	pop();

	// pop_with_timeout removes the first element of the queue.
	// If an element is not popped by the specified timeout (in
	// milliseconds), this function returns false and the
	// Message pointer is unchanged.
	bool
	pop_with_timeout(unique_message&, int);

	// push inserts an element into the queue.
	void
	push(unique_message m);

	// size returns the size of the queue.
	// Note that the size may have changed
	// immediately after this function is called.
	size_t
	size();

private:
	std::mutex mutex;
	std::condition_variable cv;
	std::queue<unique_message> queue;
	size_t capacity;
}; // Message_Queue
