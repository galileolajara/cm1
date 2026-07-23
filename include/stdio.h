##include <stdio.h>

#ifndef __EMSCRIPTEN__
##if 0
typedef struct __sFILE FILE;
FILE *stdout;
##endif

extern int fflush(FILE *stream);
#endif

int cm1_printf_begin() {
   return 0;
}
int cm1_printf_s(int n, const char *fmt, const char *s) {
   return n + printf(fmt, s);
}
int cm1_printf_c(int n, const char *fmt, int s) {
   return n + printf(fmt, s);
}
int cm1_printf_d(int n, const char *fmt, int s) {
   return n + printf(fmt, s);
}
int cm1_printf_x(int n, const char *fmt, int s) {
   return n + printf(fmt, s);
}
int cm1_printf_f(int n, const char *fmt, double s) {
   return n + printf(fmt, s);
}
int cm1_printf_end(int n, const char *s) {
   return n + printf("%s", s);
}
