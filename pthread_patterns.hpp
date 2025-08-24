#ifndef PTHREAD_PATTERNS_HPP
#define PTHREAD_PATTERNS_HPP


#include <atomic>
#include <cstdio>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <vector>

#include "PthTools.hpp"

namespace lf {
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
}

namespace pl {
	template<class Context, class Payload>
	class Pipeline {
		class ActiveObject;

	public:
		typedef ActiveObject *Stage;

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
					// Wait for work on queue
					pthread_mutex_lock(&self->task_mutex);
					while (self->workQueue.empty() && self->active)
						pthread_cond_wait(&self->task_cond, &self->task_mutex);
					if (!self->active) {
						pthread_mutex_unlock(&self->task_mutex);
						break;
					}

					// Consume work from queue
					const auto work = self->workQueue.front();
					self->workQueue.pop();
					pthread_mutex_unlock(&self->task_mutex);

					// Do work
					/*
					std::cout <<
							"worker [" << pthread_self() << "] starting work with- " <<
							"(c:" << work_context->context << ",p:" << *work_context->payload << ")" << std::endl;
					*/
					self->worker(work);

					// Exit point
					if (!self->active) {
						delete work;
						break;
					}
					// No more stages left, cleanup work
					if (work->stagesLeft.empty()) {
						delete work;
						break;
					}

					// Queue up work to next object
					const auto next_object = work->stagesLeft.front();
					work->stagesLeft.pop();
					next_object->enqueue(work);
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
			Work *work{};

			std::queue<Stage> stages = std::queue<Stage>();

		public:
			void setWork(Work *work) {
				delete work;
				this->work = work;
			}

			void setWork(Context context, Payload *payload) {
				setWork(new Work(context, payload));
			}

			void addStage(const Stage stage) { stages.push(stage); }

			void start() {
				if (stages.empty()) return;
				work->stagesLeft = stages;
				auto first_stage = stages.front();
				work->stagesLeft.pop();
				first_stage->enqueue(work);
			}
		};

		void destroy() {
			for (const auto object: activeObjects)
				delete object;
			activeObjects.clear();
		}

		~Pipeline() { destroy(); }
	};
}

#endif //PTHREAD_PATTERNS_HPP
