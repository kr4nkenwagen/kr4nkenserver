#include <stdlib.h>

#ifndef BODY
#define BODY

typedef struct body {
  size_t size;
  unsigned char *data;
} body_t;

body_t *parse_body(unsigned char *raw_body, size_t size);
body_t *create_body(const char *target);
unsigned char *serialize_body(body_t *body);
void destroy_body(body_t *body);

#endif // !BODY
