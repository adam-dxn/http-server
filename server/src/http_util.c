#include <http_util.h>

const char *
http_method_str(const enum http_request_method m)
{
	switch(m) {
	case http_request_method_post:
		return "POST";
	case http_request_method_put:
		return "PUT";
	case http_request_method_get:
		return "GET";
	case http_request_method_unsupported:
		return "Unsupported";
	}
}

const char *
http_version_str(const enum http_request_version v)
{
	switch(v) {
	case http_request_version_0_9:
		return "HTTP/0.9";
	case http_request_version_1_0:
		return "HTTP/1.0";
	case http_request_version_unsupported:
		return "Unsupported";
	}
}

const char *
http_status_line_str(const enum http_response_status_line s)
{
	switch(s) {
	case status_ok:
		return SL_OK;
	case status_not_found:
		return SL_NOT_FOUND;
	case status_bad_request:
		return SL_BAD_REQUEST;
	case status_internal_fail:
		return SL_INTERNAL_FAIL;
	case status_unauthorized:
		return SL_UNAUTHORIZED;
	case status_method_unsupported:
		return SL_METHOD_UNSUPPORTED;
	case status_version_unsupported:
		return SL_VER_UNSUPPORTED;
	case status_malicious:
		return SL_MALICIOUS;
	}
}