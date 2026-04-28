#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./parse_url/parser.h"
#include "./tls/tls.h"

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
    int sockfd;
    char buf[2056];
    int byte_count;

} HTTP_Request;

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    HTTPS_Request https_req = {0};

    init_openssl(&https_req);

    int sockfd;
    Url url;
    url.url = get_args(argc, argv);
    url.scheme = url.url[0];
    url.host = url.url[1];
    url.page = url.url[2];
    url.port = url.url[3];

    // get host info, make socket and connect it
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    // get address info
    if (getaddrinfo(url.host, url.port, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

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
    if (strcasecmp(url.scheme, "https") == 0) {
        connect_tls(&https_req, sockfd, url.host);
    } else if (strcasecmp(url.scheme, "http") != 0) {
        fprintf(stderr, "Invalid URL Scheme: %s\n", url.scheme);
        exit(EXIT_FAILURE);
    }
    printf("Connected! with %s\n", url.scheme);

    printf("Sending GET Request...\n");
    char header[256];

    if (strcmp(url.port, "80") == 0 || strcmp(url.port, "443") == 0) {
        snprintf(header, sizeof(header),
                 "GET %s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Connection: close\r\n\r\n",
                 url.page, url.host);
    } else {
        snprintf(header, sizeof(header),
                 "GET %s HTTP/1.1\r\n"
                 "Host: %s:%s\r\n"
                 "Connection: close\r\n\r\n",
                 url.page, url.host, url.port);
    }
    printf("Header:\n%s", header);
    printf("BYTES SENT: %zu\n", strlen(header));
    if (strcasecmp(url.scheme, "HTTPS") == 0) {
        if (SSL_write(https_req.ssl, header, strlen(header)) == -1) {
            perror("SSL_write Failed");
            exit(EXIT_FAILURE);
        }
    } else if (strcasecmp(url.scheme, "HTTP") == 0) {
        if (send(sockfd, header, strlen(header), 0) == -1) {
            perror("Send Failed");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Invalid URL Scheme: %s\n", url.scheme);
        exit(EXIT_FAILURE);
    }
    printf("GET Sent...\n");

    char buf[2056];
    int byte_count = 0;
    int total_bytes = 0;

    if (strcasecmp(url.scheme, "https") == 0) {

        while ((byte_count = SSL_read(https_req.ssl, buf, sizeof(buf))) > 0) {
            write(1, buf, byte_count);
            total_bytes += byte_count;
        }

        if (byte_count < 0) {
            perror("SSL_read");
        }

    } else if (strcasecmp(url.scheme, "http") == 0) {

        while ((byte_count = recv(sockfd, buf, sizeof(buf), 0)) > 0) {
            write(1, buf, byte_count);
            total_bytes += byte_count;
        }

        if (byte_count < 0) {
            perror("recv");
        }
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

    shutdown_tls(&https_req);
    free(url.url);
    free(res);

    return 0;
}
