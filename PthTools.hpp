//
// Created by aviad on 7/27/25.
//

#ifndef PTHTOOLS_H
#define PTHTOOLS_H

typedef void * (*pthread_func_t)(void *);

struct Func {
	pthread_func_t func = nullptr;
	void *args = nullptr;
	void **ret_ptr = nullptr;

	void operator()() const {
		if (func) {
			void *ret = func(args);
			if (ret_ptr) *ret_ptr = ret;
		}
	}
};

#endif //PTHTOOLS_H
