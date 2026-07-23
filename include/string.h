##include <string.h>
extern void *memchr(const void *ptr, int ch, size_t len);
extern int memcmp(const void *lhs, const void *rhs, size_t len);
extern void *memcpy(void *des, const void *src, size_t len);
extern void *memset(void *des, int value, size_t len);
extern char *strcpy(char *des, const char *src);
extern char *strdup(const char *str);
extern size_t strlen(const char *str);
extern char *strrchr(const char *str, int ch);
extern char *strtok(char *str, const char *delimiters);
