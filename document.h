#ifndef DOCUMENT
#define DOCUMENT

#include "body.h"
#include "header.h"

typedef struct document {
  header_t *header;
  body_t *body;
} document_t;

document_t *create_document(header_t *header, body_t *body);
unsigned char *serialize_document(document_t *document);

#endif // !DOCUMENT
