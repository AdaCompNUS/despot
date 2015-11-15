#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

/* Compile with -lpthread*/

namespace despot {

class Timer {
private:
	pthread_t timer;
	int millis;
	bool done;

	void* timing(void* arg) {
		usleep(millis * 1000);
		done = true;
	}

	static void* helper(void* context) {
		return ((Timer*) context)->timing(NULL);
	}

public:
	Timer(int millis) {
		this->millis = millis;
		done = true;
	}

	~Timer() {
		//pthread_join(timer, NULL);
	}

	void start() {
		done = false;
		int status = pthread_create(&timer, NULL, &Timer::helper, this);
		assert(status == 0);
	}

	bool finished() {
		return done;
	}
};

} // namespace despot

#endif
