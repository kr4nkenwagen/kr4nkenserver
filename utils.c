#include "config.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Converts a size_t value to a string.
 *
 * This function converts a size_t value to a string representation that can be
used for printing or storing. The result resulting string will have the format
"zu", where "u" is the unsigned integer representation of the value.
 *
 * @param value The size_t value to convert.
 * @return A newly allocated string containing the converted value, or NULL if
an error occurred.
 */
char *size_t_to_string(size_t value) {
  char buffer[32];
  int needed = snprintf(buffer, sizeof(buffer), "%zu", value);
  if (needed < 0) {
    return NULL;
  }
  char *result = malloc((size_t)needed + 1);
  if (!result) {
    return NULL;
  }
  snprintf(result, needed + 1, "%zu", value);
  return result;
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
 * @brief Translates a target name into its corresponding path in the target
directory.
 *
 * This function takes a target name as input and returns its corresponding path
in the target directory.
 * The target directory is defined by the TARGET_DIRECTORY constant, which
should be set to the desired directory where targe targets are stored.
 *
 * @param target Name of the target to be translated.
 * @return Translated path of the target in the target directory.
 */
char *translate_target(const char *target) {
  if (!target) {
    return NULL;
  }
  size_t len = strlen(TARGET_DIRECTORY) + strlen(target) + 1;
  char *translated_target = malloc(len);
  if (!translated_target) {
    return NULL;
  }
  translated_target[0] = '\0';
  strncat(translated_target, TARGET_DIRECTORY, len - 1);
  strncat(translated_target, target, len - strlen(translated_target) - 1);
  return translated_target;
}

/**
 * @brief Gets the size of a file in bytes.
 *
 * This function opens a file and returns its size in bytes. If the file cannot
 * be opened or does not exist, it returns 0.
 *
 * @param filepath The path to the file whose size needs to be obtained.
 * @return The size of the file in bytes.
 */
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

static char *get_file_extension(char *path) {
  char *out = calloc(BUFFER_SIZE, 1);
  for (int idx = strlen(path); idx > 0; idx--) {
    if (path[idx] == '.') {
      if (idx == strlen(path)) {
        break;
      }
      out = strdup(path + idx + 1);
      break;
    }
    if (path[idx] == '/') {
      out = "html";
      break;
    }
  }
  return out;
}

/**
 * @brief Determine if a file is an image file
 *
 * This function checks if a given file is an image file by looking at its magic
 * number. The function returns true if the file is an image file, false
 * otherwise.
 *
 * @param path The path to the file
 * @return True if the file is an image file, false otherwise
 */
bool is_image_file(char *path, char **out) {
  unsigned char buf[BUFFER_SIZE];
  FILE *f = fopen(path, "rb");
  if (!f) {
    *out = get_file_extension(path);
    return false;
  }
  size_t n = fread(buf, 1, sizeof(buf), f);
  fclose(f);
  if (n < 12) {
    *out = get_file_extension(path);
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
    while (i < n && isspace(buf[i])) {
      i++;
    }
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
  *out = get_file_extension(path);
  return false;
}

/**
 * @brief Loads the contents of a file into memory.
 *
 * This function reads the contents of a file into memory and returns a pointer
 * to the data. The caller is responsible for freeing the memory using the
 * `free()` function.
 *
 * @param filepath The path to the file to load.
 * @return A pointer to the data in memory, or NULL if there was an error.
 */
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

/**
 * @brief Convert a string to a size_t value.
 *
 * This function converts a string representation of a number to an unsigned
 * integer. The string is assumed to be in base 10. If the conversion fails or
 * the resulting value is greater than the maximum value that can be represented
 * by a size_t, the function returns 0.
 *
 * @param s Pointer to the string to convert.
 * @return The converted value on success, 0 on failure.
 */
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

/**
 * @brief Joins two strings together.
 *
 * The resulting string will contain a concatenation of the two input strings,
 * separated by a single space.
 *
 * @param a First string to join.
 * @param b Second string to join.
 * @return A new string that is the result of joining `a` and `b`.
 */
char *str_join(const char *a, const char *b) {
  if (!a && !b) {
    return NULL;
  }
  if (!b) {
    return (char *)a;
  }
  if (!a) {
    return (char *)b;
  }
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
