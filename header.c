#include "header.h"
#include "config.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *RESPONSE_CODE_STRINGS[] = {
    [CONTINUE] = "Continue",
    [SWITCHING_PROCTOLS] = "Switching Proctols",
    [PROCESSING] = "Processing",
    [EARLY_HINTS] = "Early Hints",
    [OK] = "Ok",
    [CREATED] = "Created",
    [ACCEPTED] = "Accepted",
    [NON_AUTHORATIVE_INFORMATION] = "Non Authorative Information",
    [NO_CONTENT] = "No Content",
    [RESET_CONTENT] = "Reset Content",
    [PARTIAL_CONTENT] = "Partial Content",
    [MULTI_STATUS] = "Multi Status",
    [ALREADY_REPORTED] = "Already Reported",
    [IM_USED] = "IM Used",
    [MULTIPLE_CHOICES] = "Multiple Choices",
    [FOUND] = "Found",
    [SEE_OTHER] = "See Other",
    [NOT_MODIFIED] = "Not Modified",
    [USE_PROXY] = "Use Proxy",
    [UNUSED] = "Unused",
    [TEMPORARY_REDIRECT] = "Temporary Redirect",
    [PERMANENT_REDIRECT] = "Permanent Redirect",
    [BAD_REQUEST] = "Bad Request",
    [UNAUTHORIZED] = "Unauthorized",
    [PAYMENT_REQUIRED] = "Payment Required",
    [FORBIDDEN] = "Forbidden",
    [NOT_FOUND] = "Not Found",
    [METHOD_NOT_ALLOWED] = "Method Not Allowed",
    [NOT_ACCEPTABLE] = "Not Acceptable",
    [PROXY_AUHENTICAION_REQUIRED] = "Proxy Auhenticaion Required",
    [REQUEST_TIMEOUT] = "Request Timeout",
    [CONFLICT] = "Conflict",
    [GONE] = "Gone",
    [LENGTH_REQUIRED] = "Length Required",
    [PRECONDITION_FAILED] = "Precondition Failed",
    [CONTENT_TOO_LARGE] = "Content Too Large",
    [URI_TOO_LONG] = "Uri Too Long",
    [UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type",
    [EXPECTATION_FAILED] = "Expectation Failed",
    [IM_A_TEAPOT] = "Im A Teapot",
    [MISDIRECTED_REQUEST] = "Misdirected Request",
    [UNPROCESSED_CONTENT] = "Unprocessed Content",
    [LOCKED] = "Locked",
    [FAILED_DEPENDENCY] = "Failed Dependency",
    [TOO_EARLY] = "Too Early",
    [UPGRADE_REQUIRED] = "Upgrade Required",
    [PRECONDITION_REQUIRED] = "Precondition Required",
    [TOO_MANY_REQUESTS] = "Too Many Requests",
    [REQUEST_HEADER_FIELDS_TOO_LARGE] = "Request Header Fields Too Large",
    [UNAVAILABLE_FOR_LEGAL_REASONS] = "Unavailable For Legal Reasons",
    [INTERNAL_SERVER_ERROR] = "Internal Server Error",
    [NOT_IMPLEMENTED] = "Not Implemented",
    [BAD_GATEWAY] = "Bad Gateway",
    [SERVICE_UNAVAILABLE] = "Service Unavailable",
    [GATEWAY_TIMEOUT] = "Gateway Timeout",
    [HTTP_VERSION_NOT_SUPPORTED] = "Http Version Not Supported",
    [VARIANT_ALSO_NEGOTIONATE] = "Variant Also Negotionates",
    [INSUFFICIENT_STORAGE] = "Insufficient Storage",
    [LOOP_DETECTED] = "Loop Detected",
    [NOT_EXTENDED] = "Not Extended",
    [NETWORK_AUTHENTICATION_REQUIRED] = "Network Authentication Required"};

static void str_to_upper(char *s) {
  while (*s) {
    if (isalpha(*s)) {
      *s = toupper((unsigned char)*s);
    }
    s++;
  }
}

/**
 * @brief Get the string representation of a request method.
 *
 * This function returns the string representation of a given request method.
The possible values for the method are GET, POS POST, PUT, OPTIONS, HEAD,
CONNECT, TRACE, and DELETE.
 *
 * @param method The request method to get the string representation of.
 * @return The string representation of the request method.
 */
const char *get_method_string(REQUEST_METHOD_T method) {
  switch (method) {
  case GET:
    return "GET";
  case POST:
    return "POST";
  case PUT:
    return "PUT";
  case OPTIONS:
    return "OPTIONS";
  case HEAD:
    return "HEAD";
  case CONNECT:
    return "CONNECT";
  case TRACE:
    return "TRACE";
  case DELETE:
    return "DELETE";
  }
}

/**
 * @brief Gets a string representation of the response code.
 *
 * The function takes an integer value representing the response code and
 * returns a string containing the name of the code. If the input is not a valid
 * response code, the function will return "Unknown Code".
 *
 * @param code The response code to get the string representation for.
 * @return A string containing the name of the response code, or "Unknown Code"
 * if the input is not a valid code.
 */
const char *get_response_code_string(RESPONSE_CODE_T code) {
  if (code < 100 || code > 511)
    return "Unknown Code";
  return RESPONSE_CODE_STRINGS[code];
}

static header_request_line_t *parse_request_line(unsigned char *raw_header) {
  header_request_line_t *request_line = malloc(sizeof(header_request_line_t));
  int raw_header_index = 0;
  while (raw_header[raw_header_index++] != ' ') {
  }
  char *method = calloc(raw_header_index, 1);
  strncpy(method, (char *)raw_header, raw_header_index - 1);
  if (strcmp(method, "GET") == 0) {
    request_line->method = GET;
  } else if (strcmp(method, "POST") == 0) {
    request_line->method = POST;
  }
  int target_start = raw_header_index;
  while (raw_header[raw_header_index++] != ' ') {
  }
  request_line->target = calloc(raw_header_index - target_start, 1);
  strncpy(request_line->target, (char *)raw_header + target_start,
          raw_header_index - target_start - 1);
  int protocol_start = raw_header_index;
  while (raw_header[raw_header_index++] != '\n') {
  }
  request_line->version = calloc(raw_header_index - protocol_start - 2, 1);
  strncpy(request_line->version, (char *)raw_header + protocol_start,
          raw_header_index - 2 - protocol_start);
  return request_line;
}

static int find_header_count(unsigned char *raw_header) {
  int count = 0;
  for (int i = 0; raw_header[i] != '\0'; i++) {
    if (raw_header[i] == '\n') {
      count++;
    }
  }
  return count - 1;
}

/**
 * @brief Finds a header item in the given header by its name.
 *
 * The function searches for a header item with the specified name in the given
header structure. If an item with that name i is found, it is returned.
Otherwise, NULL is returned.
 *
 * @param header The header structure to search in.
 * @param name The name of the header item to find.
 * @return The found header item, or NULL if not found.
 */
header_item_t *get_header_item(header_t *header, char *name) {
  for (int i = 0; i < header->count; i++) {
    if (strcmp(header->items[i]->key, name) == 0) {
      return header->items[i];
    }
  }
  return NULL;
}

/**
 * @brief Attaches a header item to the end of the header list.
 *
 * This function attaches a header item to the end of the header list, which is
a linked list of header items. The function t takes two arguments: a pointer to
the header list and a pointer to the header item to be attached.
 *
 * @param header A pointer to the header list.
 * @param item A pointer to the header item to be attached.
 * @return Nothing.
 */
void attach_header(header_t *header, header_item_t *item) {
  header->items =
      realloc(header->items, (header->count + 1) * sizeof(header_item_t *));
  if (!header->items) {
    return;
  }
  header->items[header->count] = item;
  header->count++;
}

/**
 * @brief Creates a new header item with the given key and value.
 *
 * This function creates a new header item with the specified key and value. The
key and value are copied, so they can be fre freed after this function returns.
 *
 * @param key The key for the header item.
 * @param value The value for the header item.
 * @return A pointer to the newly created header item. NULL is returned if an
error occurs.
 */
header_item_t *create_header_item(char *key, char *value) {
  header_item_t *item = malloc(sizeof(header_item_t));
  if (!item) {
    return NULL;
  }
  item->key = strdup(key);
  item->value = strdup(value);
  if (!item->value || !item->key) {
    free(item->key);
    free(item->value);
    free(item);
    return NULL;
  }
  return item;
}

/**
 * @brief Get the current time in UTC/GMT format.
 *
 * This function returns a string representation of the current time in UTC/GMT
format. The returned string is allocated on t the heap using `calloc()`, and it
is the responsibility of the caller to free the memory when it is no longer
needed.
 *
 * @param none
 * @return A string representing the current time in UTC/GMT format.
 */
char *get_time() {
  time_t now = time(NULL);
  struct tm gmt;
  gmtime_r(&now, &gmt); // convert to UTC / GMT

  char *buf = calloc(100, 1);
  if (!buf) {
    return "";
  }
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &gmt);

  return buf;
}

/**
 * @brief Creates a default HTTP header.
 *
 * This function creates a default HTTP header that includes essential headers
such as "Connection", "Date", "Server", and "K "Keep-Alive". The "Connection"
header is set to "keep-alive" and the "Keep-Alive" header is set to "timeout=5,
max=997".
 *
 * @return A pointer to a `header_t` structure containing the default HTTP
header.
 */
header_t *create_default_header() {
  header_t *header = malloc(sizeof(header_t));
  header->request_line = NULL;
  header->count = 5;
  header->items = malloc(header->count * sizeof(header_item_t *));
  header->items[0] = create_header_item("connection", "keep-alive");
  header->items[1] = create_header_item("date", get_time());
  header->items[2] = create_header_item("server", "kr4nkenserver");
  header->items[3] = create_header_item("server-version", "0.1alpha");
  header->items[4] = create_header_item("keep-alive", "timeout=5, max=997");
  return header;
}

/**
 * @brief Serialize a header structure into a binary buffer.
 *
 * The serialized buffer contains the following fields in the order specified:
 * - The method, target, and version of the request line (if type is REQUEST).
 * - The HTTP version and status code of the response line (if type is
 * RESPONSE).
 * - Each header item in the list, in the format "key: value\r\n".
 * - A final newline character ("\r\n").
 *
 * @param header The header structure to serialize.
 * @return The serialized binary buffer, or NULL if an error occurred.
 */
unsigned char *serialize_header(header_t *header) {
  size_t capacity = BUFFER_SIZE;
  size_t len = 0;
  unsigned char *output = malloc(capacity);
  if (!output)
    return NULL;
  output[0] = '\0';

#define APPEND(s)                                                              \
  do {                                                                         \
    size_t slen = strlen(s);                                                   \
    if (len + slen + 1 > capacity) {                                           \
      capacity = (len + slen + 1) * 2;                                         \
      unsigned char *tmp = realloc(output, capacity);                          \
      if (!tmp) {                                                              \
        free(output);                                                          \
        return NULL;                                                           \
      }                                                                        \
      output = tmp;                                                            \
    }                                                                          \
    memcpy(output + len, s, slen);                                             \
    len += slen;                                                               \
    output[len] = '\0';                                                        \
  } while (0)
  if (header->type == REQUEST) {
    APPEND(get_method_string(header->request_line->method));
    APPEND(SP);
    APPEND(header->request_line->target);
    APPEND(SP);
    APPEND(header->request_line->version);
    APPEND(LF);
  } else if (header->type == RESPONSE) {
    APPEND(header->response_line->version);
    APPEND(SP);
    char code[4];
    snprintf(code, sizeof(code), "%d", header->response_line->code);
    APPEND(code);
    APPEND(SP);
    APPEND(get_response_code_string(header->response_line->code));
    APPEND(LF);
  }
  for (int i = 0; i < header->count; i++) {
    APPEND(header->items[i]->key);
    APPEND(": ");
    APPEND(header->items[i]->value);
    APPEND(LF);
  }
  APPEND(CRLF);
#undef APPEND
  return output;
}

/**
 * @brief Creates a new header response line with the given version and code.
 *
 * The created response line is returned, or NULL if an error occurred.
 *
 * @param version The version of the HTTP protocol to use in the response line.
 * @param code The response code to use in the response line.
 * @return The newly created header response line.
 */
header_response_line_t *create_response_line(RESPONSE_CODE_T code,
                                             char *version) {
  header_response_line_t *response_line =
      malloc(sizeof(header_response_line_t));
  if (!response_line) {
    return NULL;
  }
  response_line->version = malloc(strlen(version) + 1);
  if (!response_line->version) {
    free(response_line);
    return NULL;
  }
  strcpy(response_line->version, version);
  response_line->version = version;
  response_line->code = code;
  return response_line;
}

static void combine_duplicate_header_items(header_t *header) {
  for (int i = 0; i < header->count; i++) {
    if (header->items[i] == NULL) {
      continue;
    }
    for (int x = i + 1; x < header->count; x++) {
      if (header->items[x] == NULL) {
        continue;
      }
      if (strcmp(header->items[i]->key, header->items[x]->key) == 0) {
        strcat(header->items[i]->value, ", ");
        strcat(header->items[i]->value, header->items[x]->value);
        free(header->items[x]);
        header->items[x] = NULL;
      }
    }
  }
  for (int i = 0; i < header->count; i++) {
    if (header->items[i] == NULL) {
      for (int x = i + 1; x < header->count; x++) {
        header->items[x - 1] = header->items[x];
      }
      header->count--;
      i--;
    }
  }
}

/**
 * @brief Parse HTTP header from raw data.
 *
 * This function parses an HTTP header from a block of raw data and returns a
 * header struct with all the parsed information.
 *
 * @param raw_header A pointer to the raw header data.
 * @return The parsed header struct.
 */
header_t *parse_header(unsigned char *raw_header) {
  header_t *header = malloc(sizeof(header_t));
  header->count = find_header_count(raw_header);
  header->items = malloc(header->count * sizeof(header_item_t *));
  header->request_line = parse_request_line(raw_header);
  int raw_header_index = 0;
  while (raw_header[raw_header_index++] != '\n') {
  }
  for (int i = 0; i < header->count; i++) {
    header->items[i] = malloc(sizeof(header_item_t));
    int end = 0;
    int divider = 0;
    while (raw_header[raw_header_index + end] != '\r') {
      if (divider == 0 && raw_header[raw_header_index + end] == ':') {
        divider = end;
      }
      end++;
    }
    header->items[i]->key = malloc(divider + 1);
    header->items[i]->value = malloc(end - divider);

    memcpy(header->items[i]->key, raw_header + raw_header_index, divider);
    header->items[i]->key[divider] = '\0';
    str_to_upper(header->items[i]->key);
    memcpy(header->items[i]->value, raw_header + raw_header_index + divider + 1,
           end - divider - 1);
    header->items[i]->value[end - divider - 1] = '\0';
    raw_header_index += end + 2;
  }
  combine_duplicate_header_items(header);
  return header;
}

/**
 * @brief Destroys a HTTP header object.
 *
 * This function destroys a HTTP header object, including its request or
 * response line and any items. The memory allocated for the header is also
 * freed.
 *
 * @param header The header object to destroy.
 */
void destroy_header(header_t *header) {
  printf("%s\n", serialize_header(header));
  if (header->type == REQUEST) {
    free(header->request_line->target);
    free(header->request_line->version);
    free(header->request_line);
  }
  if (header->type == RESPONSE) {
    free(header->response_line);
  }
  for (int i = 0; i < header->count; i++) {
    free(header->items[i]->key);
    if (header->items[i]->value) {
      free(header->items[i]->value);
    }
    free(header->items[i]);
  }
  free(header);
}
