//
// Created by aviad on 7/27/25.
//

#ifndef PTHTOOLS_H
#define PTHTOOLS_H

typedef void * (*pthread_func_t)(void *);

struct Func {
	pthread_func_t func = nullptr;
	void *args = nullptr;
	void **retval = nullptr;

	void operator()() const {
		if (func) {
			void *ret = func(args);
			if (retval) *retval = ret;
		}
	}
};

#endif //PTHTOOLS_H
