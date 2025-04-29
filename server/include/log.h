#pragma once
#include <http.h>
#include <security.h>

void populate_failure_log(const char *, const char *, SSL *, struct sockaddr_in);
void populate_malicious_log(struct http_response *, enum client_attack,
			    SSL *, struct sockaddr_in, const char *);
void populate_log(struct http_request *, struct http_response *, SSL *,
		  struct sockaddr_in);
void logs_show(void);
