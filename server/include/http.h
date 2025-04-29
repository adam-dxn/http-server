#pragma once
#include <arpa/inet.h>
#include <openssl/ssl.h>

enum http_response_status_line {
	status_ok,
	status_not_found,
	status_bad_request,
	status_internal_fail,
	status_unauthorized,
	status_malicious,
	status_method_unsupported,
	status_version_unsupported,
};

enum http_request_method {
	http_request_method_unsupported = 0,
	http_request_method_post,
	http_request_method_put,
	http_request_method_get,
};

enum http_request_version {
	http_request_version_unsupported = 0,
	http_request_version_0_9,
	http_request_version_1_0,
};

struct http_request_line {
	char uri[64]; /* currently holds the filename -> fix that */
	size_t uri_len;
	enum http_request_method method;
	enum http_request_version ver;
};

struct http_request {
	struct http_request_line *rl;
	char cl[32]; /* content-length for POST/PUT */
	char ct[64]; /* content-type for POST/PUT */
	const char *headers;
	const char *body;
};

struct http_response {
	const char *full_response; /* malloc'd => must be free'd */
	char cl[32]; /* content-length for GET */
	char date[64];
	enum http_response_status_line status;
};

struct http_request *http_request_parse(const char *, const size_t);
struct http_response *http_response(const char *, const size_t, SSL *, struct sockaddr_in);
void http_response_free(struct http_response *);
