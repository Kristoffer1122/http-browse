#include "tls.h"

void init_openssl(HTTPS_Request *https_req) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    https_req->ctx = NULL;
    https_req->ssl = NULL;
}

void connect_tls(HTTPS_Request *https_req, int sockfd, char *host) {
    https_req->ctx = SSL_CTX_new(TLS_client_method());
    if (https_req->ctx == NULL) {
        perror("SSL_CTX_new Failed");
        exit(EXIT_FAILURE);
    }

    https_req->ssl = SSL_new(https_req->ctx);
    if (https_req->ssl == NULL) {
        perror("SSL_new Failed");
        exit(EXIT_FAILURE);
    }

    SSL_set_fd(https_req->ssl, sockfd);
    SSL_set_tlsext_host_name(https_req->ssl, host);
    int ret = SSL_connect(https_req->ssl);
    if (ret <= 0) {
        int err = SSL_get_error(https_req->ssl, ret);
        printf("SSL_get_error: %d\n", err);
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void shutdown_tls(HTTPS_Request *https_req) {
    if (https_req->ssl) {
        SSL_shutdown(https_req->ssl);
        SSL_free(https_req->ssl);
    }
    if (https_req->ctx) {
        SSL_CTX_free(https_req->ctx);
    }
}
