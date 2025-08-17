#include <iostream>

#include "pthread_patterns.hpp"

typedef pl::Pipeline<int, int> NumPipeline;


int *s;
int *p;
int *m;

void *sum(void *arg) {
	const auto vals = (int *) arg;
	return new int(vals[0] + vals[1]);
}

void *mul(void *arg) {
	const auto vals = (int *) arg;
	return new int(vals[0] * vals[1]);
}

void *min(void *arg) {
	const auto vals = (int *) arg;
	return new int(vals[0] < vals[1] ? vals[0] : vals[1]);
}

void *print(void *arg) {
	const auto n = *(int **) arg;
	printf("%d\n", *n);
	return nullptr;
}

void pipe_add(const NumPipeline::Work *work) {
	*work->payload += 5;
}

void pipe_print(const NumPipeline::Work *work) {
	std::cout << work->context << ": " << *work->payload << std::endl;
}

int main() {
	auto fs = lf::LF(6);

	const auto args = new int[]{2, 45};
	int rs, rm, rp;

	fs.start();
	fs.run({{sum, (void *) args, (void **) &rs}, {print, (void *) &rs, nullptr}});
	fs.run({{mul, (void *) args, (void **) &rp}, {print, (void *) &rp, nullptr}});
	fs.run({{min, (void *) args, (void **) &rm}, {print, (void *) &rm, nullptr}});
	fs.complete();
	fs.stop();

	NumPipeline p;
	const auto adder = p.startActiveObject(pipe_add);
	const auto printer = p.startActiveObject(pipe_print);

	auto job = NumPipeline::Job();
	job.setContext(new pl::Pipeline<int, int>::Work(9999, new int(4)));
	job.addStage(printer);
	job.addStage(adder);
	job.addStage(printer);
	job.addStage(printer);
	job.addStage(adder);
	job.addStage(printer);
	job.start();

	sleep(3);

	delete args;


	return 0;
}
