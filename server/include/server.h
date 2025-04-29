#pragma once
#include <stdlib.h> /* size_t/uint8_t */
#include <openssl/ssl.h> /* SSL_CTX */
#include <arpa/inet.h>
#include <stdbool.h>

#define MAX_WORKERS 16
#define MAX_CERT_PATH 128
#define MAX_KEY_PATH 128

enum server_debug_level {
	level_none = 0,
	level_info = 1,
	level_verbose = 2,
};

struct server_config {
	char cert[MAX_CERT_PATH];
	char key[MAX_KEY_PATH];
	size_t max_buffer_size;
	uint8_t num_workers;
	enum server_debug_level debug;
};

struct manager {
	pid_t workers[MAX_WORKERS];
	uint8_t worker_idx;
	struct server_config *cfg;
	/* Need to remember these when we restart workers. */
	int server_fd;
	SSL_CTX *ssl_ctx;
	struct sockaddr_in addr;
};

int manager_server_run(struct manager *);
bool manager_fork_workers(void);
bool manager_shutdown_workers(void);
