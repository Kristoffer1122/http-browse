#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "url_parse/url_parse.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef struct {
    char *scheme, *host, *page, *port;
    char **url;
} Url;

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
    if (strstr(argv[1], "HTTPS") == NULL && strstr(argv[1], "http") == NULL) {
        target[0] = "HTTPS";
        target[1] = argv[1];
        target[2] = "/";

    } else {
        target[0] = "HTTP";
        target[1] = argv[1];
        target[2] = "/";
    }

    printf("url: %s://%s%s\n", target[0], target[1], target[2]);
    return target;
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int sockfd;

    char buf[2056];
    int byte_count;

    Url url;
    url.url = get_args(argc, argv);
    url.scheme = url.url[0];
    url.host = url.url[1];
    url.page = url.url[2];
    url.port = strcmp(url.scheme, "https") == 0 ? "443" : "80";

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

    // i know if we have a socket, it could return -1 with errno EINPROGRESS,
    // but for now im not implementing non-blocking sockets, so if we get -1, we
    // will just exit with an error
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected!\n");

    printf("Sending GET Request...\n");
    char header[256];
    snprintf(header, sizeof(header), "GET %s %s/1.1\r\nHost: %s:%s\r\n\r\n",
             url.page, url.scheme, url.host, url.port);
    printf("Header:\n%s", header);

    if (send(sockfd, header, strlen(header), 0) == -1) {
        perror("Send Failed");
        exit(EXIT_FAILURE);
    }
    printf("GET Sent...\n");

    // add null terminator to end of buffer
    // since recv does not add it
    byte_count = recv(sockfd, buf, sizeof(buf) - 1, 0);
    buf[byte_count] = 0;
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

    free(url.url);
    free(res);

    return 0;
}
