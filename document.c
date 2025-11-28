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
    header->response_line = create_response_line(OK, VERSION);
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

unsigned char *serialize_document(document_t *document) {
  unsigned char *header = serialize_header(document->header);
  size_t header_len = strlen(header);
  size_t body_len = 0;
  unsigned char *body_str = NULL;
  header_item_t *cl = get_header_item(document->header, "CONTENT-LENGTH");
  if (cl && document->body) {
    body_str = serialize_body(document->body);
    body_len = strlen(body_str);
  }
  size_t total_len = header_len + body_len + 1; // +1 for '\0'
  char *output = malloc(total_len);
  if (!output)
    return NULL;
  memcpy(output, header, header_len);
  if (body_str)
    memcpy(output + header_len, body_str, body_len);
  output[total_len - 1] = '\0';
  return output;
}
