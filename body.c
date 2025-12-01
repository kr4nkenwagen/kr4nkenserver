#include "body.h"
#include "config.h"
#include "response.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

body_t *parse_body(unsigned char *raw_body, size_t size) {
  body_t *body = malloc(sizeof(body_t));
  if (body == NULL) {
    return NULL;
  }
  body->data = malloc(size + 1);
  if (body->data == NULL) {
    free(body);
    return NULL;
  }
  body->size = size;
  memcpy(body->data, raw_body, body->size);
  return body;
}

body_t *create_body(const char *target) {
  body_t *body = malloc(sizeof(body_t));
  if (!body) {
    return NULL;
  }
  unsigned char *data = fetch_body((char *)target);
  if (!data) {
    return NULL;
  }
  char *out[BUFFER_SIZE] = {0};
  if (is_image_file((char *)target, out)) {
    body->size = file_size((char *)target);

  } else {
    size_t size = 0;
    while (data[size++] != '\0') {
    }
    body->size = size - 1;
  }
  body->data = malloc(body->size);
  memcpy(body->data, data, body->size);
  if (!body->data) {
    free(body);
    return NULL;
  }
  return body;
}

void destroy_body(body_t *body) {
  if (!body) {
    return;
  }
  if (body->data) {
    free(body->data);
  }
  free(body);
}
unsigned char *serialize_body(body_t *body) { return body->data; }
