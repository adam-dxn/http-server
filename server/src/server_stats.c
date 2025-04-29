#include <server_stats.h>
#include <sys/mman.h>
#include <fcntl.h> 
#include <unistd.h> /* ftruncate(), close() */
#include <string.h> /* memset() */
#include <assert.h>
#include <server.h>
#include <debug.h>

#define PRINT_STAT(field) printf("│ %-52s │ %14"PRIu64" │\n", #field, stats->field)

#define RESET "\x1B[0m"
#define BOLD  "\x1B[1m"

#define HEADER_START BOLD
#define SECTION_START BOLD

extern struct server_stats *stats;

void stats_table_show()
{
	printf("\n%s┌──────────────────────────────────────────────────────┬────────────────┐%s\n", SECTION_START, RESET);
	printf("%s│                     REQUEST STATS                    │                │%s\n", HEADER_START, RESET);
	printf("%s├──────────────────────────────────────────────────────┼────────────────┤%s\n", SECTION_START, RESET);
    
	PRINT_STAT(worker.requests_complete);
	PRINT_STAT(worker.oversized_request);
	PRINT_STAT(worker.http.post_put);
	PRINT_STAT(worker.http.get);
	PRINT_STAT(worker.http.method_unsup);
	PRINT_STAT(worker.http.http_1_0);
	PRINT_STAT(worker.http.http_0_9);
	PRINT_STAT(worker.http.version_unsup);
    
	printf("%s├──────────────────────────────────────────────────────┼────────────────┤%s\n", SECTION_START, RESET);
	printf("%s│                  MALICIOUS REQUESTS                  │                │%s\n", HEADER_START, RESET);
	printf("%s├──────────────────────────────────────────────────────┼────────────────┤%s\n", SECTION_START, RESET);
    
	PRINT_STAT(worker.security.traversal_attack);
	PRINT_STAT(worker.security.sql_injection_attack); 
	PRINT_STAT(worker.security.url_encoded_null_byte_attack);

	printf("%s├──────────────────────────────────────────────────────┼────────────────┤%s\n", SECTION_START, RESET);
	printf("%s│                     MEMORY STATS                     │                │%s\n", HEADER_START, RESET);
	printf("%s├──────────────────────────────────────────────────────┼────────────────┤%s\n", SECTION_START, RESET);
    
	PRINT_STAT(manager.mem.req_buf_malloc);
	PRINT_STAT(manager.mem.req_buf_free);
	PRINT_STAT(manager.mem.full_response_malloc);
	PRINT_STAT(manager.mem.full_response_free);
	PRINT_STAT(manager.mem.http_req_malloc);
	PRINT_STAT(manager.mem.http_req_free);
	PRINT_STAT(manager.mem.http_resp_malloc);
	PRINT_STAT(manager.mem.http_resp_free);
	PRINT_STAT(manager.mem.http_rl_malloc);
	PRINT_STAT(manager.mem.http_rl_free);
    
	printf("%s├──────────────────────────────────────────────────────┼────────────────┤%s\n", SECTION_START, RESET);
	printf("%s│                   FAILURE STATS                      │                │%s\n", HEADER_START, RESET);
	printf("%s├──────────────────────────────────────────────────────┼────────────────┤%s\n", SECTION_START, RESET);

	PRINT_STAT(worker.malloc_request_buffer_fail);
	PRINT_STAT(worker.ssl_new_fail);
	PRINT_STAT(worker.ssl_accept_fail);
	PRINT_STAT(worker.http_request_process_fail);
	PRINT_STAT(worker.realloc_fail);
	PRINT_STAT(worker.write_response_fail);
	PRINT_STAT(manager.ssl_fail.ssl_ctx_new);
	PRINT_STAT(manager.ssl_fail.load_cert);
	PRINT_STAT(manager.ssl_fail.load_key);
	PRINT_STAT(manager.ssl_fail.load_ver_loc);
	PRINT_STAT(manager.fail.server_fd);
	PRINT_STAT(manager.fail.shm_open);
	PRINT_STAT(manager.fail.ftruncate);
	PRINT_STAT(manager.fail.mmap);
	PRINT_STAT(manager.fail.stats_init);
	PRINT_STAT(manager.fail.setsockopt);
	PRINT_STAT(manager.fail.bind);
	PRINT_STAT(manager.fail.listen);
    
    	printf("%s└──────────────────────────────────────────────────────┴────────────────┘%s\n\n", SECTION_START, RESET);
}

int stats_init() {
	int ret = -1, fd;

	shm_unlink("/stats");

	fd = shm_open("/stats", O_CREAT | O_RDWR, 0666);
	if (fd < 0) {
		stats->manager.fail.shm_open++;
		debug_print(level_info, "shm_open() fail, err: %s\n", strerror(errno));
		goto shm_open_fail;
	}
	if (ftruncate(fd, sizeof(struct server_stats)) < 0) {
		stats->manager.fail.ftruncate++;
		debug_print(level_info, "ftruncate() fail, err: %s\n", strerror(errno));
		goto error;
	}
	stats = mmap(NULL, sizeof(struct server_stats), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (stats == MAP_FAILED) {
		stats->manager.fail.mmap++;
		debug_print(level_info, "mmap() fail, err: %s\n", strerror(errno));
		goto error;
	}
	memset(stats, 0, sizeof(struct server_stats));
	ret = 1;
error:
	close(fd);
shm_open_fail:
	return ret;
}
