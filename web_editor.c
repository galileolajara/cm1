// #define EXTRACT_WEB_CM1_C

#include "web-cm1.cp1.c"
#include <emscripten/emscripten.h>

uint64_t io_mem_v[1024 * 1024 * 16];

EMSCRIPTEN_KEEPALIVE
size_t io_limit() {
   return sizeof(io_mem_v);
}

#define STR_WITH_SIZE(x) x, sizeof(x) - 1

#define CM1_SRC_PATH "code.cm1.c"
#define CM1_CFG_PATH "code.cm1.cfg"
#define CM1_C_PATH "code.c"
#define CM1_BC_PATH "code.cm1"

EMSCRIPTEN_KEEPALIVE
void* io_mem() {
   int fd = open(CM1_CFG_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
   write(fd,
      STR_WITH_SIZE(
      "C\n"
      "cm1_printf_begin\n"
      "cm1_printf_s\n"
      "cm1_printf_d\n"
      "cm1_printf_x\n"
      "cm1_printf_f\n"
      "cm1_printf_c\n"
      "cm1_printf_end\n"
      )
   );
   close(fd);
   return io_mem_v;
}

#include "stdio.h.h"

EMSCRIPTEN_KEEPALIVE
int32_t compile(int32_t input_size) {
   int fd = open(CM1_SRC_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
   write(fd, include_stdio_h, include_stdio_h_len);
   write(fd,
		STR_WITH_SIZE(
		"void run();\n"
      // "# 1 \"code.cm1.c\"\n"
		)
	);
   write(fd, io_mem_v, input_size);
   close(fd);

   char* argv[5];
   argv[0] = "cm1";
   argv[1] = CM1_SRC_PATH;
   argv[2] = CM1_CFG_PATH;
   argv[3] = CM1_C_PATH;
   argv[4] = CM1_BC_PATH;
   _Frun_2(5, argv);

	#ifdef EXTRACT_WEB_CM1_C
   fd = open(CM1_C_PATH, O_RDONLY);
   size_t len = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   read(fd, io_mem_v, len);
   close(fd);
   return len;
	#else
   return 0;
   #endif
}

void run();

void cm1_init(const char* path);

EMSCRIPTEN_KEEPALIVE
void execute() {
	printf("Executing...\n");
	cm1_init(CM1_BC_PATH);
	run();
	printf("Executed successfully\n");
}

#include "web_cm1.c"
