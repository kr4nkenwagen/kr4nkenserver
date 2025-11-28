#ifndef RESPONSE_DOC
#define RESPONSE_DOC
#include "document.h"

document_t *create_response(RESPONSE_CODE_T code, body_t *body);
unsigned char *fetch_body(const char *target);

#endif // !RESPONSE
