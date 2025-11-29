
#include "config.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *translate_target(char *target) {
  char *translated_target = malloc(strlen(target) + strlen(TARGET_DIRECTORY));
  snprintf(translated_target, BUFFER_SIZE, "%s%s", TARGET_DIRECTORY, target);
  return translated_target;
}

size_t file_size(char *filepath) {
  FILE *file = fopen(filepath, "rb");
  if (!file) {
    return 0;
  }
  fseek(file, 0, SEEK_END);
  return ftell(file);
}

static bool starts_with(const unsigned char *buf, const char *str, size_t n) {
  return memcmp(buf, str, n) == 0;
}

bool is_image_file(char *path, char **out) {
  unsigned char buf[BUFFER_SIZE];
  FILE *f = fopen(path, "rb");
  if (!f) {
    return false;
  }
  size_t n = fread(buf, 1, sizeof(buf), f);
  fclose(f);
  if (n < 12) {
    return false;
  }
  /* --- JPEG: FF D8 FF --- */
  if (buf[0] == 0xFF && buf[1] == 0xD8 && buf[2] == 0xFF) {
    *out = strdup("jpeg");
    return true;
  }
  /* --- PNG: 89 50 4E 47 0D 0A 1A 0A --- */
  if (n >= 8 && buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E &&
      buf[3] == 0x47 && buf[4] == 0x0D && buf[5] == 0x0A && buf[6] == 0x1A &&
      buf[7] == 0x0A) {
    *out = strdup("png");
    return true;
  }
  /* --- GIF: GIF87a or GIF89a --- */
  if (n >= 6 &&
      (starts_with(buf, "GIF87a", 6) || starts_with(buf, "GIF89a", 6))) {
    *out = strdup("gif");
    return true;
  }
  /* --- WEBP (RIFF....WEBP) --- */
  if (n >= 12 && starts_with(buf, "RIFF", 4) &&
      starts_with(buf + 8, "WEBP", 4)) {
    *out = strdup("webp");
    return true;
  }
  /* --- AVIF / HEIF / HEIC: ISO Base Media File Format ---
     Look for ftyp 'avif', 'avis', 'mif1', 'heic', 'heix', 'hevc' etc.
  */
  if (n >= 12 && starts_with(buf + 4, "ftyp", 4)) {
    const char *brands[] = {"avif", "avis", "mif1", "heic",
                            "heix", "hevc", "hevx"};
    for (size_t i = 0; i < sizeof(brands) / sizeof(brands[0]); i++) {
      if (starts_with(buf + 8, brands[i], 4))
        *out = strdup("avif");
      return true;
    }
  }
  /* --- SVG: text-based, starts with '<svg' or '<?xml ... <svg' ---
     We check the first non-whitespace character.
  */
  {
    size_t i = 0;
    while (i < n && isspace(buf[i]))
      i++;

    if (i < n && buf[i] == '<') {
      const char *needle1 = "<svg";
      const char *needle2 = "<?xml";

      if (n - i >= strlen(needle1) &&
          strncasecmp((char *)&buf[i], needle1, strlen(needle1)) == 0)
        *out = strdup("svg");
      return true;

      if (n - i >= strlen(needle2) &&
          strncasecmp((char *)&buf[i], needle2, strlen(needle2)) == 0)
        *out = strdup("svg");
      return true;
    }
  }
  return false;
}

unsigned char *load_file(const char *filepath) {
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
  unsigned char *content = malloc(file_size + 1);
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

char *str_join(const char *a, const char *b) {
  size_t len_a = strlen(a);
  size_t len_b = strlen(b);

  char *result = malloc(len_a + len_b + 1); // +1 for null terminator
  if (!result)
    return NULL;

  memcpy(result, a, len_a);
  memcpy(result + len_a, b, len_b);
  result[len_a + len_b] = '\0';

  return result;
}
