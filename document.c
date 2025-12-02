#include "document.h"
#include "config.h"
#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *size_t_to_string(size_t value) {
  char buffer[32];
  int needed = snprintf(buffer, sizeof(buffer), "%zu", value);
  if (needed < 0) {
    return NULL;
  }
  char *result = malloc((size_t)needed + 1);
  if (!result) {
    return NULL;
  }
  snprintf(result, needed + 1, "%zu", value);
  return result;
}

/**
 * @brief Creates a new document with the given header and body.
 *
 * If no header is provided, a default header will be created using the RESPONSE type and an OK response line.
 * If no body is provided, the document will have no body.
 * The function also attaches the content-length header item to the header with the size of the body.
 *
 * @param header The header for the document.
 * @param body The body of the document.
 * @return A new document with the given header and body, or NULL if an error occurred.
 */
document_t *create_document(header_t *header, body_t *body) {
  if (!header) {
    header = create_default_header();
    header->type = RESPONSE;
    header->response_line = create_response_line(OK, VERSION);
  }
  document_t *document = malloc(sizeof(document_t));
  if (document == NULL) {
    return NULL;
  }
  document->header = header;
  if (!body) {
    document->body = NULL;
    return document;
  }
  document->body = body;
  attach_header(document->header,
                create_header_item("content-length",
                                   size_t_to_string(document->body->size)));
  return document;
}

/**
 * @brief Serializes a document object into a byte array.
 *
 * The serialized data includes the header and the body of the document, if it is not empty.
 *
 * @param document Pointer to the document object to serialize.
 * @param size Pointer to a variable that will hold the size of the serialized data.
 * @return A pointer to the serialized data, or NULL if an error occurred.
 */
unsigned char *serialize_document(document_t *document, size_t *size) {
  unsigned char *header = serialize_header(document->header);
  size_t header_len = strlen((char *)header);
  size_t total_len = header_len + 1; // +1 for '\0'
  if (document->body) {
    total_len += document->body->size;
  }
  unsigned char *output = malloc(total_len);
  if (!output)
    return NULL;
  memcpy(output, header, header_len);
  if (document->body)
    memcpy(output + header_len, document->body->data, document->body->size);
  *size = total_len;
  output[total_len - 1] = '\0';
  return output;
}

/**
 * @brief Destroys a document and its components.
 *
 * This function destroys a document and all of its components, including the header and body. It is important to call this f
function when you are done with a document to avoid memory leaks.
 *
 * @param document The document to destroy.
 */
void destroy_document(document_t *document) {
  if (!document) {
    return;
  }
  if (document->header) {
    destroy_header(document->header);
  }
  if (document->body) {
    destroy_body(document->body);
  }
  free(document);
}
