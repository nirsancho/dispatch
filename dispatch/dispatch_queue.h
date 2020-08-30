/*
 * dispatch_queue.h
 *
 *  Created on: 30 Aug 2020
 *      Author: nirsancho
 */

#pragma once
#include "safe_queue.h"

#include <functional>
#include <list>

namespace dispatch {

class DisptchQueue {
public:
	void registerHandler(Poller &poller) {
		poller.registerFd(_queue.getReadFd(), POLLIN,
				[this](int fd, Poller::poll_t event) {
					std::function<void()> f;
					_queue.pop(f, 0);
					if (f) {
						f();
					}
				});
	}

	void mainLoop(Poller &poller) {
		registerHandler(poller);
		while (_running) {
			poller.handle(-1);
		}
	}

	void dispatch(const std::function<void()> &f) {
		_queue.push(f);
	}

	void dispatchClose() {
		dispatch([this]() {
			_running = false;
		});
	}

	template<typename T>
	T sync(const std::function<T()> &f) {
		auto task_queue = getBlockingQueue();
		T v;
		dispatch([&v, &f, &task_queue]() {
			v = f();
			task_queue->push(true);
		});
		bool dummy = false;
		task_queue->pop(dummy, -1);
		assert(dummy);
		return v;
	}

//	template<typename T>
//	void async(const std::function<T()> &f, ) {
//
//	}

protected:
	SafeQueue<bool>* getBlockingQueue() {
		static __thread SafeQueue<bool>* bq = nullptr;
		if (bq == nullptr) {
			std::lock_guard<std::mutex> q(_blocking_queues_lock);
			auto _bg = std::make_shared<SafeQueue<bool>>();
			_blocking_queues.push_back(_bg);
			bq = _bg.get();
		}
		return bq;
	}

	SafeQueue<std::function<void()>> _queue;
	bool _running = true;

	std::vector<std::shared_ptr<SafeQueue<bool>>> _blocking_queues;
	std::mutex _blocking_queues_lock;
};

} // namespace dispatch
