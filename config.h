#ifndef CONFIG
#define CONFIG

#define BUFFER_SIZE 1024
#ifdef PROD
#define PORT 80
#endif
#ifndef PROD
#define PORT 42069
#endif // !
#define TARGET_DIRECTORY "target"
#define DEFAULT_INDEX "index.htm"

#define VERSION "HTTP/1.1"

#endif // CONFIG
