#ifndef UTILS
#define UTILS
#include <stdbool.h>
#include <stddef.h>

char *translate_target(char *target);
size_t file_size(char *filepath);
bool is_image_file(char *path, char **out);
unsigned char *load_file(const char *filepath);
size_t str_to_size_t(const char *s);
char *str_join(const char *a, const char *b);
#endif // !UTILS
