#ifndef RESPONSE_DOC
#define RESPONSE_DOC
#include "document.h"
#include <stdbool.h>

document_t *create_response(RESPONSE_CODE_T code, body_t *body);
unsigned char *fetch_body(char *target);
bool is_image_file(char *path, char **out);
#endif // !RESPONSE
