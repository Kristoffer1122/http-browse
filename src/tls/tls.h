#pragma once

#include <openssl/ssl.h>
#include <openssl/err.h>

typedef struct {
    SSL_CTX *ctx;
    SSL *ssl;

}HTTPS_Request;

extern HTTPS_Request https_req;

// init openssl HTTPS_Request struct
void init_openssl(HTTPS_Request *https_req);

// connect using TLS and setup the SSL struct
void connect_tls(HTTPS_Request *https_req, int sockfd, char *host);

void shutdown_tls(HTTPS_Request *https_req);
