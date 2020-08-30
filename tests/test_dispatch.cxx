/*
 * test_dispatch.cxx
 *
 *  Created on: 29 Aug 2020
 *      Author: nirsancho
 */
#include "../dispatch/safe_queue.h"
#include "../dispatch/dispatch_queue.h"

#include <thread>
#include <chrono>

using namespace dispatch;
#define NITEMS 50000
#define TIMEOUT 10

struct Tiktok {
	Tiktok(const std::string &label) :
			_label(label), _s(std::chrono::steady_clock::now()) {
	}

	~Tiktok() {
		auto ms = elapsed();
		printf("%s: %fms\n", _label.c_str(), ms.count() * 1000);
	}
//
	std::chrono::duration<double> elapsed() {
		return (std::chrono::steady_clock::now() - _s);
	}

private:
	const std::string _label;
	const std::chrono::time_point<std::chrono::steady_clock> _s;
};

void test_safe_queue() {
	SafeQueue<int> q;
	std::thread t1([&q]() {
		Tiktok t("t1");
		for (int i = 0; i < 2 * NITEMS; i++) {
			q.push(2 * i);
		}
	});

	std::thread t2([&q]() {
		Tiktok t("t2");
		for (int i = 0; i < NITEMS; i++) {
			q.push(2 * i + 1);
		}
	});

	std::thread t3([&q]() {
		Tiktok t("t3");
		int items = 0;
		int v;
		while (true) {
			if (!q.pop(v, TIMEOUT)) {
				break;
			}
			items++;
		}
		printf("t3 read %d items\n", items);
	});

//	std::thread t4([&q]() {
//		Tiktok t("t4");
//		int items = 0;
//		int v;
//		while (true) {
//			if (!q.pop(v, TIMEOUT)) {
//				break;
//			}
//			items++;
//		}
//		printf("t4 read %d items\n", items);
//	});
//
//	std::thread t5([&q]() {
//		Tiktok t("t5");
//		int items = 0;
//		int v;
//		while (true) {
//			if (!q.pop(v, TIMEOUT)) {
//				break;
//			}
//			items++;
//		}
//		printf("t5 read %d items\n", items);
//	});

	t1.join();
	t2.join();
	t3.join();
//	t4.join();
//	t5.join();
}
void test_dispatch_queue() {

	Poller mainpoller;
	DisptchQueue bgq;
	DisptchQueue mainq;
	std::thread bg([&bgq]() {
		Poller poller;
		bgq.mainLoop(poller);
	});

	Poller poller;
	mainq.dispatch([&bgq, &mainq]() {
		int i = bgq.sync<int>([]() {
			return 10;
		});
		bgq.dispatchClose();
		mainq.dispatchClose();
	});
	mainq.mainLoop(poller);
	bg.join();

}
int main() {
//	test_safe_queue();
	test_dispatch_queue();
	return 0;
}

