#include "config.h"
#include "document.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>

size_t str_to_size_t(const char *s) {
  char *end;
  unsigned long long val = strtoull(s, &end, 10);
  if (end == s) {
    fprintf(stderr, "Invalid number: %s\n", s);
    return 0;
  }
  if (val > (size_t)-1) {
    fprintf(stderr, "Overflow converting '%s' to size_t\n", s);
    return 0;
  }
  return (size_t)val;
}

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

int write_to_conn(int connfd, const char *data) {
  size_t remaining = strlen(data);
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

document_t *document_from_stream(int connfd) {
  unsigned char buffer[BUFFER_SIZE];
  size_t raw_header_size = 0;
  unsigned char *raw_header = malloc(BUFFER_SIZE);
  char rollover[3] = {0};
  ssize_t nread;
  int header_complete = 0;
  size_t body_size = 0;
  unsigned char *raw_body = NULL;
  body_t *body;
  header_t *header;
  document_t *document;
  while (!header_complete && (nread = read(connfd, buffer, BUFFER_SIZE)) > 0) {
    raw_header = realloc(raw_header, raw_header_size + nread + 1);
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
        body = parse_body((const char *)raw_body, body_size);
        free(raw_body);
        free(raw_header);
      }
    }
  }
  return create_document(header, body);
}

int handle_conn(void *arg) {
  int connfd = *(int *)arg;
  free(arg);
  printf("client (id:%d) connected\n", connfd);
  document_t *request_document = document_from_stream(connfd);
  const char *target =
      fetch_body(request_document->header->request_line->target);

  body_t *response_body =
      create_body(request_document->header->request_line->target);
  header_t *response_header = create_default_header();
  response_header->type = RESPONSE;
  if (!response_body) {
    response_header->response_line =
        create_response_line(NOT_FOUND, "HTTP/1.1");
  } else {
    response_header->response_line = create_response_line(OK, "HTTP/1.1");
  }
  document_t *response_document =
      create_document(response_header, response_body);
  const char *response = serialize_document(response_document);
  write_to_conn(connfd, response);
  close(connfd);
  printf("client(id:%d) disconnected\n", connfd);
  return EXIT_SUCCESS;
}

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
    connfd = accept(sockfd, NULL, NULL);
    if (connfd < 0) {
      continue;
    }
    pthread_t tid;
    int *pconn = malloc(sizeof(int));
    *pconn = connfd;
    thrd_create(&tid, handle_conn, pconn);
    thrd_detach(tid);
  }
  return EXIT_SUCCESS;
}
