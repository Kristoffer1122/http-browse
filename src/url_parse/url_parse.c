#include "./url_parse.h"
#include <stdio.h>

char *url_parse(char *str) {
  for (size_t i = 0; i < strlen(str); i++) {
    if (str[i] == '/') {
      // remove / from requested page
      memmove(&str[i], &str[i + 1], strlen(str) - i);
    }
    return str;
  }

  return fprintf(stderr, "Could not parse url\n"), NULL;
}
