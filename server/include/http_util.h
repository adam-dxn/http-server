#pragma once
#include <http.h>

#define CRLF		       "\r\n"
#define DOUBLE_CRLF 	       "\r\n\r\n"
#define SL_OK                  "HTTP/1.0 200 OK" CRLF
#define SL_BAD_REQUEST         "HTTP/1.0 400 Bad Request" CRLF
#define SL_UNAUTHORIZED        "HTTP/1.0 401 Unauthorized" CRLF
#define SL_NOT_FOUND           "HTTP/1.0 404 Not Found" CRLF
#define SL_METHOD_UNSUPPORTED  "HTTP/1.0 405 Method Not Allowed" CRLF
#define SL_INTERNAL_FAIL       "HTTP/1.0 500 Internal Server Error" CRLF
#define SL_VER_UNSUPPORTED     "HTTP/1.0 505 Version Unsupported" CRLF
#define SL_MALICIOUS	       "HTTP/1.0 999 Malicious Request" CRLF

const char *http_method_str(const enum http_request_method);
const char *http_version_str(const enum http_request_version);
const char *http_status_line_str(const enum http_response_status_line);