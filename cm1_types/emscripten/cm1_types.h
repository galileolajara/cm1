#ifndef CM1_TYPES_EMSCRIPTEN_CM1_TYPES_H
#define CM1_TYPES_EMSCRIPTEN_CM1_TYPES_H

/*
 * Emscripten's wasm32 C data model:
 *   char       8-bit signed
 *   short      16-bit
 *   int        32-bit
 *   long       32-bit
 *   long long  64-bit
 *   size_t     32-bit unsigned
 *   ssize_t    32-bit signed
 *
 * These definitions describe the bytecode target, not the machine on which
 * the Cm1 cross compiler itself is running.
 */
#define CM1_C_TOKEN_INT           CM1_TOKEN_INT32
#define CM1_C_TOKEN_UINT          CM1_TOKEN_UINT32
#define CM1_C_TOKEN_SHORT         CM1_TOKEN_INT16
#define CM1_C_TOKEN_USHORT        CM1_TOKEN_UINT16
#define CM1_C_TOKEN_LONG          CM1_TOKEN_INT32
#define CM1_C_TOKEN_ULONG         CM1_TOKEN_UINT32
#define CM1_C_TOKEN_LLONG         CM1_TOKEN_INT64
#define CM1_C_TOKEN_ULLONG        CM1_TOKEN_UINT64
#define CM1_C_TOKEN_SCHAR         CM1_TOKEN_INT8
#define CM1_C_TOKEN_UCHAR         CM1_TOKEN_UINT8
#define CM1_C_TOKEN_SIZE_T        CM1_TOKEN_UINT32
#define CM1_C_TOKEN_SSIZE_T       CM1_TOKEN_INT32

/* Literal suffixes follow the same target widths. */
#define CM1_C_TOKEN_LONG_LITERAL  CM1_TOKEN_I32
#define CM1_C_TOKEN_ULONG_LITERAL CM1_TOKEN_U32
#define CM1_C_TOKEN_LLONG_LITERAL CM1_TOKEN_I64
#define CM1_C_TOKEN_ULLONG_LITERAL CM1_TOKEN_U64

#define CM1_C_CHAR_IS_SIGNED 1
#define CM1_C_POINTER_SIZE 4u

#endif
