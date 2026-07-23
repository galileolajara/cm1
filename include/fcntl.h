##include <fcntl.h>
#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_CREAT  0x0200
#define O_TRUNC  0x0400
// #define O_BINARY 0

extern int open(const char *path, int flags, int mode);

#define CM1_OPEN_PICK(_1, _2, _3, NAME, ...) NAME(_1, _2, _3)
#define CM1_OPEN2(path, flags, ignored) open(path, flags, 0)
#define CM1_OPEN3(path, flags, mode) open(path, flags, mode)
#define open(...) CM1_OPEN_PICK(__VA_ARGS__, CM1_OPEN3, CM1_OPEN2, 0)
