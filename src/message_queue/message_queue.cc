#include "message_queue.hpp"

unique_message
Message_Queue :: pop() {
	std::unique_lock<std::mutex> lk(mutex);
	// Wait until there is a message.
	cv.wait(lk, [this]() {
		return queue.size() > 0;
	});
	unique_message m = std::move(queue.front());
	queue.pop();
	return std::move(m);
}

bool
Message_Queue :: pop_with_timeout(unique_message& m, int milliseconds) {
	std::unique_lock<std::mutex> lk(mutex);
	// Wait until there is a message or we timeout.
	if (cv.wait_for(lk, std::chrono::milliseconds((long)milliseconds),
		[this]() { return queue.size() > 0; })) {
		unique_message message = std::move(queue.front());
		queue.pop();
		m.swap(message);
		return true;
	}
	// Timed out
	return false;
}

void
Message_Queue :: push(unique_message m) {
	std::unique_lock<std::mutex> lk(mutex);
	queue.push(std::move(m));
	while (capacity > 0 && queue.size() > capacity) {
		queue.pop();
	}
	cv.notify_one();
}

size_t
Message_Queue :: size() {
	std::unique_lock<std::mutex> lk(mutex);
	return queue.size();
}
