/*!max:re2c */

#include <stdint.h>
#include <limits.h>
#include <stdlib.h>

#if INT_MAX == INT8_MAX
#define CM1_C_TOKEN_INT CM1_TOKEN_INT8
#define CM1_C_TOKEN_UINT CM1_TOKEN_UINT8
#elif INT_MAX == INT16_MAX
#define CM1_C_TOKEN_INT CM1_TOKEN_INT16
#define CM1_C_TOKEN_UINT CM1_TOKEN_UINT16
#elif INT_MAX == INT32_MAX
#define CM1_C_TOKEN_INT CM1_TOKEN_INT32
#define CM1_C_TOKEN_UINT CM1_TOKEN_UINT32
#elif INT_MAX == INT64_MAX
#define CM1_C_TOKEN_INT CM1_TOKEN_INT64
#define CM1_C_TOKEN_UINT CM1_TOKEN_UINT64
#else
#error "CM1 does not support this int width"
#endif

#if SHRT_MAX == INT8_MAX
#define CM1_C_TOKEN_SHORT CM1_TOKEN_INT8
#define CM1_C_TOKEN_USHORT CM1_TOKEN_UINT8
#elif SHRT_MAX == INT16_MAX
#define CM1_C_TOKEN_SHORT CM1_TOKEN_INT16
#define CM1_C_TOKEN_USHORT CM1_TOKEN_UINT16
#elif SHRT_MAX == INT32_MAX
#define CM1_C_TOKEN_SHORT CM1_TOKEN_INT32
#define CM1_C_TOKEN_USHORT CM1_TOKEN_UINT32
#elif SHRT_MAX == INT64_MAX
#define CM1_C_TOKEN_SHORT CM1_TOKEN_INT64
#define CM1_C_TOKEN_USHORT CM1_TOKEN_UINT64
#else
#error "CM1 does not support this short width"
#endif

#if LONG_MAX == INT8_MAX
#define CM1_C_TOKEN_LONG CM1_TOKEN_INT8
#define CM1_C_TOKEN_ULONG CM1_TOKEN_UINT8
#elif LONG_MAX == INT16_MAX
#define CM1_C_TOKEN_LONG CM1_TOKEN_INT16
#define CM1_C_TOKEN_ULONG CM1_TOKEN_UINT16
#elif LONG_MAX == INT32_MAX
#define CM1_C_TOKEN_LONG CM1_TOKEN_INT32
#define CM1_C_TOKEN_ULONG CM1_TOKEN_UINT32
#elif LONG_MAX == INT64_MAX
#define CM1_C_TOKEN_LONG CM1_TOKEN_INT64
#define CM1_C_TOKEN_ULONG CM1_TOKEN_UINT64
#else
#error "CM1 does not support this long width"
#endif

#if LLONG_MAX == INT8_MAX
#define CM1_C_TOKEN_LLONG CM1_TOKEN_INT8
#define CM1_C_TOKEN_ULLONG CM1_TOKEN_UINT8
#elif LLONG_MAX == INT16_MAX
#define CM1_C_TOKEN_LLONG CM1_TOKEN_INT16
#define CM1_C_TOKEN_ULLONG CM1_TOKEN_UINT16
#elif LLONG_MAX == INT32_MAX
#define CM1_C_TOKEN_LLONG CM1_TOKEN_INT32
#define CM1_C_TOKEN_ULLONG CM1_TOKEN_UINT32
#elif LLONG_MAX == INT64_MAX
#define CM1_C_TOKEN_LLONG CM1_TOKEN_INT64
#define CM1_C_TOKEN_ULLONG CM1_TOKEN_UINT64
#else
#error "CM1 does not support this long long width"
#endif

#if SCHAR_MAX == INT8_MAX
#define CM1_C_TOKEN_SCHAR CM1_TOKEN_INT8
#elif SCHAR_MAX == INT16_MAX
#define CM1_C_TOKEN_SCHAR CM1_TOKEN_INT16
#elif SCHAR_MAX == INT32_MAX
#define CM1_C_TOKEN_SCHAR CM1_TOKEN_INT32
#elif SCHAR_MAX == INT64_MAX
#define CM1_C_TOKEN_SCHAR CM1_TOKEN_INT64
#else
#error "CM1 does not support this signed char width"
#endif

#if UCHAR_MAX == UINT8_MAX
#define CM1_C_TOKEN_UCHAR CM1_TOKEN_UINT8
#elif UCHAR_MAX == UINT16_MAX
#define CM1_C_TOKEN_UCHAR CM1_TOKEN_UINT16
#elif UCHAR_MAX == UINT32_MAX
#define CM1_C_TOKEN_UCHAR CM1_TOKEN_UINT32
#elif UCHAR_MAX == UINT64_MAX
#define CM1_C_TOKEN_UCHAR CM1_TOKEN_UINT64
#else
#error "CM1 does not support this unsigned char width"
#endif

#if CHAR_MIN < 0
#define CM1_C_TOKEN_CHAR CM1_C_TOKEN_SCHAR
#else
#define CM1_C_TOKEN_CHAR CM1_C_TOKEN_UCHAR
#endif

#if SIZE_MAX == UINT8_MAX
#define CM1_C_TOKEN_SIZE_T CM1_TOKEN_UINT8
#define CM1_C_TOKEN_SSIZE_T CM1_TOKEN_INT8
#elif SIZE_MAX == UINT16_MAX
#define CM1_C_TOKEN_SIZE_T CM1_TOKEN_UINT16
#define CM1_C_TOKEN_SSIZE_T CM1_TOKEN_INT16
#elif SIZE_MAX == UINT32_MAX
#define CM1_C_TOKEN_SIZE_T CM1_TOKEN_UINT32
#define CM1_C_TOKEN_SSIZE_T CM1_TOKEN_INT32
#elif SIZE_MAX == UINT64_MAX
#define CM1_C_TOKEN_SIZE_T CM1_TOKEN_UINT64
#define CM1_C_TOKEN_SSIZE_T CM1_TOKEN_INT64
#else
#error "CM1 does not support this size_t width"
#endif

extern char string_mem[1];

float cm1_parse_f32(const uint8_t* text) {
   return strtof((const char*)text, NULL);
}

double cm1_parse_f64(const uint8_t* text) {
   return strtod((const char*)text, NULL);
}

bool cm1_char_is_signed(void) {
   return CHAR_MIN < 0;
}

bool _Tcm1_Ffunc_is_bytecode_2(const char*, size_t);

uint8_t curly_brace_depth;

#define CURLY_BRACE_MODE_NONE 0
#define CURLY_BRACE_MODE_CM1_FUNC 1
#define CURLY_BRACE_MODE_C_FUNC 2
#define CURLY_BRACE_MODE_STRUCT 3

uint8_t curly_brace_mode;

void _Tcm1_Fclear_memory_lexer_0() {
   curly_brace_depth = 0;
   curly_brace_mode = 0;
}

static const char* cm1_c_code_end(const char* c) {
   enum {
      CM1_C_CODE_NORMAL,
      CM1_C_CODE_STRING,
      CM1_C_CODE_CHAR,
      CM1_C_CODE_LINE_COMMENT,
      CM1_C_CODE_BLOCK_COMMENT,
   } state = CM1_C_CODE_NORMAL;
   size_t level = 0;

   for (;;) {
      if (c[0] == '\0') {
         printf("Unterminated C code\n");
         exit(EXIT_FAILURE);
      }

      if (state == CM1_C_CODE_STRING || state == CM1_C_CODE_CHAR) {
         if (c[0] == '\\') {
            if (c[1] == '\0') {
               printf("Unterminated C code\n");
               exit(EXIT_FAILURE);
            }
            c += 2;
            continue;
         }
         if ((state == CM1_C_CODE_STRING && c[0] == '"')
            || (state == CM1_C_CODE_CHAR && c[0] == '\'')) {
            state = CM1_C_CODE_NORMAL;
         }
         c++;
         continue;
      }

      if (state == CM1_C_CODE_LINE_COMMENT) {
         if (c[0] == '\n') {
            state = CM1_C_CODE_NORMAL;
         }
         c++;
         continue;
      }

      if (state == CM1_C_CODE_BLOCK_COMMENT) {
         if (c[0] == '*' && c[1] == '/') {
            state = CM1_C_CODE_NORMAL;
            c += 2;
         } else {
            c++;
         }
         continue;
      }

      if (c[0] == '"') {
         state = CM1_C_CODE_STRING;
      } else if (c[0] == '\'') {
         state = CM1_C_CODE_CHAR;
      } else if (c[0] == '/' && c[1] == '/') {
         state = CM1_C_CODE_LINE_COMMENT;
         c += 2;
         continue;
      } else if (c[0] == '/' && c[1] == '*') {
         state = CM1_C_CODE_BLOCK_COMMENT;
         c += 2;
         continue;
      } else if (c[0] == '{') {
         level++;
      } else if (c[0] == '}') {
         if (level == 0) {
            return c;
         }
         level--;
      }
      c++;
   }
}

int cm1_lexer_scan(struct cm1_lexer* l) {
   const char *yyt1;
   const char *yyt2;
   const char *id_start;
   const char *id_space;

   const char *marker;
   const char *cursor = l->cursor;
   l->start = cursor;
   #define YYCTYPE  uint8_t
   #define YYCURSOR cursor
   #define YYMARKER marker
   #define YYFILL(n)   

   /*!re2c
   re2c:tags            = 1;
   re2c:indent:top      = 1;
   re2c:yyfill:enable   = 0;

   spaces = [ \n\t]+;
   kw_signed = "signed";
   kw_unsigned = "unsigned";
   kw_int = "int";
   kw_short = "short";
   kw_long = "long";
   decimal_digits = [0-9]+;
   hex_digits = [0-9a-fA-F]+;
   hex_integer = "0" [xX] hex_digits;
   octal_integer = "0" [0-7]+;
   decimal_exponent = [eE][+-]? decimal_digits;
   decimal_float =
      (decimal_digits "." [0-9]* | "." decimal_digits) decimal_exponent?
      | decimal_digits decimal_exponent;
   hex_float = "0" [xX]
      (hex_digits "." [0-9a-fA-F]* | "." hex_digits | hex_digits)
      [pP][+-]? decimal_digits;
   f32_num = (decimal_float | hex_float) [fF];
   f64_num = decimal_float | hex_float;
   string_literal = ["] ([^"\000\n\\] | [\\] [^\000\n])* ["];
   character_literal = ['] ([^'\000\n\\] | [\\] [^\000\n])+ ['];

   c_signed_int =
      kw_int
      | kw_signed
      | kw_signed spaces kw_int
      | kw_int spaces kw_signed;
   c_unsigned_int =
      kw_unsigned
      | kw_unsigned spaces kw_int
      | kw_int spaces kw_unsigned;

   c_signed_short =
      kw_short
      | kw_short spaces kw_int
      | kw_int spaces kw_short
      | kw_signed spaces kw_short
      | kw_short spaces kw_signed
      | kw_signed spaces kw_short spaces kw_int
      | kw_signed spaces kw_int spaces kw_short
      | kw_short spaces kw_signed spaces kw_int
      | kw_short spaces kw_int spaces kw_signed
      | kw_int spaces kw_signed spaces kw_short
      | kw_int spaces kw_short spaces kw_signed;
   c_unsigned_short =
      kw_unsigned spaces kw_short
      | kw_short spaces kw_unsigned
      | kw_unsigned spaces kw_short spaces kw_int
      | kw_unsigned spaces kw_int spaces kw_short
      | kw_short spaces kw_unsigned spaces kw_int
      | kw_short spaces kw_int spaces kw_unsigned
      | kw_int spaces kw_unsigned spaces kw_short
      | kw_int spaces kw_short spaces kw_unsigned;

   c_signed_long =
      kw_long
      | kw_long spaces kw_int
      | kw_int spaces kw_long
      | kw_signed spaces kw_long
      | kw_long spaces kw_signed
      | kw_signed spaces kw_long spaces kw_int
      | kw_signed spaces kw_int spaces kw_long
      | kw_long spaces kw_signed spaces kw_int
      | kw_long spaces kw_int spaces kw_signed
      | kw_int spaces kw_signed spaces kw_long
      | kw_int spaces kw_long spaces kw_signed;
   c_unsigned_long =
      kw_unsigned spaces kw_long
      | kw_long spaces kw_unsigned
      | kw_unsigned spaces kw_long spaces kw_int
      | kw_unsigned spaces kw_int spaces kw_long
      | kw_long spaces kw_unsigned spaces kw_int
      | kw_long spaces kw_int spaces kw_unsigned
      | kw_int spaces kw_unsigned spaces kw_long
      | kw_int spaces kw_long spaces kw_unsigned;

   c_signed_long_long =
      kw_long spaces kw_long
      | kw_int spaces kw_long spaces kw_long
      | kw_long spaces kw_int spaces kw_long
      | kw_long spaces kw_long spaces kw_int
      | kw_signed spaces kw_long spaces kw_long
      | kw_long spaces kw_signed spaces kw_long
      | kw_long spaces kw_long spaces kw_signed
      | kw_signed spaces kw_int spaces kw_long spaces kw_long
      | kw_signed spaces kw_long spaces kw_int spaces kw_long
      | kw_signed spaces kw_long spaces kw_long spaces kw_int
      | kw_int spaces kw_signed spaces kw_long spaces kw_long
      | kw_int spaces kw_long spaces kw_signed spaces kw_long
      | kw_int spaces kw_long spaces kw_long spaces kw_signed
      | kw_long spaces kw_signed spaces kw_int spaces kw_long
      | kw_long spaces kw_signed spaces kw_long spaces kw_int
      | kw_long spaces kw_int spaces kw_signed spaces kw_long
      | kw_long spaces kw_int spaces kw_long spaces kw_signed
      | kw_long spaces kw_long spaces kw_signed spaces kw_int
      | kw_long spaces kw_long spaces kw_int spaces kw_signed;
   c_unsigned_long_long =
      kw_unsigned spaces kw_long spaces kw_long
      | kw_long spaces kw_unsigned spaces kw_long
      | kw_long spaces kw_long spaces kw_unsigned
      | kw_unsigned spaces kw_int spaces kw_long spaces kw_long
      | kw_unsigned spaces kw_long spaces kw_int spaces kw_long
      | kw_unsigned spaces kw_long spaces kw_long spaces kw_int
      | kw_int spaces kw_unsigned spaces kw_long spaces kw_long
      | kw_int spaces kw_long spaces kw_unsigned spaces kw_long
      | kw_int spaces kw_long spaces kw_long spaces kw_unsigned
      | kw_long spaces kw_unsigned spaces kw_int spaces kw_long
      | kw_long spaces kw_unsigned spaces kw_long spaces kw_int
      | kw_long spaces kw_int spaces kw_unsigned spaces kw_long
      | kw_long spaces kw_int spaces kw_long spaces kw_unsigned
      | kw_long spaces kw_long spaces kw_unsigned spaces kw_int
      | kw_long spaces kw_long spaces kw_int spaces kw_unsigned;

   *                                { string_mem[0] = l->start[0]; l->cursor = cursor; return CM1_TOKEN_END; }
   string_literal                   { l->cursor = cursor; return CM1_TOKEN_STRING; }
   character_literal                { l->cursor = cursor; return CM1_TOKEN_I32; }
   "const"                          { l->cursor = cursor; return CM1_TOKEN_OR_ASSIGN + 3; }
   "static"                         { l->cursor = cursor; return CM1_TOKEN_OR_ASSIGN + 4; }
   "inline"                         { l->cursor = cursor; return CM1_TOKEN_OR_ASSIGN + 5; }
   "# " [^\000\n]+                  {
      l->cursor = cursor; return CM1_TOKEN_OR_ASSIGN + 2;
   }
   "#" [^ ] [^\000\n]+              { l->cursor = cursor; return CM1_TOKEN_PREPROCESSOR; }
   spaces                           { l->cursor = cursor; return CM1_TOKEN_OR_ASSIGN + 1; }
   ";"                              {
      if (curly_brace_depth == 0) {
         // A top-level semicolon ends a function declaration without a body.
         curly_brace_mode = CURLY_BRACE_MODE_NONE;
      }
      l->cursor = cursor; return CM1_TOKEN_SEMICOLON;
   }
   "*"                              { l->cursor = cursor; return CM1_TOKEN_STAR; }
   "return" / [^a-zA-Z0-9_]        { l->cursor = cursor; return CM1_TOKEN_RETURN; }
   "typedef" / [^a-zA-Z0-9_]       { l->cursor = cursor; return CM1_TOKEN_TYPEDEF; }
   "extern" / [^a-zA-Z0-9_]        { l->cursor = cursor; return CM1_TOKEN_EXTERN; }
   "switch" / [^a-zA-Z0-9_]        { l->cursor = cursor; return CM1_TOKEN_SWITCH; }
   "case" / [^a-zA-Z0-9_]          { l->cursor = cursor; return CM1_TOKEN_CASE; }
   "default" / [^a-zA-Z0-9_]       { l->cursor = cursor; return CM1_TOKEN_DEFAULT; }
   "break" / [^a-zA-Z0-9_]         { l->cursor = cursor; return CM1_TOKEN_BREAK; }
   "continue" / [^a-zA-Z0-9_]      { l->cursor = cursor; return CM1_TOKEN_CONTINUE; }
   "goto" / [^a-zA-Z0-9_]          { l->cursor = cursor; return CM1_TOKEN_GOTO; }
   "do" / [^a-zA-Z0-9_]            { l->cursor = cursor; return CM1_TOKEN_DO; }
   "while" / [^a-zA-Z0-9_]         { l->cursor = cursor; return CM1_TOKEN_WHILE; }
   "for" / [^a-zA-Z0-9_]           { l->cursor = cursor; return CM1_TOKEN_FOR; }
   "sizeof" "("                     { l->cursor = cursor - 1; return CM1_TOKEN_SIZEOF; }
   "sizeof" spaces "("              { l->cursor += 6; return CM1_TOKEN_SIZEOF; }
   "sizeof" / [^a-zA-Z0-9_]        { l->cursor = cursor; return CM1_TOKEN_SIZEOF; }
   "alignof" "("                    { l->cursor += 7; return CM1_TOKEN_ALIGNOF; }
   "alignof" / [^a-zA-Z0-9_]       { l->cursor = cursor; return CM1_TOKEN_ALIGNOF; }
   "__alignof__" spaces "("         { l->cursor += 11; return CM1_TOKEN_ALIGNOF; }
   "__alignof__" / [^a-zA-Z0-9_]   { l->cursor = cursor; return CM1_TOKEN_ALIGNOF; }
   "_Alignof" spaces "("            { l->cursor += 8; return CM1_TOKEN_ALIGNOF; }
   "_Alignof" / [^a-zA-Z0-9_]      { l->cursor = cursor; return CM1_TOKEN_ALIGNOF; }
   "if" / [^a-zA-Z0-9_]            { l->cursor = cursor; return CM1_TOKEN_IF; }
   "else" / [^a-zA-Z0-9_]          { l->cursor = cursor; return CM1_TOKEN_ELSE; }
   c_signed_int / [^a-zA-Z0-9_]    { l->cursor = cursor; return CM1_C_TOKEN_INT; }
   c_unsigned_int / [^a-zA-Z0-9_]  { l->cursor = cursor; return CM1_C_TOKEN_UINT; }
   c_signed_short / [^a-zA-Z0-9_]  { l->cursor = cursor; return CM1_C_TOKEN_SHORT; }
   c_unsigned_short / [^a-zA-Z0-9_] { l->cursor = cursor; return CM1_C_TOKEN_USHORT; }
   "char" / [^a-zA-Z0-9_]          { l->cursor = cursor; return CM1_C_TOKEN_CHAR; }
   (kw_signed spaces "char" | "char" spaces kw_signed) / [^a-zA-Z0-9_]
                                    { l->cursor = cursor; return CM1_C_TOKEN_SCHAR; }
   (kw_unsigned spaces "char" | "char" spaces kw_unsigned) / [^a-zA-Z0-9_]
                                    { l->cursor = cursor; return CM1_C_TOKEN_UCHAR; }
   c_signed_long_long / [^a-zA-Z0-9_] { l->cursor = cursor; return CM1_C_TOKEN_LLONG; }
   c_unsigned_long_long / [^a-zA-Z0-9_] { l->cursor = cursor; return CM1_C_TOKEN_ULLONG; }
   c_signed_long / [^a-zA-Z0-9_]   { l->cursor = cursor; return CM1_C_TOKEN_LONG; }
   c_unsigned_long / [^a-zA-Z0-9_] { l->cursor = cursor; return CM1_C_TOKEN_ULONG; }
   "int64_t" / [^a-zA-Z0-9_]       { l->cursor = cursor; return CM1_TOKEN_INT64; }
   "int32_t" / [^a-zA-Z0-9_]       { l->cursor = cursor; return CM1_TOKEN_INT32; }
   "int16_t" / [^a-zA-Z0-9_]       { l->cursor = cursor; return CM1_TOKEN_INT16; }
   "int8_t" / [^a-zA-Z0-9_]        { l->cursor = cursor; return CM1_TOKEN_INT8; }
   "uint64_t" / [^a-zA-Z0-9_]      { l->cursor = cursor; return CM1_TOKEN_UINT64; }
   "uint32_t" / [^a-zA-Z0-9_]      { l->cursor = cursor; return CM1_TOKEN_UINT32; }
   "uint16_t" / [^a-zA-Z0-9_]      { l->cursor = cursor; return CM1_TOKEN_UINT16; }
   "uint8_t" / [^a-zA-Z0-9_]       { l->cursor = cursor; return CM1_TOKEN_UINT8; }
   "size_t" / [^a-zA-Z0-9_]        { l->cursor = cursor; return CM1_C_TOKEN_SIZE_T; }
   "ssize_t" / [^a-zA-Z0-9_]       { l->cursor = cursor; return CM1_C_TOKEN_SSIZE_T; }
   "float" / [^a-zA-Z0-9_]         { l->cursor = cursor; return CM1_TOKEN_F32; }
   "double" / [^a-zA-Z0-9_]        { l->cursor = cursor; return CM1_TOKEN_F64; }
   "void" / [^a-zA-Z0-9_]          { l->cursor = cursor; return CM1_TOKEN_VOID; }
   "struct" / [^a-zA-Z0-9_]        { l->cursor = cursor; return CM1_TOKEN_STRUCT; }
   "struct" spaces [a-zA-Z_][a-zA-Z0-9_]* { l->cursor = cursor; return CM1_TOKEN_STRUCT_SPACE_ID; }
   "union" / [^a-zA-Z0-9_]         { l->cursor = cursor; return CM1_TOKEN_UNION; }
   "union" spaces [a-zA-Z_][a-zA-Z0-9_]* { l->cursor = cursor; return CM1_TOKEN_UNION_SPACE_ID; }
   "enum" / [^a-zA-Z0-9_]          { l->cursor = cursor; return CM1_TOKEN_ENUM; }
   "enum" spaces [a-zA-Z_][a-zA-Z0-9_]* { l->cursor = cursor; return CM1_TOKEN_ENUM_SPACE_ID; }
   [a-zA-Z_][a-zA-Z0-9_]*           { l->cursor = cursor; return CM1_TOKEN_ID; }
   [a-zA-Z_][a-zA-Z0-9_]* "("       {
      if (curly_brace_depth == 0) {
         if (_Tcm1_Ffunc_is_bytecode_2(l->start, cursor - l->start - 1)) {
            curly_brace_mode = CURLY_BRACE_MODE_CM1_FUNC;
            l->cursor = cursor; return CM1_TOKEN_FUNC_BC;
         } else {
            curly_brace_mode = CURLY_BRACE_MODE_C_FUNC;
            l->cursor = cursor; return CM1_TOKEN_FUNC_C;
         }
      }
      l->cursor = cursor; return CM1_TOKEN_FUNC;
   }
   "("                              { l->cursor = cursor; return CM1_TOKEN_LPAREN; }
   ")"                              { l->cursor = cursor; return CM1_TOKEN_RPAREN; }
   ")" spaces "{"                   { l->cursor++; return CM1_TOKEN_RPAREN_THEN_CURLY; }
   "["                              { l->cursor = cursor; return CM1_TOKEN_LBRACKET; }
   "]"                              { l->cursor = cursor; return CM1_TOKEN_RBRACKET; }
   ","                              { l->cursor = cursor; return CM1_TOKEN_COMMA; }
   "?"                              { l->cursor = cursor; return CM1_TOKEN_QUESTION; }
   ":"                              { l->cursor = cursor; return CM1_TOKEN_COLON; }
   "."                              { l->cursor = cursor; return CM1_TOKEN_DOT; }
   "->"                             { l->cursor = cursor; return CM1_TOKEN_ARROW; }
   "++"                             { l->cursor = cursor; return CM1_TOKEN_INC; }
   "--"                             { l->cursor = cursor; return CM1_TOKEN_DEC; }
   "+"                              { l->cursor = cursor; return CM1_TOKEN_PLUS; }
   "-"                              { l->cursor = cursor; return CM1_TOKEN_MINUS; }
   "/"                              { l->cursor = cursor; return CM1_TOKEN_SLASH; }
   "&&"                             { l->cursor = cursor; return CM1_TOKEN_LAND; }
   "||"                             { l->cursor = cursor; return CM1_TOKEN_LOR; }
   "&"                              { l->cursor = cursor; return CM1_TOKEN_AMP; }
   "^"                              { l->cursor = cursor; return CM1_TOKEN_CARET; }
   "|"                              { l->cursor = cursor; return CM1_TOKEN_PIPE; }
   "%"                              { l->cursor = cursor; return CM1_TOKEN_PERCENT; }
   "<<"                             { l->cursor = cursor; return CM1_TOKEN_SHL; }
   ">>"                             { l->cursor = cursor; return CM1_TOKEN_SHR; }
   "~"                              { l->cursor = cursor; return CM1_TOKEN_TILDE; }
   "=="                             { l->cursor = cursor; return CM1_TOKEN_EQ; }
   "!="                             { l->cursor = cursor; return CM1_TOKEN_NE; }
   "<"                              { l->cursor = cursor; return CM1_TOKEN_LT; }
   "<="                             { l->cursor = cursor; return CM1_TOKEN_LE; }
   ">"                              { l->cursor = cursor; return CM1_TOKEN_GT; }
   ">="                             { l->cursor = cursor; return CM1_TOKEN_GE; }
   "!"                              { l->cursor = cursor; return CM1_TOKEN_BANG; }
   "="                              { l->cursor = cursor; return CM1_TOKEN_ASSIGN; }
   "+="                             { l->cursor = cursor; return CM1_TOKEN_ADD_ASSIGN; }
   "-="                             { l->cursor = cursor; return CM1_TOKEN_SUB_ASSIGN; }
   "*="                             { l->cursor = cursor; return CM1_TOKEN_MUL_ASSIGN; }
   "/="                             { l->cursor = cursor; return CM1_TOKEN_DIV_ASSIGN; }
   "%="                             { l->cursor = cursor; return CM1_TOKEN_MOD_ASSIGN; }
   "<<="                            { l->cursor = cursor; return CM1_TOKEN_SHL_ASSIGN; }
   ">>="                            { l->cursor = cursor; return CM1_TOKEN_SHR_ASSIGN; }
   "&="                             { l->cursor = cursor; return CM1_TOKEN_AND_ASSIGN; }
   "^="                             { l->cursor = cursor; return CM1_TOKEN_XOR_ASSIGN; }
   "|="                             { l->cursor = cursor; return CM1_TOKEN_OR_ASSIGN; }
   f32_num                          { l->cursor = cursor; return CM1_TOKEN_F32_NUM; }
   f64_num                          { l->cursor = cursor; return CM1_TOKEN_F64_NUM; }
   "0"                              { l->cursor = cursor; return CM1_TOKEN_ZERO; }
   hex_integer                      { l->cursor = cursor; return CM1_TOKEN_I32; }
   octal_integer                    { l->cursor = cursor; return CM1_TOKEN_I32; }
   [1-9][0-9]* "u"?                      { l->cursor = cursor; return CM1_TOKEN_I32; }
   "{"                              {
      if (curly_brace_depth == 0) {
         if (curly_brace_mode == CURLY_BRACE_MODE_C_FUNC) {
            curly_brace_mode = CURLY_BRACE_MODE_NONE;
            const char* c = cm1_c_code_end(cursor);
            cursor = c + 1;
            l->cursor = cursor; return CM1_TOKEN_C_CODE;
         }
         curly_brace_mode = CURLY_BRACE_MODE_NONE;
      }
      curly_brace_depth++;
      l->cursor = cursor; return CM1_TOKEN_OPEN_CURLY_BRACE;
   }
   "}"                              {
      curly_brace_depth--;
      l->cursor = cursor; return CM1_TOKEN_CLOSE_CURLY_BRACE;
   }
   */
}
