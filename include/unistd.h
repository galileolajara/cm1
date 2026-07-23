##include <unistd.h>
extern int close(int fd);
extern char *getcwd(char *buffer, size_t size);
extern ssize_t lseek(int fd, ssize_t offset, int origin);
extern ssize_t read(int fd, void *buffer, size_t size);
extern int unlink(const char *path);
extern ssize_t write(int fd, const void *buffer, size_t size);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
