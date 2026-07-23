##include <stdlib.h>
extern void *malloc(size_t size);
extern void *realloc(void* oldptr, size_t size);
extern void free(void *ptr);
extern void exit(int status);
extern char *getenv(const char *name);
extern char *realpath(const char *path, char *resolved_path);
