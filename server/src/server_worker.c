#include <server_worker.h>
#include <debug.h>
#include <http.h>
#include <http_util.h>
#include <errno.h>
#include <server_stats.h>
#include <memory.h>
#include <log.h>
#include <openssl/err.h>
#include <unistd.h>

extern struct server_stats *stats;

static size_t
server_worker_http_request_read(const size_t max_buffer_size, SSL *ssl, char *buf)
{
	const char *header_end, *cl_header;
	size_t header_len, cl, bytes_read, total_read;

	bytes_read = SSL_read(ssl, buf, max_buffer_size - 1);
	if (bytes_read <= 0) {
		debug_print(level_info, "SSL_read() failed, err: %d, errno: %d, bytes_read: %zu",
			    SSL_get_error(ssl, bytes_read), errno, bytes_read);
		return 0;
	}
	total_read = bytes_read;
	buf[total_read] = '\0';

	header_end = strstr(buf, DOUBLE_CRLF);
	if (!header_end) {
		debug_print(level_info, "unable to find end of headers after (%zu) bytes", total_read);
		return max_buffer_size + 1;
	}
	header_len = header_end - buf + strlen(DOUBLE_CRLF);
	cl_header = strcasestr(buf, "Content-Length: ");
	cl = cl_header ? atol(cl_header + strlen("Content-Length: ")) : 0;

	if (cl + header_len > max_buffer_size)
		return max_buffer_size + 1;

	while (total_read < header_len + cl) {
		bytes_read = SSL_read(ssl, buf + total_read, max_buffer_size - total_read - 1);
		if (bytes_read <= 0) {
			debug_print(level_info, "SSL_read() failed, err: %d, errno: %d, bytes_read: %zu",
				    SSL_get_error(ssl, bytes_read), errno, bytes_read);
			return 0;
		}
		total_read += bytes_read;
		buf[total_read] = '\0';
	}
	debug_print(level_verbose, "request read from client (%zu) bytes", total_read);
	return total_read;
}

static void
server_worker_http_request_process(SSL *ssl, const size_t max_buffer_size,
				   struct sockaddr_in addr)
{
	const char *response;
	char *req_buf, *realloc_buf;
	size_t total_read;
	int ret;
	struct http_response *resp = NULL;

	req_buf = worker_malloc(max_buffer_size + 1, &stats->manager.mem.req_buf_malloc);
	if (!req_buf) {
		stats->worker.malloc_request_buffer_fail++;
		debug_print(level_info, "failed to malloc request buffer");
		populate_failure_log(SL_INTERNAL_FAIL,
				     "failed to malloc request buffer", ssl, addr);
		response = SL_INTERNAL_FAIL;
		goto send;
	}
	total_read = server_worker_http_request_read(max_buffer_size + 1, ssl,
						     req_buf);
	if (total_read == 0) {
		debug_print(level_info, "failed to read client request");
		populate_failure_log(SL_BAD_REQUEST,
				     "failed to read client request", ssl, addr);
		response = SL_BAD_REQUEST;
		goto send;
	}
	if (total_read > max_buffer_size) {
		debug_print(level_info, "request oversized");
		populate_failure_log(SL_BAD_REQUEST, "request oversized", ssl, addr);
		response = SL_BAD_REQUEST;
		goto send;
	}
	realloc_buf = realloc(req_buf, total_read + 1);
	if (realloc_buf)
		req_buf = realloc_buf;
	else
		stats->worker.realloc_fail++;
	
	resp = http_request_buffer_security(req_buf, ssl, addr);
	if (resp)
		goto response_complete;
	resp = http_response(req_buf, total_read, ssl, addr);
	if (!resp) {
		debug_print(level_info, "failure creating response");
		populate_failure_log(SL_INTERNAL_FAIL,
				     "failure creating response", ssl, addr);
		response = SL_INTERNAL_FAIL;
		goto send;
	}
response_complete:
	if (resp->status == status_ok)
		response = resp->full_response;
	else
		response = http_status_line_str(resp->status);
send:
	ret = SSL_write(ssl, response, strlen(response));
	if (ret < 0) {
		debug_print(level_info, "SSL_write() fail, errno: %d",
			    SSL_get_error(ssl, ret));
		populate_failure_log(SL_INTERNAL_FAIL,
				     "failed writing to client", ssl, addr);
		stats->worker.write_response_fail++;
		goto send_error;
	}
	debug_print(level_info, "response sent to client.");
send_error:
	if (resp)
		http_response_free(resp);
	worker_free(req_buf, &stats->manager.mem.req_buf_free);
}

void
server_worker_loop(SSL_CTX *ssl_ctx, const int fd, struct sockaddr_in addr,
		   const size_t max_buffer_size)
{
	int accept_fd, ssl_ret;
	SSL *ssl = NULL;
	socklen_t addrlen = sizeof(addr);

	while (1) {
		accept_fd = accept(fd, (struct sockaddr*)&addr, &addrlen);
		if (accept_fd < 0) {
			stats->worker.accept_fail++;
			debug_print(level_info, "accept() fail, err: %s\n", strerror(errno));
			goto done;
		}
		ssl = SSL_new(ssl_ctx);
		if (!ssl) {
			stats->worker.ssl_new_fail++;
			debug_print(level_info, "SSL_new() fail: %s\n", 
				    ERR_error_string(ERR_get_error(), NULL));
			goto done;
		}
		SSL_set_fd(ssl, accept_fd);
		ssl_ret = SSL_accept(ssl);
		if (ssl_ret <= 0) {
			stats->worker.ssl_accept_fail++;
			debug_print(level_info, "SSL_accept() fail, error: %d.\n",
				    SSL_get_error(ssl, ssl_ret));
			goto done;
		}
		server_worker_http_request_process(ssl, max_buffer_size, addr);
		debug_print(level_verbose, "Worker (%d) finished request...\n", getpid());
		stats->worker.requests_complete++;
done:
		if (ssl) {
			SSL_free(ssl);
			ssl = NULL;
		}
		close(accept_fd);
	}
}