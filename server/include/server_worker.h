#include <ssl.h>
#include <netinet/in.h>
#include <stdlib.h>

void
server_worker_loop(SSL_CTX *, const int, struct sockaddr_in, const size_t);