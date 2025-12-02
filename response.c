#include "config.h"
#include "document.h"
#include "header.h"
#include "utils.h"
#include <stdbool.h>
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
  body_t *body = create_body((unsigned char *)PAGE_404);
  document_t *document = create_document(header, body);
  return document;
}

/**
 * @brief Creates a response document based on the given code and body.
 *
 * @param code The response code.
 * @param body The response body.
 * @return A pointer to the created response document.
 */
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

/**
 * @brief Fetches the contents of a web page and returns it as a string.
 *
 * This function takes in a URL as input, fetches the contents of the web page, and returns it as a string. If the URL does n
not point to an existing web page, it will return NULL.
 *
 * @param target The URL of the web page to be fetched.
 * @return The contents of the web page as a string, or NULL if the URL does not point to an existing web page.
 */
unsigned char *fetch_body(char *target) {
  bool needs_malloc = false;
  unsigned char *content;
  if (target[strlen(target) - 1] == '/') {
    char *target_root = str_join(target, "index.htm");
    content = load_file(target_root);
    free(target_root);
  } else {
    content = load_file(target);
  }
  if (content) {
    return content;
  }
  return NULL;
}
