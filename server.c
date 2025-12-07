#include "config.h"
#include "document.h"
#include "header.h"
#include "response.h"
#include "utils.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief Reads exactly `count` bytes from the file descriptor `fd` into the
buffer `buf`.
 *
 * This function reads as many bytes as necessary to fill the entire buffer, or
until an error occurs. It returns the total n number of bytes read.
 *
 * @param fd The file descriptor to read from.
 * @param buf The buffer to read into.
 * @param count The maximum number of bytes to read.
 * @return The total number of bytes read, or a negative value on error.
 */
ssize_t read_full(int fd, unsigned char *buf, size_t count) {
  size_t total_read = 0;
  while (total_read < count) {
    ssize_t n = read(fd, buf + total_read, count - total_read);
    if (n <= 0)
      return n;
    total_read += n;
  }
  return total_read;
}

/**
 * @brief Write data to an open connection.
 *
 * This function writes data to an open connection, starting at the given file
descriptor. It takes a buffer of unsigned char chars and a length in bytes as
input, and returns -1 on error or the number of bytes written if successful.
 *
 * @param connfd The file descriptor for the open connection.
 * @param data The buffer containing the data to write.
 * @param length The length of the data buffer in bytes.
 * @return The number of bytes written, or -1 on error.
 */
int write_to_conn(int connfd, unsigned char *data, size_t length) {
  size_t remaining = length;
  size_t idx = 0;
  while (remaining > 0) {
    ssize_t to_write = remaining < BUFFER_SIZE ? remaining : BUFFER_SIZE;
    ssize_t n = write(connfd, data + idx, to_write);
    if (n <= 0) {
      perror("write");
      return -1;
    }
    idx += n;
    remaining -= n;
  }
  return 0;
}

/**
 * @brief Reads an HTTP request from the given socket and returns a document
object.
 *
 * This function reads data from the socket until it receives a complete HTTP
request. It then parses the request into its header and body parts, and returns
a document object that represents the entire request.
 *
 * @param connfd The socket file descriptor.
 * @return A document object representing the received HTTP request.
 */
document_t *document_from_stream(int connfd) {
  unsigned char buffer[BUFFER_SIZE];
  size_t raw_header_size = 0;
  unsigned char *raw_header = malloc(BUFFER_SIZE);
  char rollover[3] = {0};
  ssize_t nread;
  int header_complete = 0;
  size_t body_size = 0;
  unsigned char *raw_body = NULL;
  body_t *body = NULL;
  header_t *header = NULL;
  document_t *document;
  while (!header_complete && (nread = read(connfd, buffer, BUFFER_SIZE)) > 0) {
    void *tmp = realloc(raw_header, raw_header_size + nread + 1);
    if (!tmp) {
      free(raw_header);
      return NULL;
    }
    raw_header = tmp;
    int header_end = -1;
    for (int i = 0; i < nread; i++) {
      if (rollover[0] == '\r' && rollover[1] == '\n' && rollover[2] == '\r' &&
          buffer[i] == '\n') {
        header_end = i;
        break;
      }
      rollover[0] = rollover[1];
      rollover[1] = rollover[2];
      rollover[2] = buffer[i];
    }
    if (header_end == -1) {
      memcpy(raw_header + raw_header_size, buffer, nread);
      raw_header_size += nread;
    } else {
      memcpy(raw_header + raw_header_size, buffer, header_end);
      raw_header_size += header_end;
      raw_header[raw_header_size] = '\0';
      header_complete = 1;
      if (strlen((const char *)raw_header) < 10) {
        free(raw_header);
        return NULL;
      }
      header = parse_header(raw_header);
      header_item_t *content_length = get_header_item(header, "CONTENT-LENGTH");
      if (content_length) {
        body_size = str_to_size_t(content_length->value);
        raw_body = malloc(body_size + 1);
        size_t remaining = nread - header_end - 1;
        if (remaining > 0) {
          size_t to_copy = remaining < body_size ? remaining : body_size;
          memcpy(raw_body, buffer + header_end + 1, to_copy);
        }
        if (remaining < body_size) {
          read_full(connfd, raw_body + remaining, body_size - remaining);
        }
        body = parse_body(raw_body, body_size);
        free(raw_body);
      }
    }
  }
  free(raw_header);
  return create_document(header, body, REQUEST);
}

/**
 * @brief Handle a GET request
 *
 * This function handles a GET request by translating the target URL into a file
path, fetching the contents of that file, an and then creating a response
document with the appropriate content type and sending it back to the client.
 *
 * @param request The request document
 * @param connfd The connection socket file descriptor
 */
void handle_GET(document_t *request, int connfd) {
  char *translated_target =
      translate_target(request->header->request_line->target);
  unsigned char *target = fetch_body(translated_target);
  body_t *response_body = create_body(translated_target);
  document_t *response_document =
      create_response(response_body ? OK : NOT_FOUND, response_body);
  char *file_type;
  if (is_image_file(translated_target, &file_type)) {
    attach_header(
        response_document->header,
        create_header_item("content-type", str_join("image/", file_type)));
  } else if (strcmp(file_type, "html") == 0 || strcmp(file_type, "htm") == 0) {
    attach_header(response_document->header,
                  create_header_item("content-type", "text/html"));
  } else if (strcmp(file_type, "css") == 0) {
    attach_header(response_document->header,
                  create_header_item("content-type", "text/css"));
  } else if (strcmp(file_type, "js") == 0) {
    attach_header(response_document->header,
                  create_header_item("content-type", "application/javascript"));
  }
  size_t size = 0;
  unsigned char *response = serialize_document(response_document, &size);
  write_to_conn(connfd, response, size);
  destroy_document(response_document);
}

/**
 * @brief Handles HTTP POST requests
 *
 * Handles HTTP POST requests by creating a new document and writing it to the
 * connection. The document contains the response body, which is generated based
 * on the target of the request. If the target is an image file, the content
 * type is set to `image/<file_type>`. If the target is an HTML or CSS file, the
 * content type is set to `text/html` and `text/css`, respectively. If the
 * target is a JavaScript file, the content type is set to
 * `application/javascript`.
 *
 * @param request The HTTP POST request document
 * @param connfd The connection file descriptor
 */
void handle_POST(document_t *request, int connfd) {
  char *translated_target =
      translate_target(request->header->request_line->target);
  unsigned char *target = fetch_body(translated_target);
  body_t *response_body = create_body(translated_target);
  document_t *response_document =
      create_response(response_body ? OK : NOT_FOUND, response_body);
  char *file_type;
  if (is_image_file(translated_target, &file_type)) {
    attach_header(
        response_document->header,
        create_header_item("content-type", str_join("image/", file_type)));
  } else if (strcmp(file_type, "html") == 0 || strcmp(file_type, "htm") == 0) {
    attach_header(response_document->header,
                  create_header_item("content-type", "text/html"));
  } else if (strcmp(file_type, "css") == 0) {
    attach_header(response_document->header,
                  create_header_item("content-type", "text/css"));
  } else if (strcmp(file_type, "js") == 0) {
    attach_header(response_document->header,
                  create_header_item("content-type", "application/javascript"));
  }
  size_t size = 0;
  unsigned char *response = serialize_document(response_document, &size);
  write_to_conn(connfd, response, size);
  destroy_document(response_document);
}

/**
 * @brief Handles a connection request from a client.
 *
 * This function accepts an incoming connection from a client and processes it
accordingly. It reads the request document fro from the socket, determines the
request method, and calls the appropriate handler function to handle the
request. Once the re response has been sent back to the client, the function
closes the socket and returns.
 *
 * @param arg A pointer to an integer representing the connection file
descriptor.
 * @return NULL
 */
void *handle_conn(void *arg) {
  int connfd = *(int *)arg;
  free(arg);
  printf("client (id:%d) connected\n", connfd);
  document_t *request_document = document_from_stream(connfd);
  if (!request_document->header || !request_document->header->request_line) {
    destroy_document(request_document);
    return NULL;
  }
  switch (request_document->header->request_line->method) {
  case GET:
    handle_GET(request_document, connfd);
    break;
  case POST:
    handle_POST(request_document, connfd);
    break;
  case OPTIONS:
    handle_POST(request_document, connfd);
    break;
  case HEAD:
    handle_POST(request_document, connfd);
    break;
  case PUT:
    handle_POST(request_document, connfd);
    break;
  case DELETE:
    handle_POST(request_document, connfd);
    break;
  case TRACE:
    handle_POST(request_document, connfd);
    break;
  case CONNECT:
    handle_POST(request_document, connfd);
    break;
  }
  destroy_document(request_document);
  close(connfd);
  printf("client(id:%d) disconnected\n", connfd);
  return NULL;
}

/**
 * @brief Sets up a server socket and listens on a specific port
 *
 * This function creates a server socket and binds it to a specific port. It
then enters a loop where it waits for incoming c connections and handles them in
a separate thread.
 *
 * @param PORT The port number to listen on
 * @return EXIT_SUCCESS if the server was successfully set up, or EXIT_FAILURE
if an error occurred
 */
int server() {
  printf("Starting server...\n");
  printf("Listening to port %d\n", PORT);
  int sockfd, connfd;
  struct sockaddr_in addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    return EXIT_FAILURE;
  }
  int opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(PORT);
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    return EXIT_FAILURE;
  }
  if (listen(sockfd, 10) < 0) {
    return EXIT_FAILURE;
  }
  while (1) {
    struct sockaddr_in conn_addr;
    socklen_t addr_len = sizeof(conn_addr);
    connfd = accept(sockfd, (struct sockaddr *)&addr, &addr_len);
    if (connfd < 0) {
      continue;
    }
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &conn_addr.sin_addr, ipstr, sizeof(ipstr));
    printf("accepted connection from %s:%d\n", ipstr,
           ntohs(conn_addr.sin_port));
    pthread_t tid;
    int *pconn = malloc(sizeof(int));
    *pconn = connfd;
    pthread_create(&tid, NULL, handle_conn, pconn);
    pthread_detach(tid);
  }
  return EXIT_SUCCESS;
}
