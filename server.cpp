#include <csignal>
#include <cstring>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <sys/poll.h>

#include "fd_polling.hpp"
#include "pthread_patterns.hpp"
#include "graph/EulerAlgorithm.h"
#include "graph/Graph.h"
#include "graph/MaxCliqueAlgorithm.h"
#include "graph/MaxFlowAlgorithm.h"
#include "graph/RandomGraph.h"
#include "graph/SCCAlgorithm.h"

using namespace std;

#ifndef USE_DEQUE
#define USE_DEQUE 0
#endif

typedef int fd_t;


void lower(char *p) { for (; *p; ++p) *p = (char) tolower(*p); }

bool streq(const char *p1, const char *p2) { return strcmp(p1, p2) == 0; }


string fmtAlgoRes(const Algorithm &algorithm, const string &result) {
	return "\t" + algorithm.name() + " := " + result + "\n";
}

namespace graph_lf {
	struct GraphWork {
		fd_t requester = -1;
		Graph *graph{};
		Algorithm *algorithm{};
		string *answer{};

		~GraphWork() {
			delete graph;
			delete algorithm;
			delete answer;
		}

		static void *compute_r(void *arg) {
			const auto p = (GraphWork *) arg;
			return new string(p->algorithm->run(*p->graph));
		}

		static void compute(void *arg) {
			const auto p = (GraphWork *) arg;
			p->answer = (string *) compute_r(arg);
		}

		static void *commit(void *arg) {
			const auto p = (GraphWork *) arg;

			// send string answer to client fd requester
			dprintf(p->requester, fmtAlgoRes(*p->algorithm, *p->answer).c_str());

			delete p;

			return nullptr;
		}
	};
};

namespace graph_pl {
	class GraphPayload {
	public:
		const Graph *graph{};
		vector<string> answers;

		explicit GraphPayload(const Graph &g) : graph(new Graph(g)) {
		}

		~GraphPayload() {
			delete graph;
		}
	};

	typedef pl::Pipeline<fd_t, GraphPayload> GraphAlgoPipeline;

	namespace workers {
		namespace alg {
			void mc(const GraphAlgoPipeline::Work *work) {
				auto algo = MaxCliqueAlgorithm();
				work->payload->answers.push_back(fmtAlgoRes(algo, algo.run(*work->payload->graph)));
			}

			void mf(const GraphAlgoPipeline::Work *work) {
				auto algo = MaxFlowAlgorithm();
				work->payload->answers.push_back(fmtAlgoRes(algo, algo.run(*work->payload->graph)));
			}

			void eu(const GraphAlgoPipeline::Work *work) {
				auto algo = EulerAlgorithm();
				work->payload->answers.push_back(fmtAlgoRes(algo, algo.run(*work->payload->graph)));
			}

			void sc(const GraphAlgoPipeline::Work *work) {
				auto algo = SCCAlgorithm();
				work->payload->answers.push_back(fmtAlgoRes(algo, algo.run(*work->payload->graph)));
			}
		};

		void send_results(const GraphAlgoPipeline::Work *work) {
			for (const auto &answer: work->payload->answers)
				dprintf(work->context, "%s", answer.c_str());
			work->payload->answers.clear();
		}
	};
};

// client job thread manager
auto job_handler = lf::LF(3);
auto pipeline_handler = graph_pl::GraphAlgoPipeline();
vector<graph_pl::GraphAlgoPipeline::Stage> graph_pipeline_stages;

// client fds
vector<fd_t> client_fds;
// client connection proactor for accepting clients
pthread_t client_connection_proactor;
// client list modification protection mutex
pthread_mutex_t fds_modify_mutex;


void safe_exit(const int sig) {
	// stop server client connection proactor
	proactor::stopProactor(client_connection_proactor);

	// wait for jobs to finish
	job_handler.complete();
	// stop job thread manager
	job_handler.stop();

	// close all fds
	pthread_mutex_lock(&fds_modify_mutex);
	for (const auto client: client_fds) {
		printf("closing fd %d\n", client);
		close(client);
	}
	pthread_mutex_unlock(&fds_modify_mutex);

	// cleanup mutexes
	pthread_mutex_destroy(&fds_modify_mutex);

	// exit
	exit(sig);
}


void run_algos_lf(const Graph &graph, const fd_t response_fd) {
	printf("run_algos_lf for fd %d\n", response_fd);
	const auto payloads = vector{
		new graph_lf::GraphWork{response_fd, new Graph(graph), new MaxCliqueAlgorithm()},
		new graph_lf::GraphWork{response_fd, new Graph(graph), new EulerAlgorithm()},
		new graph_lf::GraphWork{response_fd, new Graph(graph), new MaxFlowAlgorithm()},
		new graph_lf::GraphWork{response_fd, new Graph(graph), new SCCAlgorithm()}
	};
	for (const auto p: payloads)
		job_handler.run({
			{graph_lf::GraphWork::compute_r, p, (void **) &p->answer},
			{graph_lf::GraphWork::commit, p}
		});
}

void run_algos_pl(const Graph &graph, const fd_t response_fd) {
	printf("run_algos_pl for fd %d\n", response_fd);
	graph_pl::GraphAlgoPipeline::Job algo_job;
	algo_job.setWork(response_fd, new graph_pl::GraphPayload(graph));
	for (const auto stage: graph_pipeline_stages)
		algo_job.addStage(stage);
	algo_job.start();
}


void parse_command_client(const fd_t response_fd, const char *command, const char *args) {
	Graph graph;
	if (streq(command, "newgraph")) {
		int v = 0, e = 0, mw = 0, Mw = 0;
		bool directed = false;
		try {
			dprintf(response_fd, "newgraph args %s\n", args);
			istringstream in(args);
			string cmd;
			in >> cmd >> v >> e >> mw >> Mw;
			graph = generateRandomGraph(v, e, directed, mw, Mw, time(nullptr));
			dprintf(response_fd, "generated new random graph:\n\t%s\n", to_string_human(graph).c_str());
			// run_algos_lf(graph, response_fd);
			run_algos_pl(graph, response_fd);
		} catch (exception &ex) {
			dprintf(response_fd, "failed to generate graph: %s\n", ex.what());
		}
	} else if (streq(command, "graph")) {
		// parse graph
		try {
			graph = from_string(std::string(args));
			run_algos_lf(graph, response_fd);
		} catch (exception &ex) {
			dprintf(response_fd, "failed to parse graph: %s", ex.what());
		}
	} else dprintf(response_fd, "unknown command \"%s\"\n", command);
}

void parse_command_stdin(const char *command, const char *buff) {
	if (streq(command, "exit") || streq(command, "quit") || streq(command, "q"))
		safe_exit(EXIT_SUCCESS);
	parse_command_client(STDOUT_FILENO, command, buff);
}


void *handle_client(const fd_t client_fd) {
	// track client fd
	client_fds.push_back(client_fd);
	printf("new client connected on fd %d\n", client_fd);

	char buff[256 + 1];

	while (true) {
		pollfd pfd = {.fd = client_fd, .events = POLLIN, .revents = POLLIN};
		// poll for input (cancellation point)
		if (poll(&pfd, 1, -1) == -1) {
			perror("poll");
			continue;
		}
		if (!(pfd.revents & POLLIN)) continue;

		const ssize_t rn = read(client_fd, buff, 256);

		if (rn <= 0) {
			if (rn == 0) printf("socket %d hung up\n", client_fd);
			else perror("read");

			// client disconnected

			pthread_mutex_lock(&fds_modify_mutex);

			// close client fd
			close(client_fd);
			// stop tracking client fd
			erase(client_fds, client_fd);

			pthread_mutex_unlock(&fds_modify_mutex);

			// return and terminate thread
			return nullptr;
		}


		buff[rn] = '\0';
		printf("[client %d] %s", client_fd, buff);

		char command[256 + 1];
		if (sscanf(buff, "%256s", command) == EOF) {
			perror("failed to read command");
			continue;
		}

		// parse command
		lower(command);
		parse_command_client(client_fd, command, buff);

		// cancellation point
		usleep(1000);
	}
}

void handle_input() {
	string buff;

	while (true) {
		// read input
		getline(cin, buff);

		char command[256 + 1];
		if (sscanf(buff.c_str(), "%256s", command) == EOF) {
			perror("failed to read command");
			continue;
		}

		// parse command
		lower(command);
		parse_command_stdin(command, buff.c_str());
	}
}


fd_t setup_server() {
	sockaddr_in server_addr{};

	const fd_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("socket");
		safe_exit(EXIT_FAILURE);
	}

	constexpr int yes = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(9034);

	if (bind(server_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		safe_exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0) {
		perror("listen");
		safe_exit(EXIT_FAILURE);
	}

	return server_fd;
}

int main() {
	signal(SIGINT, safe_exit);

	// init mutexes
	pthread_mutex_init(&fds_modify_mutex, nullptr);

	// create server socket
	const fd_t server_socket = setup_server();

	// start client job thread manager
	job_handler.start();
	// setup client job pipeline
	graph_pipeline_stages = {
		pipeline_handler.startActiveObject(graph_pl::workers::alg::mc),
		pipeline_handler.startActiveObject(graph_pl::workers::alg::eu),
		pipeline_handler.startActiveObject(graph_pl::workers::alg::mf),
		pipeline_handler.startActiveObject(graph_pl::workers::alg::sc),

		pipeline_handler.startActiveObject(graph_pl::workers::send_results)
	};

	// start server client connection proactor
	client_connection_proactor = proactor::startProactor(server_socket, handle_client);

	// main thread handles std input
	handle_input();

	safe_exit(EXIT_SUCCESS);
}
