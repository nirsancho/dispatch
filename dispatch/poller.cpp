/*
 * Poller.cpp
 *
 *  Created on: 30 Aug 2020
 *      Author: nirsancho
 */

#include "poller.h"

namespace dispatch {

bool Poller::registerFd(int fd, Poller::poll_t event,
		const callback_type &callback) {

	const auto it = std::find_if(_pollfds.begin(), _pollfds.end(),
			[fd](const auto &item) {
				return item.fd == fd;
			});
	if (it == _pollfds.end()) {
		_pollfds.resize(_pollfds.size() + 1);
		auto &pollevent = _pollfds.back();
		pollevent.fd = fd;
		pollevent.events = event;
	} else {
		it->events |= event;
	}

	if (_callbacks.count(fd) != 0) {
		return false;
	}
	_callbacks.emplace(fd, callback);
	return true;
}

bool Poller::unregisterFd(int fd) {
	{
		const auto it = std::find_if(_pollfds.begin(), _pollfds.end(),
				[fd](const auto &item) {
					return item.fd == fd;
				});
		if (it != _pollfds.end()) {
			_pollfds.erase(it);
		}
	}
	{
		const auto it = _callbacks.find(fd);
		if (it == _callbacks.end()) {
			return false;
		} else {
			_callbacks.erase(it);
			return true;
		}
	}
}

int Poller::wait(int timeout_ms,
		std::vector<std::pair<int, Poller::poll_t>> &ready_fds) {
	int ret = poll(_pollfds.data(), _pollfds.size(), timeout_ms);
	if (ret > 0) {
		ready_fds.reserve(ret);
		for (size_t i = 0; i < _pollfds.size(); ++i) {
			if (_pollfds[i].revents) {
				ready_fds.push_back(
						std::make_pair(_pollfds[i].fd, _pollfds[i].revents));
			}
		}
	}
	return ret;
}

int Poller::handle(int timeout_ms) {
	std::vector<std::pair<int, Poller::poll_t>> ready_fds;
	int ret = wait(timeout_ms, ready_fds);
	for (const auto &f : ready_fds) {
		const auto &it = _callbacks.find(f.first);
		if (it != _callbacks.end() && it->second) {
			it->second(f.first, f.second);
		}
	}
	return ret;
}

} /* namespace dispatch */
