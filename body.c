#include "body.h"
#include "response.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

body_t *parse_body(const char *raw_body, size_t size) {
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
  strcpy(body->data, raw_body);
  return body;
}

body_t *create_body(const char *target) {
  body_t *body = malloc(sizeof(body_t));
  if (!body) {
    return NULL;
  }
  const char *data = fetch_body(target);
  if (!data) {
    return NULL;
  }
  body->size = strlen(data);
  body->data = strdup(data);
  if (!body->data) {
    free(body);
    return NULL;
  }
  return body;
}

const char *serialize_body(body_t *body) { return body->data; }
