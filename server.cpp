#include <csignal>
#include <cstring>
#include <iostream>
#include <ranges>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <sys/poll.h>

#include "fd_polling.hpp"

using namespace std;

#ifndef USE_DEQUE
#define USE_DEQUE 0
#endif

typedef int fd_t;


void lower(char *p) { for (; *p; ++p) *p = (char) tolower(*p); }

bool streq(const char *p1, const char *p2) { return strcmp(p1, p2) == 0; }


// global data
// vector<geo::Point2D> graph;
// data modification protection mutex
pthread_mutex_t graph_modify_mutex;


// client fds
vector<fd_t> client_fds;
// client connection proactor for accepting clients
pthread_t client_connection_proactor;
// client list modification protection mutex
pthread_mutex_t fds_modify_mutex;


void safe_exit(const int sig) {
	// stop server client connection proactor
	proactor::stopProactor(client_connection_proactor);

	// close all fds
	pthread_mutex_lock(&fds_modify_mutex);
	for (const auto client: client_fds) {
		printf("closing fd %d\n", client);
		close(client);
	}
	pthread_mutex_unlock(&fds_modify_mutex);

	// cleanup mutexes
	pthread_mutex_destroy(&fds_modify_mutex);
	pthread_mutex_destroy(&graph_modify_mutex);

	// exit
	exit(sig);
}


void parse_command_client(const fd_t response_fd, const char *command, const char *buff) {
	if (streq(command, "newgraph")) {
		pthread_mutex_lock(&graph_modify_mutex);
		// graph.clear();
		pthread_mutex_unlock(&graph_modify_mutex);

		dprintf(response_fd, "reset graph\n");
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
	pthread_mutex_init(&graph_modify_mutex, nullptr);
	pthread_mutex_init(&fds_modify_mutex, nullptr);

	// create server socket
	const fd_t server_socket = setup_server();

	// start server client connection proactor
	client_connection_proactor = proactor::startProactor(server_socket, handle_client);

	// main thread handles std input
	handle_input();

	safe_exit(EXIT_SUCCESS);
}
