#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **get_args(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s example.com\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //           1        2      3    4
    // url == scheme://host.com/path port
    char **target = malloc(sizeof(char *) * 4);
    target[0] = target[1] = target[2] = target[3] = NULL;

    // parse the url and split it into scheme, host, and page
    char *url = argv[1];
    char *host;

    if (strncmp(url, "https://", 8) == 0) {
        host = url + 8;
        target[0] = "https";
    } else if (strncmp(url, "http://", 7) == 0) {
        host = url + 7;
        target[0] = "http";
    } else { // assume https like if ./program google.com
        host = url;
        target[0] = "https";
    }

    char *slash = strchr(host, '/');
    if (slash) {
        *slash = '\0';
        size_t len = strlen(slash + 1) + 2;
        target[2] = malloc(len);
        snprintf(target[2], len, "/%s", slash + 1);
        target[1] = host;
    } else {
        target[1] = host;
        target[2] = "/";
    }

    char *colon = strchr(target[1], ':');
    if (colon) {
        *colon = '\0';
        target[3] = colon + 1;
    } else {
        if (strcasecmp(target[0], "https") == 0) {
            target[3] = "443";
        } else if (strcasecmp(target[0], "http") == 0) {
            target[3] = "80";
        } else if (strcasecmp(target[1], "localhost") == 0) {
            target[3] = "3000";
        }
    }

    printf("url: %s://%s:%s%s\n", target[0], target[1], target[3], target[2]);
    return target;
}
