#pragma once
#include <http.h> 	  /* struct http_response */
#include <server_stats.h>
#include <openssl/ssl.h>  /* SSL */
#include <arpa/inet.h> 	  /* struct sockaddr_in */
#include <stdbool.h> 	  /* bool */

enum client_attack {
	url_encoded_null_byte_attack,
	traversal_attack,
	sql_injection_attack,
};

struct http_response *
http_request_buffer_security(const char *, SSL *, struct sockaddr_in);

struct http_response *
http_request_uri_security(const char *, SSL *, struct sockaddr_in);