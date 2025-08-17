#include "pthread_patterns.hpp"

namespace lf {
	void *LF::thread_func(void *arg) {
		const auto lf = (LF *) arg;

		while (lf->running) {
			// Follow leader
			pthread_mutex_lock(&lf->leader_mutex);
			while (lf->leader_set && lf->running)
				pthread_cond_wait(&lf->leader_changed_cond, &lf->leader_mutex);
			if (!lf->running) {
				pthread_mutex_unlock(&lf->leader_mutex);
				break;
			}
			lf->leader_set = true;
			pthread_mutex_unlock(&lf->leader_mutex);

			// Wait for tasks
			pthread_mutex_lock(&lf->tasks_mutex);
			while (lf->task_queue.empty() && lf->running)
				pthread_cond_wait(&lf->tasks_changed_cond, &lf->tasks_mutex);
			if (!lf->running && lf->task_queue.empty()) {
				pthread_mutex_unlock(&lf->tasks_mutex);
				pthread_mutex_lock(&lf->leader_mutex);
				lf->leader_set = false;
				pthread_mutex_unlock(&lf->leader_mutex);
				break;
			}
			// Consume task
			auto tasks = lf->task_queue.front();
			lf->task_queue.pop();
			pthread_mutex_unlock(&lf->tasks_mutex);

			// Abdicate leadership
			pthread_mutex_lock(&lf->leader_mutex);
			lf->leader_set = false;
			pthread_mutex_unlock(&lf->leader_mutex);
			pthread_cond_signal(&lf->leader_changed_cond);

			// Run tasks
			pthread_mutex_lock(&lf->tasks_mutex);
			++lf->working_threads;
			pthread_mutex_unlock(&lf->tasks_mutex);
			for (auto &task: tasks)
				task();
			// Notify task complete
			pthread_mutex_lock(&lf->tasks_mutex);
			--lf->working_threads;
			pthread_mutex_unlock(&lf->tasks_mutex);
			pthread_cond_broadcast(&lf->tasks_changed_cond);
		}

		// wake any blocked threads
		pthread_cond_broadcast(&lf->leader_changed_cond);
		pthread_cond_broadcast(&lf->tasks_changed_cond);
		return nullptr;
	}

	LF::LF(const int thread_count) {
		thread_pool.reserve(thread_count);
		for (int i = 0; i < thread_count; i++)
			thread_pool.emplace_back(0);
	}

	int LF::start() {
		running = true;
		for (pthread_t &pid: thread_pool) {
			if (pthread_create(&pid, nullptr, thread_func, this) != 0) {
				perror("pthread_create");
				stop();
				return -1;
			}
		}
		return 0;
	}

	void LF::stop() {
		running = false;
		pthread_cond_broadcast(&leader_changed_cond);
		pthread_cond_broadcast(&tasks_changed_cond);

		for (pthread_t &pid: thread_pool) {
			if (pid != 0) {
				if (pthread_join(pid, nullptr) != 0)
					perror("pthread_join");
				pid = 0;
			}
		}

		pthread_mutex_destroy(&tasks_mutex);
		pthread_mutex_destroy(&leader_mutex);
		pthread_cond_destroy(&tasks_changed_cond);
		pthread_cond_destroy(&leader_changed_cond);
	}

	int LF::run(const std::vector<Func> &tasks) {
		pthread_mutex_lock(&tasks_mutex);
		task_queue.push(tasks);
		pthread_cond_broadcast(&tasks_changed_cond);
		pthread_mutex_unlock(&tasks_mutex);
		return 0;
	}

	void LF::complete() {
		pthread_mutex_lock(&tasks_mutex);
		while (!task_queue.empty() || working_threads > 0)
			pthread_cond_wait(&tasks_changed_cond, &tasks_mutex);
		pthread_mutex_unlock(&tasks_mutex);
	}
}

namespace pl {
}
