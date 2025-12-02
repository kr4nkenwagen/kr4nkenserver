#include "body.h"
#include "config.h"
#include "response.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Parses a raw HTTP body into an internal representation.
 *
 * This function takes in a raw HTTP body and parses it into the internal
 * representation of the body, which is a struct containing the data and size
 * of the body. The memory for the body is allocated using malloc() and should
 * be freed by the caller using free(). If there is an error parsing the body,
 * this function returns NULL.
 *
 * @param raw_body Pointer to the raw HTTP body.
 * @param size Size of the raw HTTP body in bytes.
 * @return A pointer to the internal representation of the HTTP body, or NULL if
 * there was an error parsing the body.
 */
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

/**
 * @brief Creates a new body object from the given target.
 *
 * This function creates a new body object from the given target, which can be either a file path or a string. If th
the target is a file path, the function will read the contents of the file and return it as a body object. If the ta
target is a string, the function will simply copy the string into a new body object.
 *
 * @param target The target to create a body from. Can be either a file path or a string.
 * @return A new body object containing the contents of the given target.
 */
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

/**
 * @brief Destroys a body and its associated data.
 *
 * The function destroys the given body and releases any memory allocated for its data.
 *
 * @param body A pointer to the body to be destroyed. If NULL, the function does nothing.
 */
void destroy_body(body_t *body) {
  if (!body) {
    return;
  }
  if (body->data) {
    free(body->data);
  }
  free(body);
}

/**
 * @brief Serializes a body object into a byte array.
 *
 * The serialized data can be sent over the network or written to a file for later use.
 *
 * @param body The body object to serialize.
 * @return A pointer to the serialized data.
 */
unsigned char *serialize_body(body_t *body) { return body->data; }
