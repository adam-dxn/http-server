#include <unistd.h>
#include <log.h>
#include <security.h>
#include <http_util.h>

extern struct server_stats *stats;

#define PRINT_LOG(field, prefix) \
    if (field[0] != '\0') printf(", " prefix "=\"%s\"", field)

void logs_show() {
	struct log *log;
	uint8_t idx = stats->log_idx;

	printf("***** Server Logs *****\n");

	for (uint8_t i = 0; i < MAX_LOGS; i++, idx = (idx + 1) % MAX_LOGS) {
		log = &stats->logs[idx];
		
		if (log->date[0] == '\0')
			continue;
		printf("%d: ", idx + 1);
		printf("date=\"%s\"", log->date);
		PRINT_LOG(log->uri, "uri");
		PRINT_LOG(log->status, "status");
		PRINT_LOG(log->attack_detected, "attack_detected");
		PRINT_LOG(log->failure_reason, "failure_reason");
		PRINT_LOG(log->method, "method");
		PRINT_LOG(log->version, "version");
		PRINT_LOG(log->content_length, "content-length");
		PRINT_LOG(log->content_type, "content-type");
		PRINT_LOG(log->subject, "subject");
		PRINT_LOG(log->issuer, "issuer");
		PRINT_LOG(log->ip_addr, "ip_addr");
		PRINT_LOG(log->port, "port");
		printf(", worker_pid=\"%d\"\n\n\n", log->worker_pid);
	}
	printf("***********************\n");
}

static void
populate_connection_log(struct log *log, SSL *ssl, struct sockaddr_in client_addr)
{
	uint16_t port;
	X509_NAME *subject, *issuer;
	X509 *cert = SSL_get_peer_certificate(ssl);

	if (!cert)
		return;

	subject = X509_get_subject_name(cert);
	if (!subject) {
		goto err;
	}
	X509_NAME_oneline(subject, log->subject, sizeof(log->subject));
	
	issuer = X509_get_issuer_name(cert);
	if (!issuer) {
		goto err;
	}
	X509_NAME_oneline(issuer, log->issuer, sizeof(log->issuer));

	if (!inet_ntop(AF_INET, &client_addr.sin_addr, log->ip_addr, sizeof(log->ip_addr))) {
		goto err;
	}
	port = ntohs(client_addr.sin_port);
	snprintf(log->port, sizeof(log->port), "%u", port);
err:
	X509_free(cert);
}

void populate_malicious_log(struct http_response *resp, enum client_attack a,
			    SSL *ssl, struct sockaddr_in addr, const char *uri)
{
	const char *status;
	struct log *log = &stats->logs[stats->log_idx];
	time_t now;
	char date[64];

	now = time(NULL);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
	strncpy(log->date, date, sizeof(log->date));

	if (uri != NULL)
		strncpy(log->uri, uri, sizeof(log->uri));

	populate_connection_log(log, ssl, addr);

	status = http_status_line_str(resp->status);
	strncpy(log->status, status, strlen(status) - strlen(CRLF));
	switch (a) {
	case url_encoded_null_byte_attack:
		strncpy(log->attack_detected, "URL encoded null byte attack",
			strlen("URL encoded null byte attack"));
		break;
	case traversal_attack:
		strncpy(log->attack_detected, "Traversal attack",
			strlen("Traversal attack"));
		break;
	case sql_injection_attack:
		strncpy(log->attack_detected, "SQL injection", strlen("SQL injection"));
		break;
	}
	log->worker_pid = getpid();
	/* Circular. Only store up to 64 logs at a time. */
	stats->log_idx = (stats->log_idx + 1) % MAX_LOGS;
	return;
}

void populate_failure_log(const char *status, const char *reason,
			  SSL* ssl, struct sockaddr_in addr)
{
	struct log *log = &stats->logs[stats->log_idx];
	time_t now;
	char date[64];

	now = time(NULL);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
	strncpy(log->date, date, sizeof(log->date));

	strncpy(log->failure_reason, reason, sizeof(log->failure_reason));

	populate_connection_log(log, ssl, addr);

	strncpy(log->status, status, strlen(status) - strlen(CRLF));
	log->worker_pid = getpid();
	/* Circular. Only store up to 64 logs at a time. */
	stats->log_idx = (stats->log_idx + 1) % MAX_LOGS;
	return;
}

void populate_log(struct http_request *req, struct http_response *resp, SSL *ssl,
		  struct sockaddr_in addr)
{
	const char *status, *method, *version;
	struct log *log = &stats->logs[stats->log_idx];
	time_t now;
	char date[64];

	populate_connection_log(log, ssl, addr);

	status = http_status_line_str(resp->status);
	method = http_method_str(req->rl->method);
	version = http_version_str(req->rl->ver);

	strncpy(log->status, status, strlen(status) - strlen(CRLF));
	strncpy(log->uri, req->rl->uri, req->rl->uri_len);
	strncpy(log->method, method, strlen(method));
	strncpy(log->version, version, strlen(version));
 	/* If unsupported, a date/time was never generated and/or sent to the client */
	if (method[0] == 'U' || version[0] == 'U') {
		now = time(NULL);
		strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
		strncpy(log->date, date, sizeof(log->date));
	} else {
		strncpy(log->date, resp->date, sizeof(log->date));
	}
	switch (req->rl->method) {
	case http_request_method_get:
		strncpy(log->content_length, resp->cl, sizeof(log->content_length));
		break;
	case http_request_method_post:
	case http_request_method_put:
		strncpy(log->content_type, req->ct, sizeof(log->content_type));
		strncpy(log->content_length, req->cl, sizeof(log->content_length));
		break;
	case http_request_method_unsupported:
		break;
	}
	log->worker_pid = getpid();
	/* Circular. Only store up to 64 logs at a time. */
	stats->log_idx = (stats->log_idx + 1) % MAX_LOGS;
	return;
}
