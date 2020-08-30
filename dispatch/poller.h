/*
 * Poller.h
 *
 *  Created on: 30 Aug 2020
 *      Author: nirsancho
 */

#pragma once
#include <poll.h>
#include <functional>
#include <vector>
#include <map>

namespace dispatch {

class Poller {

public:
	typedef short poll_t;
	typedef std::function<void(int, poll_t)> callback_type;

	bool registerFd(int fd, poll_t event, const callback_type& callback);
	bool unregisterFd(int fd);

	int wait(int timeout_ms, std::vector<std::pair<int, Poller::poll_t>>& ready_fds);
	int handle(int timeout_ms);

protected:
	std::map<int, callback_type> _callbacks;
	std::vector<pollfd> _pollfds;

};

} /* namespace dispatch */

