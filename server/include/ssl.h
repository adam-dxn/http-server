#include <openssl/ssl.h>

SSL_CTX *ssl_ctx_init(const char *, const char *);
void ssl_ctx_free(SSL_CTX *);
