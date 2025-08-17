//
// Created by aviad on 7/27/25.
//

#ifndef PIPELINE_H
#define PIPELINE_H
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <pthread.h>
#include <queue>
#include <bits/pthreadtypes.h>


template<class Context, class Payload>
class Pipeline {
	class ActiveObject;
	typedef ActiveObject *Stage;

public:
	struct Work {
		Context context;
		Payload *payload;

		std::queue<Stage> stagesLeft = std::queue<Stage>();


		explicit Work(Context context, Payload *payload) : context(context), payload(payload) {
		}

		~Work() { delete payload; }
	};

	typedef void (*Worker)(const Work *);

private:
	class ActiveObject {
		const Worker worker;
		std::queue<Work *> workQueue = std::queue<Work *>();

		std::atomic_bool active = false;
		pthread_t thread{};
		pthread_mutex_t task_mutex = PTHREAD_MUTEX_INITIALIZER;
		pthread_cond_t task_cond = PTHREAD_COND_INITIALIZER;

		static void *thread_func(void *arg) {
			const auto self = (ActiveObject *) arg;

			while (self->active) {
				// wait for work on queue
				pthread_mutex_lock(&self->task_mutex);
				while (self->workQueue.empty() && self->active)
					pthread_cond_wait(&self->task_cond, &self->task_mutex);
				if (!self->active) {
					pthread_mutex_unlock(&self->task_mutex);
					break;
				}

				// consume work from queue
				const auto work_context = self->workQueue.front();
				self->workQueue.pop();
				pthread_mutex_unlock(&self->task_mutex);

				// do work
				self->worker(work_context);

				if (!self->active) {
					delete work_context;
					break;
				}

				// send work to next object
				if (work_context->stagesLeft.empty())
					delete work_context;
				else {
					const auto next_object = work_context->stagesLeft.front();
					work_context->stagesLeft.pop();
					next_object->enqueue(work_context);
				}
			}

			return nullptr;
		}

	public:
		explicit ActiveObject(const Worker worker) : worker(worker) {
		}

		~ActiveObject() { stop(); }

		int start() {
			active = true;
			if (pthread_create(&thread, nullptr, thread_func, this) != 0) {
				perror("pthread_create");
				active = false;
				return -1;
			}
			return 0;
		}

		void enqueue(Work *work) {
			pthread_mutex_lock(&task_mutex);
			workQueue.push(work);
			pthread_mutex_unlock(&task_mutex);
			pthread_cond_signal(&task_cond);
		}

		int stop() {
			active = false;
			if (pthread_cond_signal(&task_cond) != 0) {
				perror("pthread_cond_signal");
			}
			if (pthread_join(thread, nullptr) != 0) {
				perror("pthread_join");
				return -1;
			}
			pthread_mutex_destroy(&task_mutex);
			pthread_cond_destroy(&task_cond);
			return 0;
		}
	};

	std::vector<ActiveObject *> activeObjects = std::vector<ActiveObject *>();

public:
	Stage startActiveObject(const Worker worker) {
		const auto object = new ActiveObject(worker);
		activeObjects.push_back(object);
		object->start();
		return object;
	}

	void removeActiveObject(Stage object) {
		const auto it = std::ranges::find(activeObjects, object);
		if (it != activeObjects.end()) {
			activeObjects.erase(it);
			object->stop();
			delete object;
		}
	}

	class Job {
		Work *work_context{};

		std::queue<Stage> stages = std::queue<Stage>();

	public:
		void setContext(Work *context) { this->work_context = context; }

		void addStage(const Stage state) { stages.push(state); }

		void start() {
			if (stages.empty()) return;
			work_context->stagesLeft = stages;
			auto first_stage = stages.front();
			work_context->stagesLeft.pop();
			first_stage->enqueue(work_context);
		}
	};

	void destroy() {
		for (const auto object: activeObjects)
			delete object;
		activeObjects.clear();
	}

	~Pipeline() { destroy(); }
};


#endif //PIPELINE_H
