//
// Created by aviad on 6/22/25.
//

#include "fd_polling.hpp"


#include <pthread.h>
#include <poll.h>
#include <ranges>
#include <unistd.h>
#include <vector>


namespace reactor {
	// routine to run a reactor, sent to pthread_create
	/*
	static void *reactor_routine(void *arg) {
		const auto r = static_cast<Reactor *>(arg);
		while (true) {
			r->react();
			usleep(500);
		}
	}
	*/

	// list of global active reactors
	std::vector<fd_polling *> reactors;

	int fd_polling::start() {
		/*if (pthread_create(&thread, nullptr, reactor_routine, this) < 0) {
			perror("pthread_create");
			return -1;
		}
		if (pthread_mutex_init(&mutex, nullptr) < 0) {
			perror("pthread_mutex_init");
			return -1;
		}*/
		return 0;
	}

	void fd_polling::react() {
		// add fds to poll
		//pthread_mutex_lock(&mutex);
		std::vector<pollfd> fds;
		for (const auto &fd: fd_funcs | std::views::keys)
			fds.push_back({.fd = fd, .events = POLLIN, .revents = POLLIN});
		//pthread_mutex_unlock(&mutex);

		// poll for in
		poll(fds.data(), fd_funcs.size(), -1);

		// check for hot fds and fire callbacks
		//pthread_mutex_lock(&mutex);
		// clone callbacks in-case func modifies it
		auto funcs = fd_funcs;
		for (const auto fd: fds)
			if (fd.revents & POLLIN && funcs.contains(fd.fd))
				funcs[fd.fd](fd.fd);
		//pthread_mutex_unlock(&mutex);
	}

	int fd_polling::addFd(const fd_t fd, const fd_func callback) {
		//pthread_mutex_lock(&mutex);
		fd_funcs.insert(std::pair(fd, callback));
		//pthread_mutex_unlock(&mutex);
		return 0;
	}

	int fd_polling::removeFd(const fd_t fd) {
		//pthread_mutex_lock(&mutex);
		fd_funcs.erase(fd);
		//pthread_mutex_unlock(&mutex);
		return 0;
	}

	int fd_polling::stop() {
		/*if (pthread_cancel(thread) < 0) {
			perror("pthread_cancel");
			return -1;
		}
		if (pthread_join(thread, nullptr) < 0) {
			perror("pthread_join");
			return -1;
		}
		if (pthread_mutex_destroy(&mutex) < 0) {
			perror("pthread_mutex_destroy");
			return -1;
		}*/
		return 0;
	}

	fd_polling::~fd_polling() { stop(); }


	void *startReactor() {
		const auto reactor = new fd_polling();
		if (reactor->start() < 0) {
			perror("Failed to start reactor");
			return nullptr;
		}
		reactors.emplace_back(reactor);
		return reactor;
	}

	void pollReactor(void *reactor) {
		if (reactor == nullptr) return;
		static_cast<fd_polling *>(reactor)->react();
	}

	int addFdToReactor(void *reactor, const fd_t fd, const reactorFunc func) {
		if (reactor == nullptr) {
			perror("error adding fd to reactor: reactor is null");
			return -1;
		}
		return static_cast<fd_polling *>(reactor)->addFd(fd, func);
	}

	int removeFdFromReactor(void *reactor, const fd_t fd) {
		if (reactor == nullptr) {
			perror("error removing fd from reactor: reactor is null");
			return -1;
		}
		return static_cast<fd_polling *>(reactor)->removeFd(fd);
	}

	int stopReactor(void *reactor) {
		if (reactor == nullptr) return 0;
		const auto r = static_cast<fd_polling *>(reactor);
		if (r->stop() < 0) {
			perror("Failed to stop reactor");
			return -1;
		}
		std::erase(reactors, r);
		delete r;
		return 0;
	}

	void stopAllReactors() {
		for (const auto reactor: reactors)
			stopReactor(reactor);
		reactors.clear();
	}
}

namespace proactor {
	typedef runnable<proactorFunc, fd_t> clientRunnable;

	struct fd_proactor {
		const fd_t socket;
		const proactorFunc clientFunc;

		pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
		std::vector<pthread_t> client_threads;
	};

	std::map<pthread_t, fd_proactor *> proactors_threads;

	static void *proactor_routine(void *arg) {
		const auto proactor = (fd_proactor *) arg;

		while (true) {
			// wait to accept new clients (cancellation point)
			const fd_t client_fd = accept(proactor->socket, nullptr, nullptr);
			if (client_fd < 0) {
				perror("accept failed");
				if (errno != EINTR) usleep(1000);
				continue;
			}

			// client accepted

			// create client thread
			pthread_t thread;
			if (const auto cr = new clientRunnable{proactor->clientFunc, client_fd};
				pthread_create(
					&thread, nullptr,
					clientRunnable::run_routine, cr
				) != 0
			) {
				perror("client thread creation failed pthread_create");
				delete cr;
				continue;
			}
			printf("started client thread %lu for client fd %d\n", thread, client_fd);
			// track client thread
			pthread_mutex_lock(&proactor->client_mutex);
			proactor->client_threads.push_back(thread);
			pthread_mutex_unlock(&proactor->client_mutex);
		}
	}

	pthread_t startProactor(const fd_t sock_fd, const proactorFunc threadFunc) {
		for (const auto &p: proactors_threads | std::views::values)
			if (p->socket == sock_fd) {
				perror("proactor already exists for socket");
				return -1;
			}

		// create proactor accept thread
		pthread_t accept_thread;
		auto fdp = new fd_proactor{sock_fd, threadFunc};
		if (pthread_create(&accept_thread, nullptr, proactor_routine, fdp) != 0) {
			perror("pthread_create failed");
			delete fdp;
			return -1;
		}
		printf("started connection accept thread %lu for socket %d\n", accept_thread, sock_fd);
		// track new proactor
		proactors_threads.insert(std::pair(accept_thread, fdp));

		return accept_thread;
	}

	int stopProactor(const pthread_t t) {
		const auto it = proactors_threads.find(t);
		if (it == proactors_threads.end()) {
			perror("proactor not found for thread");
			return -1;
		}

		// stop proactor
		const auto [thread, proactor] = *it;

		printf("stopping connection accept thread %lu for socket %d\n", thread, proactor->socket);
		// cancel proactor socket thread
		if (pthread_cancel(thread) != 0) {
			perror("proactor cancel failed");
			return -1;
		}
		if (pthread_join(thread, nullptr) != 0) {
			perror("proactor join failed");
			return -1;
		}

		// stop and clear all proactor client threads
		pthread_mutex_lock(&proactor->client_mutex);
		for (const auto ct: proactor->client_threads) {
			printf("stopping client handle thread %lu\n", ct);
			if (pthread_cancel(ct) != 0) {
				perror("client pthread_cancel");
				continue;
			}
			if (pthread_join(ct, nullptr) != 0)
				perror("client pthread_join");
		}
		proactor->client_threads.clear();
		pthread_mutex_unlock(&proactor->client_mutex);

		// cleanup proactor
		pthread_mutex_destroy(&proactor->client_mutex);
		delete proactor;

		// remove proactor by thread key
		proactors_threads.erase(thread);

		return 0;
	}
}
