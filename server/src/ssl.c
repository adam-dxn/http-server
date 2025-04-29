#include <ssl.h>
#include <server_stats.h>
#include <debug.h>

extern struct server_stats *stats;

void ssl_ctx_free(SSL_CTX *ssl_ctx) {
	if (!ssl_ctx)
		return;
	SSL_CTX_free(ssl_ctx);
}

SSL_CTX *ssl_ctx_init(const char *cert, const char *key)
{
	SSL_CTX *ssl_ctx;

	SSL_library_init();
	ssl_ctx = SSL_CTX_new(TLS_method());
	if(!ssl_ctx) {
		debug_print(level_info, "SSL_CTX_new fail.");
		stats->manager.ssl_fail.ssl_ctx_new++;
		return NULL;
	}
	if (SSL_CTX_use_certificate_file(ssl_ctx, cert, SSL_FILETYPE_PEM) <= 0) {
		debug_print(level_info, "Failed to load cert %s", cert);
		stats->manager.ssl_fail.load_cert++;
		goto setup_error;
	}
	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key, SSL_FILETYPE_PEM) <= 0) {
		debug_print(level_info, "Failed to load key %s", key);
		stats->manager.ssl_fail.load_key++;
		goto setup_error;
	}
	if (!SSL_CTX_load_verify_locations(ssl_ctx, cert, NULL)) {
		debug_print(level_info, "Failed to load CA certificate.");
		stats->manager.ssl_fail.load_ver_loc++;
		goto setup_error;
	}
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	debug_print(level_verbose, "SSL content setup success.");
	return ssl_ctx;
setup_error:
	ssl_ctx_free(ssl_ctx);
	return NULL;
}


