#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <security.h>
#include <server_stats.h>
#include <debug.h>
#include <memory.h>
#include <log.h>
#include <http_util.h>

extern struct server_stats *stats;

enum http_response_status_line
http_request_post(const char *path, const char *body)
{
	FILE *f;
	const char *file = path + 1;

	assert(file);

	f = fopen(file, "w");
	if (!f || fputs(body, f) < 0) {
		fclose(f);
		debug_print(level_info, "failed to write to (%s), err: '%s'",
			    file, strerror(errno));
		return status_internal_fail;
	}
	fclose(f);
	debug_print(level_info, "file (%s) succesfully written to.", file);
	return status_ok;
}

char *
http_request_get(const char *path, enum http_response_status_line *status,
		 size_t *len)
{
	FILE *f;
	char *get_data = NULL;
	const char *file;
	size_t file_len;

	file = path + 1; /* Move past the first '/' */
	assert(file);

	*len = 0;
	f = fopen(file, "r");
	if (!f) {
		debug_print(level_info, "fopen() fail, err: '%s'", file,
			    strerror(errno));
		*status = status_not_found;
		goto validation_error;
	}
	/* Calculate the length of the file and allocate that much memory. */
	fseek(f, 0, SEEK_END);
	file_len = ftell(f);
	fseek(f, 0, SEEK_SET);
	get_data = worker_malloc(file_len + 1, &stats->manager.mem.get_data_malloc);
	if (!get_data) {
		*status = status_internal_fail;
		goto error;
	}
	if (fread(get_data, 1, file_len, f) != file_len) {
		*status = status_internal_fail;
		goto error;
	}
	get_data[file_len] = '\0';
	*status = status_ok;
	*len = file_len;
	debug_print(level_info, "file (%s) found, file_len (%zu).", file,
		    file_len);
error:
	fclose(f);
validation_error:
	return get_data;
}

struct http_response *http_request_get_process(char *path)
{
	char response_header[512], date[64], *data, *full_resp;
	size_t file_len;
	int response_header_len;
	struct http_response *hr;
	time_t now;

	hr = worker_malloc(sizeof(struct http_response),
			   &stats->manager.mem.http_resp_malloc);
	if (!hr)
		return NULL;

	now = time(NULL);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
	strncpy(hr->date, date, sizeof(hr->date));
	data = http_request_get(path, &hr->status, &file_len);
	if (!data || hr->status != status_ok)
		return hr;
	snprintf(hr->cl, sizeof(hr->cl), "%zu", file_len);
	response_header_len = snprintf(response_header, sizeof(response_header),
				       "%s"
				       "Date: %s\r\n"
				       "Server: Adam's Server\r\n"
				       "Content-Length: %zu\r\n"
				       "\r\n",
				       http_status_line_str(hr->status), date, file_len);
	if (response_header_len <= 0)
		return NULL;
	debug_print(level_verbose, "response_header headers:\n%s", response_header);
	full_resp = worker_malloc(response_header_len + file_len,
				  &stats->manager.mem.full_response_malloc);
	if (!full_resp)
		return NULL;
	memcpy(full_resp, response_header, response_header_len);
	memcpy(full_resp + response_header_len, data, file_len);
	hr->full_response = full_resp;
	debug_print(level_verbose, "GET of %s success.", path);
	return hr;
}

struct http_response *
http_request_post_process(char *path, const char *body)
{
	char response_header[512], date[64], *full_resp;
	size_t content_len;
	int response_header_len;
	time_t now;
	struct http_response *hr;

	hr = worker_malloc(sizeof(struct http_response),
			   &stats->manager.mem.http_resp_malloc);
	if (!hr)
		return NULL;
	hr->status = http_request_post(path, body);
	if (hr->status != status_ok)
		return hr;
	now = time(NULL);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&now));
	strncpy(hr->date, date, sizeof(hr->date));
	content_len = strlen(body);
	response_header_len = snprintf(response_header, sizeof(response_header),
				       "%s"
				       "Date: %s\r\n"
				       "Server: Adam's Server\r\n"
				       "Content-Length: %zu\r\n"
				       "\r\n",
				       http_status_line_str(hr->status), date, content_len);
	if (response_header_len <= 0)
		return NULL;
	full_resp = worker_malloc(response_header_len + content_len + 1,
			          &stats->manager.mem.full_response_malloc);
	if (!full_resp)	
		return NULL;
	memcpy(full_resp, response_header, response_header_len);
	memcpy(full_resp + response_header_len, body, content_len);
	full_resp[response_header_len + content_len] = '\0';
	hr->full_response = full_resp;
	debug_print(level_verbose, "hii");
	return hr;
}

static struct http_response *
http_request_simple_response(struct http_request_line *hreq_line)
{
	stats->worker.http.post_put++;
	return http_request_get_process(hreq_line->uri);
}

/*
 * If there's a really unexpected error (like a malloc failure), the request
 * processing functions will return NULL. In that case, the client will
 * recieve an internal error.
 */
struct http_response *
http_request_full_response(struct http_request *req)
{
	struct http_response *resp;
	
	switch(req->rl->method) {
	case http_request_method_post:
	case http_request_method_put:
		stats->worker.http.post_put++;
		resp = http_request_post_process(req->rl->uri, req->body);
		break;
	case http_request_method_get:
		stats->worker.http.get++;
		resp = http_request_get_process(req->rl->uri);
		break;
	case http_request_method_unsupported:
		stats->worker.http.method_unsup++;
		debug_print(level_info, "http method unsupported.");
		resp = worker_malloc(sizeof(struct http_response),
				     &stats->manager.mem.http_resp_malloc);
		if (!resp)
			return NULL;
		resp->full_response = NULL;
		resp->status = status_method_unsupported;
		break;
	}
	/* If resp is NULL, we send an internal_error */
	return resp;
}

void http_response_free(struct http_response *h_resp)
{
	if (!h_resp)
		return;
	if (h_resp->full_response) {
		worker_free((void *)h_resp->full_response,
			    &stats->manager.mem.full_response_free);
	}
	worker_free(h_resp, &stats->manager.mem.http_resp_free);
}

static void
http_request_free(struct http_request *h_req)
{
	if (!h_req)
		return;
	if (h_req->rl)
		worker_free(h_req->rl, &stats->manager.mem.http_rl_free);
	worker_free(h_req, &stats->manager.mem.http_req_free);
}

struct http_response *
http_response(const char *buf, const size_t buflen, SSL *ssl,
	      struct sockaddr_in addr)
{
	struct http_request *req;
	struct http_response *resp;

	req = http_request_parse(buf, buflen);
	if (!req)
		return NULL;
	resp = http_request_uri_security(req->rl->uri, ssl, addr);
	if (resp)
		goto done;
	switch(req->rl->ver) {
		case http_request_version_0_9:
			stats->worker.http.http_0_9++;
			resp = http_request_simple_response(req->rl);
			break;
		case http_request_version_1_0:
			stats->worker.http.http_1_0++;
			resp = http_request_full_response(req);
			break;
		case http_request_version_unsupported:
			stats->worker.http.version_unsup++;
			resp = worker_malloc(sizeof(struct http_response),
					     &stats->manager.mem.http_resp_malloc);
			if (!resp)
				return NULL;
			resp->full_response = NULL;
			resp->status = status_version_unsupported;
			break;
	}
	populate_log(req, resp, ssl, addr);

done:
	http_request_free(req);
	return resp; /* Caller responsible for freeing resp */
}
