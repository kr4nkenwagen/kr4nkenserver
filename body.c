#include "body.h"
#include "config.h"
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

static char *load_file(const char *filepath) {
  FILE *file = fopen(filepath, "rb");
  if (!file)
    return NULL;
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);
  if (file_size < 0) {
    fclose(file);
    return NULL;
  }
  char *content = malloc(file_size + 1);
  if (!content) {
    fclose(file);
    return NULL;
  }
  size_t read_size = fread(content, 1, file_size, file);
  fclose(file);
  if (read_size != file_size) {
    free(content);
    return NULL;
  }
  content[file_size] = '\0';
  return content;
}

const char *fetch_body(const char *target) {
  char path[1024];
  snprintf(path, sizeof(path), "%s%s", TARGET_DIRECTORY, target);
  char *content = load_file(path);
  if (content) {
    return content;
  }
  snprintf(path, sizeof(path), "%s%s/index.htm", TARGET_DIRECTORY, target);
  content = load_file(path);
  if (content) {
    return content;
  }
  return NULL;
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
  body->data = malloc(body->size);
  if (!body->data) {
    free(body);
    return NULL;
  }
  strcpy(body->data, data);
  return body;
}
const char *serialize_body(body_t *body) { return body->data; }
