//
// Created by aviad on 6/22/25.
//

#ifndef FD_POLLING_H
#define FD_POLLING_H

#include <cstdio>
#include <functional>
#include <map>
#include <pthread.h>
#include <vector>
#include <bits/pthreadtypes.h>
#include <sys/socket.h>

typedef int fd_t;

typedef void * (*fd_func)(fd_t fd);


namespace reactor {
	typedef fd_func reactorFunc;


	// Reactor struct
	class fd_polling {
		std::map<fd_t, fd_func> fd_funcs;
		//pthread_t thread{};
		//pthread_mutex_t mutex{};

	public:
		int start();

		void react();

		int addFd(fd_t fd, fd_func callback);

		int removeFd(fd_t fd);

		int stop();

		fd_polling() = default;

		~fd_polling();
	};


	// starts new reactor and returns pointer to it
	void *startReactor();

	// runs the reactor. polls for hot fds and fires associated callback functions
	void pollReactor(void *reactor);

	// adds fd to Reactor (for reading) ; returns 0 on success.
	int addFdToReactor(void *reactor, fd_t fd, reactorFunc func);

	// removes fd from reactor
	int removeFdFromReactor(void *reactor, fd_t fd);

	// stops reactor
	int stopReactor(void *reactor);

	// destroys library and stops all reactors
	void stopAllReactors();
}

namespace proactor {
	typedef fd_func proactorFunc;

	template<typename Func, typename Arg>
	struct runnable {
		Func func;
		Arg arg;

		void *operator()() const { return func(arg); }

		static void *run_routine(void *arg) {
			const auto *run = (runnable *) arg;
			auto v = (*run)();
			delete run;
			return v;
		}
	};

	// starts new proactor and returns proactor thread id.
	pthread_t startProactor(fd_t sock_fd, proactorFunc threadFunc);

	// stops proactor by thread id
	int stopProactor(pthread_t t);
}

#endif
