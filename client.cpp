#include <arpa/inet.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <poll.h>
#include <unistd.h>

#include "fd_polling.hpp"

constexpr auto SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 9034;
constexpr size_t BUF_SIZE = 256;

fd_t server_connection = -1;

void safe_exit(const int sig) {
	reactor::stopAllReactors();
	if (server_connection != -1) close(server_connection);
	exit(sig);
}


fd_t connect_to_server(const char *address, const int port) {
	// Create socket
	const fd_t sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		safe_exit(EXIT_FAILURE);
	}

	// Server setup
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, address, &server_addr.sin_addr) <= 0) {
		perror("inet_pton");
		safe_exit(EXIT_FAILURE);
	}

	// Connect
	if (connect(sock, (sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("connect");
		safe_exit(EXIT_FAILURE);
	}

	std::cout << "Connected to server on " << SERVER_IP << ":" << SERVER_PORT << "\n";

	return sock;
}


void *read_in(const fd_t in_fd) {
	static char buff[BUF_SIZE + 1];

	const ssize_t n = read(in_fd, buff, BUF_SIZE);
	if (n <= 0) {
		std::cout << "stdin closed\n";
		safe_exit(EXIT_SUCCESS);
	}

	buff[n] = '\0';
	if (send(server_connection, buff, n, 0) == -1) {
		perror("send");
		safe_exit(EXIT_FAILURE);
	}

	return nullptr;
}

void *recv_server(const fd_t server_sock_fd) {
	static char buff[BUF_SIZE + 1];

	const ssize_t n = recv(server_sock_fd, buff, BUF_SIZE, 0);
	if (n <= 0) {
		std::cout << "server closed connection\n";
		safe_exit(EXIT_SUCCESS);
	}

	buff[n] = '\0';
	std::cout << "[server] " << buff;

	return nullptr;
}

int main() {
	signal(SIGINT, safe_exit);

	server_connection = connect_to_server(SERVER_IP, SERVER_PORT);

	const auto fd_reactor = reactor::startReactor();
	reactor::addFdToReactor(fd_reactor, server_connection, recv_server);
	reactor::addFdToReactor(fd_reactor, STDIN_FILENO, read_in);

	while (true) {
		reactor::pollReactor(fd_reactor);
		usleep(1000);
	}
}
