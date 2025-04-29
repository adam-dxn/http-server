#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ssl.h>
#include <user_interface.h>
#include <server.h>
#include <server_stats.h>
#include <assert.h>
#include <signal.h> 
#include <sys/mman.h>
#include <debug.h>
#include <errno.h> 
#include <server_worker.h>

#define PORT 8080
#define MAX_BACKLOG 5

struct server_stats *stats;
struct manager *m;

bool manager_fork_workers()
{
	m->ssl_ctx = ssl_ctx_init(m->cfg->cert, m->cfg->key);
	if (!m->ssl_ctx) {
		debug_print(level_info, "Failed to initalize the SSL context.\n");
		return false;
	}
	for (uint8_t i = 0; i < m->cfg->num_workers; i++) {
		pid_t pid = fork();
		if (pid < 0) {
			debug_print(level_info, "fork() fail with pid %d\n", pid);
			return false;
		} else if (pid == 0) {
			server_worker_loop(m->ssl_ctx, m->server_fd, m->addr,
					  m->cfg->max_buffer_size);
		} else {
			m->workers[m->worker_idx] = pid;
			m->worker_idx++;
		}
	}
	assert(m->worker_idx == m->cfg->num_workers);
	debug_print(level_info, "%u workers successfully forked.", m->worker_idx);
	return true;
}

bool manager_shutdown_workers()
{
	debug_print(level_info, "About to shutdown workers.");
	for (uint8_t i = 0; i < m->worker_idx; i++) {
		pid_t worker_pid = m->workers[i];

		kill(worker_pid, SIGTERM);
		waitpid(worker_pid, NULL, 0);
		debug_print(level_verbose, "Worker %d killed.", worker_pid);
	}
	m->worker_idx = 0;
	if (m->ssl_ctx) {
		ssl_ctx_free(m->ssl_ctx);
		m->ssl_ctx = NULL;
	}
	debug_print(level_info, "Worker shutdown complete.");
	return true;
}

void manager_sig_handler()
{
	stats_table_show();
	for (uint8_t i = 0; i < m->cfg->num_workers; i++) {
		pid_t worker_pid = m->workers[i];

		kill(worker_pid, SIGTERM);
		waitpid(worker_pid, NULL, 0);
	}
	if (m->ssl_ctx) {
		ssl_ctx_free(m->ssl_ctx);
		m->ssl_ctx = NULL;
	}
	m->worker_idx = 0;
	close(m->server_fd);
	munmap(stats, sizeof(struct server_stats));
	shm_unlink("/stats");
	if (m->cfg)
		free(m->cfg);
	if (m)
		free(m);
}

int manager_server_run(struct manager *manager)
{
	int server_fd, ret = -1, opt;
	struct sockaddr_in addr;
	struct sigaction sa;

	m = manager;

	debug_print(level_info, "About to start the server.");

	/* Initialize shared memory so worker processes can populate stats. */
	if (stats_init() < 0) {
		debug_print(level_info, "Error setting up stats. Server shutting down.");
		stats->manager.fail.stats_init++;
		goto shared_mem_error;
	}
	/*
	* Create a socket, bind and listen to it. Workers will accept
	* connections on this socket.
	*/
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		debug_print(level_info, "socket() fail, err: %s", strerror(errno));
		stats->manager.fail.server_fd++;
		goto error;
	}
	opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		debug_print(level_info, "setsockopt() fail, err: %s", strerror(errno));
		stats->manager.fail.setsockopt++;
		goto error;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(PORT);
	if (bind(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
		debug_print(level_info, "bind() fail, err: %s", strerror(errno));
		stats->manager.fail.bind++;
		goto error;
	}
	if (listen(server_fd, MAX_BACKLOG) < 0) {
		debug_print(level_info, "listen() fail, err: %s", strerror(errno));;
		stats->manager.fail.listen++;
		goto error;
	}
	m->server_fd = server_fd;
	m->addr = addr;

	/*
	* Create signal handlers for segfaults and interuptions. If users
	* stop the program via 'ctrl-c', the server will gracefully shutdown.
	*/
	sa.sa_handler = manager_sig_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) < 0) {
		debug_print(level_info, "sigaction(SIGINT...) fail, err: %s",
			    strerror(errno));
		goto error;	
	}
	if (sigaction(SIGSEGV, &sa, NULL) < 0) {
		debug_print(level_info, "sigaction(SIGSEGV...) fail, err: %s",
			    strerror(errno));
		goto error;		
	}
	debug_print(level_info, "manager starting up...");
	manager_fork_workers();
	/* Start the CLI. */
	ui_run();
	manager_shutdown_workers();
error:
	debug_print(level_info, "server shutting down.");
	close(server_fd);
shared_mem_error:
	munmap(stats, sizeof(struct server_stats));
	shm_unlink("/stats");
	return ret;
}
