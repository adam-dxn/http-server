#include <assert.h>
#include <stdlib.h>
#include <security.h>
#include <debug.h>
#include <memory.h>
#include <log.h>

extern struct server_stats *stats;

#define TRAVERSAL_STR ".."
#define URL_ENCODED_NULL_BYTE "%00"

static char *sql[] = {"'", "\"", ";", "--", "/*", "*/", "UNION", "SELECT", "DROP"};
static size_t sql_size = sizeof(sql)/sizeof(sql[0]);

/*
 * Reference: https://www.invicti.com/blog/web-security/sql-injection-cheat-sheet/
 */
static bool
http_request_sql_injection(const char *uri)
{
	for (size_t i = 0; i < sql_size; i++) {
		if (strstr(uri, sql[i]))
			return true;
	}
	return false;
}

static struct http_response *
http_response_malicious(enum client_attack a, SSL* ssl, struct sockaddr_in addr,
			const char *uri)
{
	struct http_response *resp;

	resp = worker_malloc(sizeof(struct http_response),
			     &stats->manager.mem.http_resp_malloc);
	if (!resp)
		return NULL;
	resp->full_response = NULL;
	resp->status = status_malicious;
	populate_malicious_log(resp, a, ssl, addr, uri);
	return resp;

}

struct http_response *
http_request_uri_security(const char *uri, SSL *ssl, struct sockaddr_in addr)
{
	assert(uri);
	assert(ssl);

	if (strstr(uri, TRAVERSAL_STR)) {
		stats->worker.security.traversal_attack++;
		debug_print(level_verbose, "WARNING: Path traversal attack detected in URI");
		return http_response_malicious(traversal_attack, ssl, addr, uri);
	} else if (http_request_sql_injection(uri)) {
		stats->worker.security.sql_injection_attack++;
		debug_print(level_verbose, "WARNING: SQL injection detected in URI");
		return http_response_malicious(sql_injection_attack, ssl, addr, uri);
	}
	return NULL;
}

struct http_response *
http_request_buffer_security(const char *buf, SSL *ssl, struct sockaddr_in addr)
{
	assert(buf);
	assert(ssl);

	if (strstr(buf, URL_ENCODED_NULL_BYTE)) {
		stats->worker.security.url_encoded_null_byte_attack++;
		debug_print(level_verbose, "WARNING: URL-encoded null byte detected in request buffer");
		return http_response_malicious(url_encoded_null_byte_attack, ssl, addr, NULL);
	}
	return NULL;
}
