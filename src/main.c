#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

char **get_args(int argc, char *argv[]) {
  char **target;
  u_int8_t size = 3;

  target = malloc(sizeof(char *) * 3);

  if (argc < 5) {
    fprintf(stderr, "Usage: %s --ip <hostname> --port <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  for (int arg = 1; arg < argc; arg++) {
    if (strcmp(argv[arg], "--ip") == 0 && arg + 1 < argc) {
      target[0] = argv[arg + 1];
      arg++;
    } else if (strcmp(argv[arg], "--port") == 0 && arg + 1 < argc) {
      target[1] = argv[arg + 1];
      arg++;
    } else if (strcmp(argv[arg], "--page") == 0 && arg + 1 < argc) {
      target[2] = argv[arg + 1];
      arg++;
    }
  }
  return target;
}

int main(int argc, char *argv[]) {
  struct addrinfo hints, *res;
  int sockfd;

  char buf[2056];
  int byte_count;

  char **target = get_args(argc, argv);
  char *target_ip = target[0];
  char *target_port = target[1];
  char *target_page = target[2];

  // get host info, make socket and connect it
  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  getaddrinfo(target_ip, target_port, &hints, &res);
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  // get address info
  struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
  char ipstr[INET_ADDRSTRLEN];
  inet_ntop(res->ai_family, &(ipv4->sin_addr), ipstr, sizeof ipstr);

  printf("Connecting to Target: %s:%d\n", ipstr, ntohs(ipv4->sin_port));
  connect(sockfd, res->ai_addr, res->ai_addrlen);
  printf("Connected!\n");

  printf("Sending GET Request...\n");
  char header[256];
  snprintf(header, sizeof(header), "GET %s HTTP/1.1\r\nHost: %s:%s\r\n\r\n",
           target_page, target_ip, target_port);
  printf("Header:\n%s", header);
  send(sockfd, header, strlen(header), 0);
  printf("GET Sent...\n");

  // add null terminator to end of buffer
  // since recv does not add it
  byte_count = recv(sockfd, buf, sizeof(buf) - 1, 0);
  buf[byte_count] = 0;
  printf("Recived %d bytes of data in buf\n", byte_count);
  printf("%s", buf);

  free(target);
  free(res);

  return 0;
}
