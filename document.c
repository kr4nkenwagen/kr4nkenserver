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
document_t *create_document(header_t *header, body_t *body) {
  if (!header) {
    header = create_default_header();
    header->type = RESPONSE;
    header->response_line = create_response_line(OK, "HTTP/1.1");
  }
  document_t *document = malloc(sizeof(document_t));
  if (document == NULL) {
    return NULL;
  }
  document->header = header;
  if (!body) {
    return document;
  }
  document->body = body;
  attach_header(document->header,
                create_header_item("CONTENT-LENGTH",
                                   size_t_to_string(document->body->size)));
  return document;
}

const char *serialize_document(document_t *document) {
  const char *header = serialize_header(document->header);
  char *output = calloc(BUFFER_SIZE, 1);
  strcat(output, header);
  if (get_header_item(document->header, "CONTENT-LENGTH")) {
    const char *body = serialize_body(document->body);
    strcat(output, body);
  }
  return output;
}
