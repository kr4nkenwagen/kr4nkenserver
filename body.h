#include <stdlib.h>

#ifndef BODY
#define BODY

typedef struct body {
  size_t size;
  char *data;
} body_t;

body_t *parse_body(const char *raw_body, size_t size);
body_t *create_body(const char *target);
const char *serialize_body(body_t *body);
const char *fetch_body(const char *target);

#endif // !BODY
