#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <http.h>
#include <http_util.h>
#include <debug.h>
#include <server_stats.h>
#include <memory.h>

extern struct server_stats *stats;

static enum http_request_method
http_request_method_parse(const char *method, const size_t method_len)
{
	if (!strncmp(method, "POST", method_len))
		return http_request_method_post;
	if (!strncmp(method, "PUT", method_len))
		return http_request_method_put;
	if (!strncmp(method, "GET", method_len))
		return http_request_method_get;
	debug_print(level_info, "Method (%s) unsupported.", method);
	return http_request_method_unsupported;
}

/* Parse the HTTP/1.0 request according to the RFC: xxx (find the RFC) */
static struct http_request_line *
http_request_request_line_parse(const char *buf, const size_t buflen)
{
	const char *space, *uri_start, *uri_end, *version_start, *version_end;
	struct http_request_line *rl;

	/* TODO: Figure out what I should do with buflen - it's currently unused */
	(void) buflen;
	rl = worker_malloc(sizeof(struct http_request_line),
			   &stats->manager.mem.http_rl_malloc);
	if (!rl)
		return NULL;
	space = strchr(buf, ' ');
	if (!space) {
		debug_print(level_info, "parse failure: request line malformed.");
		return rl;
	}
	rl->method = http_request_method_parse(buf, space - buf);

	uri_start = space + 1;
	uri_end = strchr(uri_start, ' ');
	if (uri_end) {
		version_start = uri_end + 1;
		if (!version_start) {
			debug_print(level_info, "parse failure: cannot find http version.");
			return rl;
		}
		version_end = strstr(version_start, CRLF);
		if (!strncmp(version_start, "HTTP/1.0", version_end - version_start)) {
			debug_print(level_verbose, "request version is http/1.0");
			rl->ver = http_request_version_1_0;
		} else {
			debug_print(level_verbose, "request version unsupported.");
			rl->ver = http_request_version_unsupported;
		}
	} else {
		/* HTTP/0.9. Parse a Simple-Request */
		uri_end = strstr(uri_start, CRLF);
		if (!uri_end) {
			debug_print(level_info, "No URI found for HTTP/0.9\n");
			return rl;
		}
		debug_print(level_verbose, "request version is http/0.9");
		rl->ver = http_request_version_0_9;
	}
	assert(uri_start);
	rl->uri_len = uri_end - uri_start;
	memcpy(rl->uri, uri_start, rl->uri_len);
	rl->uri[rl->uri_len] = '\0';
	debug_print(level_info, "request line successfully parsed. uri (%s)",
		    rl->uri);
	return rl;
}

static void
http_request_field_parse(const char *headers, const char *field, char *buf,
			 const size_t buf_len)
{
	const char *start, *end;
	size_t len;
	start = strcasestr(headers, field);
	if (!start) {
		debug_print(level_info, "Unable to find field (%s)", field); 
		return;
	}
	start += strlen(field);
	end = strstr(start, CRLF);
	if (!end) {
		debug_print(level_info, "Unable to find CRLF after field (%s)",
			    field); 
		return;
	}
	len = end - start;
	if (len >= buf_len) { /* Truncate (highly unlikely) */
		debug_print(level_verbose, "FYI: field truncated");
		len = buf_len - 1;
	}
	strncpy(buf, start, len);
	buf[len] = '\0';
	debug_print(level_info, "field (%s) found: (%s)", field, buf);
}

struct http_request *
http_request_parse(const char *buf, const size_t buflen)
{
	struct http_request *hreq = worker_malloc(sizeof(struct http_request), 
						  &stats->manager.mem.http_req_malloc);
	if (!hreq)
		return NULL;
	hreq->rl = http_request_request_line_parse(buf, buflen);
	if (hreq->rl->uri[0] == '\0') {
		debug_print(level_info, "unable to find request URI");
		return NULL;
	}
	hreq->headers = strstr(buf, CRLF);
	if (!hreq->headers) {
		debug_print(level_info, "request malformed: unable to find header CRLF");
		return NULL;
	}
	hreq->headers += strlen(CRLF);
	if (hreq->rl->method == http_request_method_post
	    || hreq->rl->method == http_request_method_put) {
		http_request_field_parse(hreq->headers, "Content-Type: ", hreq->ct, sizeof(hreq->ct));
		http_request_field_parse(hreq->headers, "Content-Length: ", hreq->cl, sizeof(hreq->cl));
	}
	hreq->body = strstr(hreq->headers, DOUBLE_CRLF);
	if (!hreq->body) {
		debug_print(level_info, "failed to parse request body.");
		return NULL;
	}
	hreq->body += strlen(DOUBLE_CRLF);
	debug_print(level_info, "successfully parsed the request header and body.");
	return hreq;
}
