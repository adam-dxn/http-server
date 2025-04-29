#pragma once
#include <stdlib.h> 		/* size_t/uint8_t */
#include <stdio.h>
#include <errno.h> 

#define MAX_LOGS 64

struct worker_stats {
	/*
	 * stats one might find useful, not errors.
	 */
	uint64_t requests_complete;
	uint64_t oversized_request;
	struct {
		uint64_t post_put;
		uint64_t get;
		uint64_t method_unsup;
		uint64_t http_1_0;
		uint64_t http_0_9;
		uint64_t version_unsup;
	} http;
	struct {
		uint64_t traversal_attack;
		uint64_t sql_injection_attack;
		uint64_t url_encoded_null_byte_attack;
	} security;
	/*
	 * Errors that should not happen.
	 */
	uint64_t malloc_request_buffer_fail;
	uint64_t accept_fail;
	uint64_t ssl_new_fail;
	uint64_t ssl_accept_fail;
	uint64_t http_request_process_fail;
	uint64_t realloc_fail;
	uint64_t write_response_fail;
};

struct manager_stats {
	/* To monitor memory leaks */
	struct {
		uint64_t req_buf_malloc;
		uint64_t req_buf_free;
		uint64_t full_response_malloc;
		uint64_t full_response_free;
		uint64_t http_req_malloc;
		uint64_t http_req_free;
		uint64_t http_resp_malloc;
		uint64_t http_resp_free;
		uint64_t http_rl_malloc;
		uint64_t http_rl_free;
		uint64_t get_data_malloc;
		uint64_t get_data_free;
	} mem;
	/* SSL context stats */
	struct {
		uint64_t ssl_ctx_new;
		uint64_t load_cert;
		uint64_t load_key;
		uint64_t load_ver_loc;
	} ssl_fail;
	/* Server setup stats */
	struct {
		uint64_t server_fd;
		uint64_t shm_open;
		uint64_t ftruncate;
		uint64_t mmap;
		uint64_t stats_init;
		uint64_t setsockopt;
		uint64_t bind;
		uint64_t listen;
	} fail;
};

struct log {
	char port[6];
	char ip_addr[46];
	char method[16];
	char version[16];
	char status[32];
	char content_length[32];
	char date[64];
	char uri[64];
	char content_type[64];
	char attack_detected[64];
	char failure_reason[64];
	char issuer[128];
	char subject[128];
	pid_t worker_pid;
};

struct server_stats {
	struct worker_stats worker;
	struct manager_stats manager;
	struct log logs[MAX_LOGS];
	uint8_t log_idx;
};

void stats_table_show(void);
int stats_init(void);
