#ifndef LF_HPP
#define LF_HPP
#include <atomic>
#include <cstdio>
#include <pthread.h>
#include <queue>
#include <vector>

#include "PthTools.hpp"

class LF {
	std::atomic<bool> running = true;

	std::queue<std::vector<Func> > task_queue = std::queue<std::vector<Func> >();

	std::vector<pthread_t> thread_pool = std::vector<pthread_t>();

	bool leader_set = false;
	int working_threads = 0;

	pthread_cond_t leader_changed_cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t leader_mutex = PTHREAD_MUTEX_INITIALIZER;

	pthread_cond_t tasks_changed_cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t tasks_mutex = PTHREAD_MUTEX_INITIALIZER;

	static void *thread_func(void *arg);

public:
	explicit LF(int thread_count = 4);

	int start();

	~LF() { stop(); }

	void stop();

	int run(const std::vector<Func> &tasks);

	int run(const Func &task) { return run(std::vector{task}); }

	void complete();
};


#endif //LF_HPP
