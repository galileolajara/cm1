int cm1_printf_begin() {
   return 0;
}
int cm1_printf_s(int n, char* fmt, char* s) {
   return n + printf(fmt, s);
}
int cm1_printf_x(int n, char* fmt, int x) {
   return n + printf(fmt, x);
}
int cm1_printf_c(int n, char* fmt, int c) {
   return n + printf(fmt, c);
}
int cm1_printf_d(int n, char* fmt, int val) {
   return n + printf(fmt, val);
}
int cm1_printf_f(int n, char* fmt, double val) {
   return n + printf(fmt, val);
}
int cm1_printf_end(int n, char* str) {
   return n + printf("%s", str);
}
