#include "config.h"
#include "document.h"
#include "header.h"
#include <stdio.h>
#include <string.h>

static document_t *create_OK_document(body_t *body) {
  header_t *header = create_default_header();
  header->type = RESPONSE;
  header->response_line = create_response_line(OK, "HTTP/1.1");
  document_t *document = create_document(header, body);
  return document;
}

static document_t *create_INTERNAL_SERVER_ERROR_document() {
  header_t *header = create_default_header();
  header->type = RESPONSE;
  header->response_line =
      create_response_line(INTERNAL_SERVER_ERROR, "HTTP/1.1");
  document_t *document = create_document(header, NULL);
  return document;
}

static document_t *create_NOT_FOUND_document() {
  header_t *header = create_default_header();
  header->type = RESPONSE;
  header->response_line = create_response_line(NOT_FOUND, "HTTP/1.1");
  body_t *body = create_body("/404.htm");
  document_t *document = create_document(header, body);
  return document;
}

document_t *create_response(RESPONSE_CODE_T code, body_t *body) {
  switch (code) {

  case OK:
    return create_OK_document(body);
  case NOT_FOUND:
    return create_NOT_FOUND_document();
  case CONTINUE:
  case SWITCHING_PROCTOLS:
  case PROCESSING:
  case EARLY_HINTS:
  case CREATED:
  case ACCEPTED:
  case NON_AUTHORATIVE_INFORMATION:
  case NO_CONTENT:
  case RESET_CONTENT:
  case PARTIAL_CONTENT:
  case MULTI_STATUS:
  case ALREADY_REPORTED:
  case IM_USED:
  case MULTIPLE_CHOICES:
  case FOUND:
  case SEE_OTHER:
  case NOT_MODIFIED:
  case USE_PROXY:
  case UNUSED:
  case TEMPORARY_REDIRECT:
  case PERMANENT_REDIRECT:
  case BAD_REQUEST:
  case UNAUTHORIZED:
  case PAYMENT_REQUIRED:
  case FORBIDDEN:
  case METHOD_NOT_ALLOWED:
  case NOT_ACCEPTABLE:
  case PROXY_AUHENTICAION_REQUIRED:
  case REQUEST_TIMEOUT:
  case CONFLICT:
  case GONE:
  case LENGTH_REQUIRED:
  case PRECONDITION_FAILED:
  case CONTENT_TOO_LARGE:
  case URI_TOO_LONG:
  case UNSUPPORTED_MEDIA_TYPE:
  case EXPECTATION_FAILED:
  case IM_A_TEAPOT:
  case MISDIRECTED_REQUEST:
  case UNPROCESSED_CONTENT:
  case LOCKED:
  case FAILED_DEPENDENCY:
  case TOO_EARLY:
  case UPGRADE_REQUIRED:
  case PRECONDITION_REQUIRED:
  case TOO_MANY_REQUESTS:
  case REQUEST_HEADER_FIELDS_TOO_LARGE:
  case UNAVAILABLE_FOR_LEGAL_REASONS:
  case NOT_IMPLEMENTED:
  case BAD_GATEWAY:
  case SERVICE_UNAVAILABLE:
  case GATEWAY_TIMEOUT:
  case HTTP_VERSION_NOT_SUPPORTED:
  case VARIANT_ALSO_NEGOTIONATE:
  case INSUFFICIENT_STORAGE:
  case LOOP_DETECTED:
  case NOT_EXTENDED:
  case NETWORK_AUTHENTICATION_REQUIRED:
  case INTERNAL_SERVER_ERROR:
    return create_INTERNAL_SERVER_ERROR_document();
  }
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
  char path[BUFFER_SIZE];
  snprintf(path, BUFFER_SIZE, "%s%s", TARGET_DIRECTORY, target);
  if (target[strlen(target) - 1] == '/') {
    snprintf(path, sizeof(path), "%s%sindex.htm", TARGET_DIRECTORY, target);
  }
  char *content = load_file(path);
  if (content) {
    return content;
  }
  return NULL;
}
