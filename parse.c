#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

struct _Tcm1_Ttoken_data_i32 {
   const char* path;
   uint32_t row;
   uint32_t col;
   int32_t id;
   int32_t id2;
   int32_t id3;
};
struct _Tcm1_Ttoken_data_f32 {
   const char* path;
   uint32_t row;
   uint32_t col;
   float f32;
};
struct _Tcm1_Ttoken_data_f64 {
   const char* path;
   uint32_t row;
   uint32_t col;
   double f64;
};
struct _Tcm1_Ttoken_data_u64 {
   const char* path;
   uint32_t row;
   uint32_t col;
   uint64_t u64;
};
struct _Tcm1_Ttoken_data_ptr {
   const char* path;
   uint32_t row;
   uint32_t col;
   void* ptr;
   void* ptr2;
   void* ptr3;
};
union _Tcm1_Ttoken_data {
   struct _Tcm1_Ttoken_data_i32 basic;
   struct _Tcm1_Ttoken_data_f32 f32;
   struct _Tcm1_Ttoken_data_f64 f64;
   struct _Tcm1_Ttoken_data_u64 u64;
   struct _Tcm1_Ttoken_data_ptr ptr;
};

extern int _Tcm1_Glast_token;
extern int _Tcm1_Glast_row;
extern int _Tcm1_Glast_col;
extern int _Tcm1_Grow;
extern int _Tcm1_Gcol;
typedef int _Tcm1_Ttoken;
typedef uint32_t _Tcm1_Tid;
typedef uint8_t _Tcm1_Tstars;
typedef uint8_t _Tcm1_Tassign_op;
typedef uint8_t _Tcm1_Tmath_op;
typedef uint8_t _Tcm1_Tunary_op;
extern char* input_path;
extern void* current_type;

struct _Tcm1_Tspace;
struct _Tcm1_Tstmt;
struct _Tcm1_Texpr;
struct _Tcm1_Ttype;
struct _Tcm1_Tline_info;

#include "export.h"
#include "cm1_parse.c"

struct cm1_lexer {
   const char *start;
   const char *cursor;
};

void* parser_malloc(size_t size) {
   static yyParser parser;
   return size <= sizeof(parser) ? &parser : NULL;
}

void parser_free(void* parser) {
   (void)parser;
}

void* _Tcm1_Tparser_Falloc_0() {
   return cm1ParseAlloc(parser_malloc);
}

void _Tcm1_Tparser_Ffree_1(void* parser) {
   cm1ParseFree(parser, parser_free);
}

#include "lex.c"
#include "crc32c.c"
#include "num.c"
