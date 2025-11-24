#ifndef HEADER
#define HEADER

#define SP " "
#define CRLF "\r\n"
#define LF "\n"
#define CR "\r"

typedef enum REQUEST_METHOD {
  GET,
  POST,
  OPTIONS,
  HEAD,
  PUT,
  DELETE,
  TRACE,
  CONNECT
} REQUEST_METHOD_T;

typedef enum RESPONSE_CODE {
  CONTINUE = 100,
  SWITCHING_PROCTOLS = 101,
  PROCESSING = 102,
  EARLY_HINTS = 103,
  OK = 200,
  CREATED = 201,
  ACCEPTED = 202,
  NON_AUTHORATIVE_INFORMATION = 203,
  NO_CONTENT = 204,
  RESET_CONTENT = 205,
  PARTIAL_CONTENT = 206,
  MULTI_STATUS = 207,
  ALREADY_REPORTED = 208,
  IM_USED = 226,
  MULTIPLE_CHOICES = 300,
  FOUND = 302,
  SEE_OTHER = 303,
  NOT_MODIFIED = 304,
  USE_PROXY = 305,
  UNUSED = 306,
  TEMPORARY_REDIRECT = 307,
  PERMANENT_REDIRECT = 308,
  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  PAYMENT_REQUIRED = 402,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  METHOD_NOT_ALLOWED = 405,
  NOT_ACCEPTABLE = 406,
  PROXY_AUHENTICAION_REQUIRED = 407,
  REQUEST_TIMEOUT = 408,
  CONFLICT = 409,
  GONE = 410,
  LENGTH_REQUIRED = 411,
  PRECONDITION_FAILED = 412,
  CONTENT_TOO_LARGE = 413,
  URI_TOO_LONG = 414,
  UNSUPPORTED_MEDIA_TYPE = 415,
  EXPECTATION_FAILED = 417,
  IM_A_TEAPOT = 418,
  MISDIRECTED_REQUEST = 421,
  UNPROCESSED_CONTENT = 422,
  LOCKED = 423,
  FAILED_DEPENDENCY = 424,
  TOO_EARLY = 425,
  UPGRADE_REQUIRED = 426,
  PRECONDITION_REQUIRED = 428,
  TOO_MANY_REQUESTS = 429,
  REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
  UNAVAILABLE_FOR_LEGAL_REASONS = 451,
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  BAD_GATEWAY = 502,
  SERVICE_UNAVAILABLE = 503,
  GATEWAY_TIMEOUT = 504,
  HTTP_VERSION_NOT_SUPPORTED = 505,
  VARIANT_ALSO_NEGOTIONATE = 506,
  INSUFFICIENT_STORAGE = 507,
  LOOP_DETECTED = 508,
  NOT_EXTENDED = 510,
  NETWORK_AUTHENTICATION_REQUIRED = 511
} RESPONSE_CODE_T;

typedef enum DOCUMENT_TYPE { REQUEST, RESPONSE } DOCUMENT_TYPE_T;

typedef struct header_item {
  char *key;
  char *value;
} header_item_t;

typedef struct header_request_line {
  REQUEST_METHOD_T method;
  char *version;
  char *target;
} header_request_line_t;

typedef struct header_response_line {
  RESPONSE_CODE_T code;
  char *version;
} header_response_line_t;

typedef struct header {
  int count;
  DOCUMENT_TYPE_T type;
  header_request_line_t *request_line;
  header_response_line_t *response_line;
  header_item_t **items;
} header_t;

header_t *parse_header(unsigned char *raw_header);
header_item_t *get_header_item(header_t *header, char *name);
header_item_t *create_header_item(char *key, char *value);
header_t *create_default_header();
const char *serialize_header(header_t *header);
const char *get_response_code_string(RESPONSE_CODE_T code);
void attach_header(header_t *header, header_item_t *item);
header_response_line_t *create_response_line(RESPONSE_CODE_T code,
                                             char *version);
#endif // !HEADER
