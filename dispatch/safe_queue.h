/*
 * safe_queue.h
 *
 *  Created on: 29 Aug 2020
 *      Author: nirsancho
 */

#pragma once

#include "poller.h"

#include <queue>
#include <unistd.h> // pipe
#include <fcntl.h>
#include <string>
#include <mutex>

#include <errno.h>
#include <stdexcept>

namespace dispatch {

template<typename T>
class SafeQueue {
protected:
	std::mutex _lock;
	std::queue<T> _queue;
	Poller _poller;

	int _fd_read;
	int _fd_write;

	void notify() {
		size_t s = _queue.size();
		write(_fd_write, &s, sizeof(s));
	}

	bool pop(T &v) { // non blocking pop
		size_t s = 0;
		ssize_t read_ret = 0;
		read_ret = read(_fd_read, &s, sizeof(s));

		if (read_ret == sizeof(s)) {
			v = _queue.front();
			_queue.pop();
			return true;
		} else {
			return false;
		}
	}

public:
	SafeQueue() {
		printf("SafeQueue()\n");
		int pipefds[2];
		int ret = pipe(pipefds);
		if (ret != 0) {
			throw std::runtime_error(
					std::string("failed to init pipe: ")
							+ std::string(strerror(errno)));
		}

		_fd_read = pipefds[0];
		_fd_write = pipefds[1];
		fcntl(_fd_read, F_SETFL, fcntl(_fd_read, F_GETFL) | O_NONBLOCK); // set read to non-blocking mode

		_poller.registerFd(_fd_read, POLLIN, { });

	}

	~SafeQueue() {
		printf("~SafeQueue\n");
		close(_fd_read);
		close(_fd_write);
	}

	void clear() {
		std::lock_guard<std::mutex> g(_lock);
		T dummy;
		while (!_queue.empty()) {
			pop(dummy);
		}
	}

	void push(const T &v) {
		size_t s = 0;
		{
			std::lock_guard<std::mutex> g(_lock);
			_queue.push(v);
			s = _queue.size();
		}
		write(_fd_write, &s, sizeof(s));
	}

	bool pop(T &v, int timeout_ms) { // use timeout == -1 for blocking pop or timeout == 0 for non_blocking pop
		if (timeout_ms == 0) {
			std::lock_guard<std::mutex> g(_lock);
			return pop(v);
		} else {
			std::vector<std::pair<int, Poller::poll_t>> ready_fds;
			int ret = _poller.wait(timeout_ms, ready_fds);
			if (ret > 0) {
				std::lock_guard<std::mutex> g(_lock);
				return pop(v);
			} else {
				return false;
			}
		}
	}

	int getReadFd() {
		return _fd_read;
	}

};

} // namespace dispatch
