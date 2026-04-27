#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Networking
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/tls1.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef struct {
    char *scheme, *host, *page, *port;
    char **url;
} Url;

typedef struct {
    SSL_CTX *ctx;
    SSL *ssl;

} HTTPS_Request;

typedef struct {
    int sockfd;
    char buf[2056];
    int byte_count;

} HTTP_Request;

char **get_args(int argc, char *argv[]) {
    char **target;

    //           1        2      3
    // url == scheme://host.com/path
    target = malloc(sizeof(char *) * 3);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s example.com\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // automatically add port 80 and page /
    if (strstr(argv[1], "https") == NULL && strstr(argv[1], "http") == NULL) {
        target[0] = "HTTPS";
        target[1] = argv[1];
        target[2] = "/";

    } else {
        target[0] = "HTTP";
        target[1] = argv[1];
        if (strstr(argv[1], "/") == NULL) {
            target[2] = "/";
        } else {
            char *slash = strchr(argv[1], '/');
            *slash = 0;
            target[2] = slash;
        }
    }

    printf("url: %s://%s%s\n", target[0], target[1], target[2]);
    return target;
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    HTTPS_Request https_req = {0};
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    int sockfd;

    char buf[2056];
    int byte_count = 0;

    Url url;
    url.url = get_args(argc, argv);
    url.scheme = url.url[0];
    url.host = url.url[1];
    url.page = url.url[2];
    url.port = strcmp(url.scheme, "HTTPS") == 0 ? "443" : "80";

    // get host info, make socket and connect it
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(url.host, url.port, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // get address info
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(res->ai_family, &(ipv4->sin_addr), ipstr, sizeof ipstr);
    printf("Connecting to url: %s:%d\n", ipstr, ntohs(ipv4->sin_port));

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Connect Failed");
        exit(EXIT_FAILURE);
    }

    // i know if we have a socket, it could return -1 with errno
    // EINPROGRESS, but for now im not implementing non-blocking sockets, so
    // if we get -1, we will just exit with an error
    if (strcmp(url.scheme, "HTTPS") == 0) {
        https_req.ctx = SSL_CTX_new(TLS_client_method());
        printf("Setting TLS Host Name: %s\n", url.host);
        if (https_req.ctx == NULL) {
            perror("SSL_CTX_new Failed");
            exit(EXIT_FAILURE);
        }

        https_req.ssl = SSL_new(https_req.ctx);
        if (https_req.ssl == NULL) {
            perror("SSL_new Failed");
            exit(EXIT_FAILURE);
        }

        SSL_set_fd(https_req.ssl, sockfd);
        SSL_set_tlsext_host_name(https_req.ssl, url.host);
        int ret = SSL_connect(https_req.ssl);
        if (ret <= 0) {
            int err = SSL_get_error(https_req.ssl, ret);
            printf("SSL_get_error: %d\n", err);
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(url.scheme, "HTTP") != 0) {
        fprintf(stderr, "Invalid URL Scheme: %s\n", url.scheme);
        exit(EXIT_FAILURE);
    }
    printf("Connected! with %s\n", url.scheme);

    printf("Sending GET Request...\n");
    char header[256];
    snprintf(header, sizeof(header),
             "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
             url.page, url.host);
    printf("Header:\n%s", header);

    if (strcmp(url.scheme, "HTTPS") == 0) {
        if (SSL_write(https_req.ssl, header, strlen(header)) == -1) {
            perror("SSL_write Failed");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(url.scheme, "HTTP") == 0) {
        if (send(sockfd, header, strlen(header), 0) == -1) {
            perror("Send Failed");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(url.scheme, "HTTP") == 0) {
        if (send(sockfd, header, strlen(header), 0) == -1) {
            perror("Send Failed");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Invalid URL Scheme: %s\n", url.scheme);
        exit(EXIT_FAILURE);
    }
    printf("GET Sent...\n");

    if (strcmp(url.scheme, "HTTPS") == 0) {
        byte_count = SSL_read(https_req.ssl, buf, sizeof(buf) - 1);
        if (byte_count == -1) {
            perror("SSL_read Failed");
            exit(EXIT_FAILURE);
        }
    } else if (strcmp(url.scheme, "HTTP") == 0) {
        byte_count = recv(sockfd, buf, sizeof(buf) - 1, 0);
        buf[byte_count] = 0;
    }
    printf("Recived %d bytes of data:\n", byte_count);
    printf("%s", buf);

    // close the connection
    if (shutdown(sockfd, SHUT_RDWR) == -1) {
        perror("Shutdown Failed");
        exit(EXIT_FAILURE);
    }
    if (close(sockfd))
        perror("Close Failed");
    printf("Connection Closed\n");

    SSL_shutdown(https_req.ssl);
    SSL_free(https_req.ssl);
    SSL_CTX_free(https_req.ctx);
    free(url.url);
    free(res);

    return 0;
}
