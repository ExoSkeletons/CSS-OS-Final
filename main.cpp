#include <iostream>

#include "fd_polling.hpp"
#include "pthread_patterns.hpp"

typedef pl::Pipeline<int, int> NumPipeline;


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

void *graph_bfs(void *arg) {
	typedef int graph;
	const auto g = (graph *) arg;
	// do thing...
	const std::string result = "result bfs"; // result from function...
	return new std::string(result);
}

// for LF- pack result with this
class client_result {
public:
	fd_t client = -1;
	int *result = nullptr;

	static void *send_result(void *arg) {
		const auto cr = (client_result *) arg;
		std::cout << "result " << *cr->result << " sent to " << cr->client << std::endl;
		return nullptr;
	}

	~client_result() {
		delete result;
	}

	client_result() = default;

	explicit client_result(const fd_t client): client(client) {
	}
};


int main() {
	auto fs = lf::LF(6);

	const auto args = new int[]{2, 45};
	const auto
			rs = new client_result(22),
			rm = new client_result(55),
			rp = new client_result(7777);

	fs.start();
	fs.run({
		{sum, (void *) args, (void **) &rs->result}, {print, (void *) &rs->result, nullptr},
		{client_result::send_result, (void *) rs, nullptr}
	});
	fs.run({{mul, (void *) args, (void **) &rp->result}, {print, (void *) &rp->result, nullptr}});
	fs.run({
		{min, (void *) args, (void **) &rm->result}, {print, (void *) &rm->result, nullptr},
		{client_result::send_result, (void *) rm, nullptr}
	});
	fs.complete();
	delete rs;
	delete rp;
	delete rm;
	fs.stop();

	NumPipeline p;
	const auto adder = p.startActiveObject(pipe_add);
	const auto printer = p.startActiveObject(pipe_print);

	auto job = NumPipeline::Job();
	job.setWork(new NumPipeline::Work(9999, new int(4)));
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
