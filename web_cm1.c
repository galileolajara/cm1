// c4ddf5e5
// #define CM1_DEBUG_STACK
// #define CM1_DEBUG_STACK_ALLOC

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>

#define cm1_pop_i64() cm1_stack_v[--cm1_stack_pos].i64
#define cm1_pop_u64() cm1_stack_v[--cm1_stack_pos].u64
#define cm1_pop_u32() (uint32_t)cm1_stack_v[--cm1_stack_pos].u64
#define cm1_pop_isz() (ssize_t)cm1_stack_v[--cm1_stack_pos].i64
#define cm1_pop_usz() (size_t)cm1_stack_v[--cm1_stack_pos].u64
#define cm1_pop_i32() (int32_t)cm1_stack_v[--cm1_stack_pos].i64
#define cm1_pop_f32() cm1_stack_v[--cm1_stack_pos].f32
#define cm1_pop_f64() cm1_stack_v[--cm1_stack_pos].f64
#define cm1_pop_ptr() cm1_stack_v[--cm1_stack_pos].ptr
// Use i64 when pushing so that upper bits are zeroed
#define cm1_push_i(val) cm1_stack_v[cm1_stack_pos++].i64 = val
#define cm1_push_u(val) cm1_stack_v[cm1_stack_pos++].u64 = val

#define cm1_push_ptr(val) cm1_stack_v[cm1_stack_pos++].ptr = val

union cm1_stack_item {
   uint64_t u64;
   int64_t i64;
   float f32;
   double f64;
   void* ptr;
};

#ifndef CM1_STACK_LIMIT
#define CM1_STACK_LIMIT (1024 * 1024)
#endif

static union cm1_stack_item cm1_stack_v[CM1_STACK_LIMIT];
static uint32_t cm1_stack_pos;

void cm1_init(const char* cm1_path);
void cm1_run(uint16_t func_idx);
int32_t cm1_printf_begin() {
   return 0;
}
int32_t cm1_printf_s(int32_t n, int8_t *fmt, int8_t *s) {
   return n + printf(fmt, s);
}
int32_t cm1_printf_x(int32_t n, int8_t *fmt, int32_t x) {
   return n + printf(fmt, x);
}
int32_t cm1_printf_c(int32_t n, int8_t *fmt, int32_t c) {
   return n + printf(fmt, c);
}
int32_t cm1_printf_d(int32_t n, int8_t *fmt, int32_t val) {
   return n + printf(fmt, val);
}
int32_t cm1_printf_f(int32_t n, int8_t *fmt, double val) {
   return n + printf(fmt, val);
}
int32_t cm1_printf_end(int32_t n, int8_t *str) {
   return n + printf("%s", str);
}
#ifndef run
void run();
#endif
void run() {
   cm1_run(0);
   (void)cm1_pop_ptr();
}
void* cm1_gvar(uint16_t var_idx) {
   switch (var_idx) {
   }
   return NULL;
}
void cm1_run_c(uint16_t func_idx) {
   switch (func_idx) {
      case 0: {
         int32_t ret = cm1_printf_begin();
         cm1_push_i(ret);
         break;
      }
      case 1: {
         void* arg2 = cm1_pop_ptr();
         void* arg1 = cm1_pop_ptr();
         int32_t arg0 = cm1_pop_i64();
         int32_t ret = cm1_printf_s(arg0, arg1, arg2);
         cm1_push_i(ret);
         break;
      }
      case 2: {
         int32_t arg2 = cm1_pop_i64();
         void* arg1 = cm1_pop_ptr();
         int32_t arg0 = cm1_pop_i64();
         int32_t ret = cm1_printf_x(arg0, arg1, arg2);
         cm1_push_i(ret);
         break;
      }
      case 3: {
         int32_t arg2 = cm1_pop_i64();
         void* arg1 = cm1_pop_ptr();
         int32_t arg0 = cm1_pop_i64();
         int32_t ret = cm1_printf_c(arg0, arg1, arg2);
         cm1_push_i(ret);
         break;
      }
      case 4: {
         int32_t arg2 = cm1_pop_i64();
         void* arg1 = cm1_pop_ptr();
         int32_t arg0 = cm1_pop_i64();
         int32_t ret = cm1_printf_d(arg0, arg1, arg2);
         cm1_push_i(ret);
         break;
      }
      case 5: {
         double arg2 = cm1_pop_f64();
         void* arg1 = cm1_pop_ptr();
         int32_t arg0 = cm1_pop_i64();
         int32_t ret = cm1_printf_f(arg0, arg1, arg2);
         cm1_push_i(ret);
         break;
      }
      case 6: {
         void* arg1 = cm1_pop_ptr();
         int32_t arg0 = cm1_pop_i64();
         int32_t ret = cm1_printf_end(arg0, arg1);
         cm1_push_i(ret);
         break;
      }
   }
}
#define CM1_OP_RETURN              0
#define CM1_OP_POP_RETURN          1
#define CM1_OP_CALL_CM1            2
#define CM1_OP_CALL_C              3
#define CM1_OP_POP                 4
#define CM1_OP_PUSH_ZERO           5
#define CM1_OP_PUSH_I32            6
#define CM1_OP_PUSH_U8             7
#define CM1_OP_PUSH_U16            8
#define CM1_OP_PUSH_U32            9
#define CM1_OP_PUSH_U64            10
#define CM1_OP_COPY_MEM            11
#define CM1_OP_ZERO_MEM            12
#define CM1_OP_GVAR_PUSH           13
#define CM1_OP_LVAR_PUSH           14
#define CM1_OP_LVAR_SET            15
#define CM1_OP_ADDR_ADD            16
#define CM1_OP_ADDR_SUB            17
#define CM1_OP_JUMP_IF_ZERO        18
#define CM1_OP_JUMP                19
#define CM1_OP_DUP_PAIR            20
#define CM1_OP_INEG                21
#define CM1_OP_BIT_NOT             22
#define CM1_OP_LOGICAL_NOT         23
#define CM1_OP_PUSH_MEM_PTR        24
#define CM1_OP_SET_MEM_PTR         25
#define CM1_OP_INT_CONVERT         26
#define CM1_OP_INT_BINARY          27
#define CM1_OP_PUSH_MEM_INT        28
#define CM1_OP_SET_MEM_INT         29
#define CM1_OP_LVAR_INC_DEC_INT    30
#define CM1_OP_MEM_INC_DEC_INT     31
#define CM1_OP_PUSH_F32            32
#define CM1_OP_PUSH_F64            33
#define CM1_OP_NUM_CONVERT         34
#define CM1_OP_FLOAT_BINARY        35
#define CM1_OP_PUSH_MEM_FLOAT      36
#define CM1_OP_SET_MEM_FLOAT       37
#define CM1_OP_FNEG                38
#define CM1_OP_FLOAT_TO_BOOL       39
#define CM1_OP_LVAR_INC_DEC_FLOAT  40
#define CM1_OP_MEM_INC_DEC_FLOAT   41
#define CM1_OP_PUSH_STRING         42
#define CM1_OP_LVAR_SPILL          43
#define CM1_OP_MEM_INC_DEC_PTR     44

#define CM1_TYPE_I8  1
#define CM1_TYPE_U8  2
#define CM1_TYPE_I16 3
#define CM1_TYPE_U16 4
#define CM1_TYPE_I32 5
#define CM1_TYPE_U32 6
#define CM1_TYPE_I64 7
#define CM1_TYPE_U64 8
#define CM1_TYPE_F32 9
#define CM1_TYPE_F64 10

static uint8_t* cm1_bytecode;
static uint32_t cm1_lvar_pos;

static void cm1_push_f32(float value) {
   cm1_stack_v[cm1_stack_pos].u64 = 0;
   cm1_stack_v[cm1_stack_pos++].f32 = value;
}

static void cm1_push_f64(double value) {
   cm1_stack_v[cm1_stack_pos++].f64 = value;
}

typedef uint16_t cm1_unaligned_u16 __attribute__((aligned(1), may_alias));
typedef uint32_t cm1_unaligned_u32 __attribute__((aligned(1), may_alias));
typedef uint64_t cm1_unaligned_u64 __attribute__((aligned(1), may_alias));
typedef float cm1_unaligned_f32 __attribute__((aligned(1), may_alias));
typedef double cm1_unaligned_f64 __attribute__((aligned(1), may_alias));

static bool cm1_int_is_valid(uint8_t kind) {
   return kind >= CM1_TYPE_I8 && kind <= CM1_TYPE_U64;
}

static bool cm1_int_is_signed(uint8_t kind) {
   return (kind & 1) != 0;
}

static unsigned cm1_int_rank(uint8_t kind) {
   if (!cm1_int_is_valid(kind)) {
      printf("invalid integer type: %u\n", kind);
      exit(EXIT_FAILURE);
   }
   return (kind - CM1_TYPE_I8) >> 1;
}

static size_t cm1_int_size(uint8_t kind) {
   return (size_t)1 << cm1_int_rank(kind);
}

static unsigned cm1_int_width(uint8_t kind) {
   // The compiler promotes 8- and 16-bit operands before arithmetic.
   return kind < CM1_TYPE_I64 ? 32 : 64;
}

static uint64_t cm1_int_convert(uint64_t value, uint8_t kind) {
   unsigned width = (unsigned)cm1_int_size(kind) * 8;
   if (width == 64) return value;
   uint64_t mask = (UINT64_C(1) << width) - 1;
   value &= mask;
   if (!cm1_int_is_signed(kind)) return value;
   uint64_t sign = UINT64_C(1) << (width - 1);
   return (value ^ sign) - sign;
}

static uint64_t cm1_load_int(const void* ptr, uint8_t kind) {
   uint64_t value;
   switch (cm1_int_rank(kind)) {
      case 0: value = *(const uint8_t*)ptr; break;
      case 1: value = *(const cm1_unaligned_u16*)ptr; break;
      case 2: value = *(const cm1_unaligned_u32*)ptr; break;
      case 3: value = *(const cm1_unaligned_u64*)ptr; break;
      default: abort();
   }
   return cm1_int_convert(value, kind);
}

static uint64_t cm1_store_int(void* ptr, uint64_t value, uint8_t kind) {
   value = cm1_int_convert(value, kind);
   switch (cm1_int_rank(kind)) {
      case 0: *(uint8_t*)ptr = value; break;
      case 1: *(cm1_unaligned_u16*)ptr = value; break;
      case 2: *(cm1_unaligned_u32*)ptr = value; break;
      case 3: *(cm1_unaligned_u64*)ptr = value; break;
      default: abort();
   }
   return value;
}

static bool cm1_float_is_valid(uint8_t kind) {
   return kind == CM1_TYPE_F32 || kind == CM1_TYPE_F64;
}

static bool cm1_number_is_valid(uint8_t kind) {
   return cm1_int_is_valid(kind) || cm1_float_is_valid(kind);
}

static float cm1_number_as_f32(union cm1_stack_item value, uint8_t kind) {
   if (kind == CM1_TYPE_F32) return value.f32;
   if (kind == CM1_TYPE_F64) return (float)value.f64;
   if (cm1_int_is_signed(kind)) return (float)value.i64;
   return (float)value.u64;
}

static double cm1_number_as_f64(union cm1_stack_item value, uint8_t kind) {
   if (kind == CM1_TYPE_F32) return (double)value.f32;
   if (kind == CM1_TYPE_F64) return value.f64;
   if (cm1_int_is_signed(kind)) return (double)value.i64;
   return (double)value.u64;
}

static union cm1_stack_item cm1_number_convert(
   union cm1_stack_item value, uint8_t src_kind, uint8_t des_kind
) {
   union cm1_stack_item result;
   result.u64 = 0;
   if (!cm1_number_is_valid(src_kind) || !cm1_number_is_valid(des_kind)) {
      printf("invalid numeric conversion: %u to %u\n", src_kind, des_kind);
      exit(EXIT_FAILURE);
   }
   if (des_kind == CM1_TYPE_F32) {
      result.f32 = cm1_number_as_f32(value, src_kind);
   } else if (des_kind == CM1_TYPE_F64) {
      result.f64 = cm1_number_as_f64(value, src_kind);
   } else if (cm1_float_is_valid(src_kind)) {
      if (cm1_int_is_signed(des_kind)) {
         int64_t integer = src_kind == CM1_TYPE_F32
            ? (int64_t)value.f32 : (int64_t)value.f64;
         result.u64 = cm1_int_convert((uint64_t)integer, des_kind);
      } else {
         uint64_t integer = src_kind == CM1_TYPE_F32
            ? (uint64_t)value.f32 : (uint64_t)value.f64;
         result.u64 = cm1_int_convert(integer, des_kind);
      }
   } else {
      result.u64 = cm1_int_convert(value.u64, des_kind);
   }
   return result;
}

void cm1_init(const char* cm1_path) {
   #ifdef _WIN32
   int fd = open(cm1_path, O_RDONLY | O_BINARY);
   #else
   int fd = open(cm1_path, O_RDONLY);
   #endif
   size_t size = lseek(fd, 0, SEEK_END) - 4; // Minus 4 for the crc32c
   // printf("loading cm1 bytecode: %s (%zu bytes)\n", cm1_path, size);
   lseek(fd, 4, SEEK_SET); // Skip the crc32c
   void* data = malloc(size);
   read(fd, data, size);
   close(fd);
   cm1_bytecode = (uint8_t*)data;
   #ifndef CM1_NO_STACK_RAND
   uint64_t splitmix64_state = 0;
   for (uint32_t i = 0; i < CM1_STACK_LIMIT; i++) {
      uint64_t z = (splitmix64_state += UINT64_C(0x9E3779B97F4A7C15));
      z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
      z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
      cm1_stack_v[i].i64 = z ^ (z >> 31);
   }
   #endif
}

void cm1_run(uint16_t func_idx) {
   uint32_t old_lvar_pos = cm1_lvar_pos;

   uint32_t* offset_v = (uint32_t*)cm1_bytecode;
   // printf("cm1_run function #%u, offset @ %u\n", func_idx, offset_v[func_idx]);
   uint8_t* bc = cm1_bytecode + offset_v[func_idx];
   uint8_t arg_c = *(bc++);
   cm1_lvar_pos = cm1_stack_pos - arg_c;
   uint32_t old_stack_pos = cm1_lvar_pos; // Replace arguments with the return value when the call finishes
   // printf("lvar_pos = %u, cm1_stack_pos = %u, arg_c = %u\n", cm1_lvar_pos, cm1_stack_pos, arg_c);
   uint16_t lvar_c = *((uint16_t*)bc); // Allocate stack for local variables
   bc += sizeof(uint16_t);
   cm1_stack_pos += lvar_c + 1; // +1 to leave a slot for POP_RETURN
   // Do necessary copying of arguments passed as aggregates
   // to avoid modifying the aggregate's memory that was passed by the caller
   for (;;) {
      uint8_t lvar_idx = *(bc++);
      if (lvar_idx == 0xff) break;
      uint64_t* old_ptr = (uint64_t*)cm1_stack_v[cm1_lvar_pos + lvar_idx].ptr;
      uint64_t* new_ptr = (uint64_t*)&cm1_stack_v[cm1_stack_pos];
      cm1_stack_v[cm1_lvar_pos + lvar_idx].ptr = new_ptr;
      uint16_t size = *((uint16_t*)bc);
      bc += sizeof(uint16_t);
      for (uint16_t i = 0; i < size; i++) {
         new_ptr[i] = old_ptr[i];
      }
      #ifdef CM1_DEBUG_STACK_ALLOC
      printf("Allocated %u bytes for local variable at index %u\n", size << 3, lvar_idx);
      #endif
      cm1_stack_pos += size;
   }
   // Allocate stack memory for aggregates and arrays
   for (;;) {
      uint16_t lvar_idx = *((uint16_t*)bc);
      bc += sizeof(uint16_t);
      if (lvar_idx == 0xffff) break;
      cm1_stack_v[cm1_lvar_pos + lvar_idx].ptr = &cm1_stack_v[cm1_stack_pos];
      uint16_t size = *((uint16_t*)bc);
      bc += sizeof(uint16_t);
      #ifdef CM1_DEBUG_STACK_ALLOC
      printf("Allocated %u bytes for local variable at index %u\n", size << 3, lvar_idx);
      #endif
      cm1_stack_pos += size;
   }
   for (;;) {
      uint8_t op = *(bc++);
      #ifdef CM1_DEBUG_STACK
      printf("op %u, stack:", op);
      for (uint32_t i = 0; i < cm1_stack_pos; i++) {
         printf(" %" PRIx64, cm1_stack_v[i].i64);
      }
      printf("\n");
      #endif
      switch (op) {
         case CM1_OP_RETURN: {
            cm1_lvar_pos = old_lvar_pos;
            cm1_stack_pos = old_stack_pos;
            cm1_push_ptr(NULL);
            return;
         }
         case CM1_OP_POP_RETURN: {
            union cm1_stack_item value = cm1_stack_v[--cm1_stack_pos];
            cm1_lvar_pos = old_lvar_pos;
            cm1_stack_pos = old_stack_pos;
            cm1_stack_v[cm1_stack_pos++] = value;
            return;
         }
         case CM1_OP_CALL_CM1: {
            uint16_t func_idx2 = *((uint16_t*)bc);
            bc += sizeof(uint16_t);
            cm1_run(func_idx2);
            break;
         }
         case CM1_OP_CALL_C: {
            uint16_t func_idx2 = *((uint16_t*)bc);
            bc += sizeof(uint16_t);
            // printf("running a C function, func-idx #%u\n", func_idx2);
            cm1_run_c(func_idx2);
            break;
         }
         case CM1_OP_PUSH_ZERO: {
            cm1_push_i(0);
            break;
         }
         case CM1_OP_PUSH_I32: {
            int32_t val = *((int32_t*)bc);
            bc += sizeof(int32_t);
            // printf("pushing i32 to stack: %d\n", val);
            cm1_push_i(val);
            break;
         }
         case CM1_OP_PUSH_U8: {
            uint8_t val = *((uint8_t*)bc);
            bc += sizeof(uint8_t);
            // printf("pushing i32 to stack: %d\n", val);
            cm1_push_u(val);
            break;
         }
         case CM1_OP_PUSH_U16: {
            uint16_t val = *((uint16_t*)bc);
            bc += sizeof(uint16_t);
            // printf("pushing i32 to stack: %d\n", val);
            cm1_push_u(val);
            break;
         }
         case CM1_OP_PUSH_U32: {
            uint32_t val = *((uint32_t*)bc);
            bc += sizeof(uint32_t);
            // printf("pushing i32 to stack: %d\n", val);
            cm1_push_u(val);
            break;
         }
         case CM1_OP_PUSH_U64: {
            uint64_t val = *((uint64_t*)bc);
            bc += sizeof(uint64_t);
            // printf("pushing i32 to stack: %d\n", val);
            cm1_push_u(val);
            break;
         }
         case CM1_OP_PUSH_STRING: {
            uint32_t offset = *(const cm1_unaligned_u32*)bc;
            bc += sizeof(uint32_t);
            cm1_push_ptr(cm1_bytecode + offset);
            break;
         }
         case CM1_OP_COPY_MEM: {
            size_t size = cm1_pop_usz();
            uint8_t* src = cm1_pop_ptr();
            uint8_t* des = cm1_pop_ptr();
            memcpy(des, src, size);
            cm1_push_ptr(des);
            break;
         }
         case CM1_OP_ZERO_MEM: {
            size_t size = cm1_pop_usz();
            uint8_t* des = cm1_pop_ptr();
            memset(des, 0, size);
            cm1_push_ptr(des);
            break;
         }
         case CM1_OP_GVAR_PUSH: {
            uint32_t var_idx = *((uint32_t*)bc);
            bc += sizeof(uint32_t);
            cm1_push_ptr(cm1_gvar(var_idx));
            break;
         }
         case CM1_OP_LVAR_PUSH: {
            uint16_t var_idx = *((uint16_t*)bc);
            bc += sizeof(uint16_t);
            cm1_stack_v[cm1_stack_pos++] =
               cm1_stack_v[cm1_lvar_pos + var_idx];
            break;
         }
         case CM1_OP_LVAR_SPILL: {
            uint16_t var_idx = *((uint16_t*)bc);
            bc += sizeof(uint16_t);
            uint8_t size_mode = *(bc++);
            size_t size = size_mode & 0x7f;
            union cm1_stack_item old_value =
               cm1_stack_v[cm1_lvar_pos + var_idx];
            uint8_t* storage = (uint8_t*)&cm1_stack_v[cm1_stack_pos++];
            memset(storage, 0, sizeof(union cm1_stack_item));
            if (size_mode & 0x80) memcpy(storage, &old_value, size);
            cm1_stack_v[cm1_lvar_pos + var_idx].ptr = storage;
            break;
         }
         case CM1_OP_LVAR_SET: {
            uint16_t var_idx = *((uint16_t*)bc);
            bc += sizeof(uint16_t);
            union cm1_stack_item value = cm1_stack_v[--cm1_stack_pos];
            cm1_stack_v[cm1_lvar_pos + var_idx] = value;
            cm1_stack_v[cm1_stack_pos++] = value;
            break;
         }
         case CM1_OP_PUSH_MEM_PTR: {
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            cm1_push_ptr(*(void**)(addr + offset));
            break;
         }
         case CM1_OP_SET_MEM_PTR: {
            void* val = cm1_pop_ptr();
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            *(void**)(addr + offset) = val;
            cm1_push_ptr(val);
            break;
         }
         case CM1_OP_INT_CONVERT: {
            uint8_t kind = *(bc++);
            cm1_stack_v[cm1_stack_pos - 1].u64 =
               cm1_int_convert(cm1_stack_v[cm1_stack_pos - 1].u64, kind);
            break;
         }
         case CM1_OP_INT_BINARY: {
            uint8_t op_kind = *(bc++);
            uint8_t int_op = op_kind >> 4;
            uint8_t kind = op_kind & 15;
            uint64_t op2 = cm1_int_convert(cm1_pop_u64(), kind);
            uint64_t op1 = cm1_int_convert(cm1_pop_u64(), kind);
            uint64_t result = 0;
            bool comparison = false;
            unsigned width = cm1_int_width(kind);
            bool is_signed = cm1_int_is_signed(kind);
            switch (int_op) {
               case 0: result = op1 + op2; break; // add
               case 1: result = op1 - op2; break; // sub
               case 2: result = op1 * op2; break; // mul
               case 3: { // div
                  if (op2 == 0) abort();
                  if (is_signed) {
                     int64_t lhs = (int64_t)op1;
                     int64_t rhs = (int64_t)op2;
                     if ((width == 32 && lhs == INT32_MIN && rhs == -1) ||
                         (width == 64 && lhs == INT64_MIN && rhs == -1)) {
                        abort();
                     }
                     result = (uint64_t)(lhs / rhs);
                  } else {
                     result = op1 / op2;
                  }
                  break;
               }
               case 4: { // mod
                  if (op2 == 0) abort();
                  if (is_signed) {
                     int64_t lhs = (int64_t)op1;
                     int64_t rhs = (int64_t)op2;
                     if ((width == 32 && lhs == INT32_MIN && rhs == -1) ||
                         (width == 64 && lhs == INT64_MIN && rhs == -1)) {
                        result = 0;
                     } else {
                        result = (uint64_t)(lhs % rhs);
                     }
                  } else {
                     result = op1 % op2;
                  }
                  break;
               }
               case 5: result = op1 << (op2 & (width - 1)); break; // shl
               case 6: // shr
                  if (is_signed) {
                     result = (uint64_t)((int64_t)op1 >> (op2 & (width - 1)));
                  } else {
                     result = op1 >> (op2 & (width - 1));
                  }
                  break;
               case 7: result = op1 & op2; break;
               case 8: result = op1 ^ op2; break;
               case 9: result = op1 | op2; break;
               case 10: // lt
                  result = is_signed ? (int64_t)op1 < (int64_t)op2 : op1 < op2;
                  comparison = true;
                  break;
               case 11: // le
                  result = is_signed ? (int64_t)op1 <= (int64_t)op2 : op1 <= op2;
                  comparison = true;
                  break;
               case 12: // gt
                  result = is_signed ? (int64_t)op1 > (int64_t)op2 : op1 > op2;
                  comparison = true;
                  break;
               case 13: // ge
                  result = is_signed ? (int64_t)op1 >= (int64_t)op2 : op1 >= op2;
                  comparison = true;
                  break;
               case 14: result = op1 == op2; comparison = true; break;
               case 15: result = op1 != op2; comparison = true; break;
               default:
                  printf("invalid integer binary op: %u\n", int_op);
                  exit(EXIT_FAILURE);
            }
            if (comparison) {
               cm1_push_i(result);
            } else {
               cm1_push_u(cm1_int_convert(result, kind));
            }
            break;
         }
         case CM1_OP_PUSH_MEM_INT: {
            uint8_t kind = *(bc++);
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            cm1_push_u(cm1_load_int(addr + offset, kind));
            break;
         }
         case CM1_OP_SET_MEM_INT: {
            uint8_t kind = *(bc++);
            uint64_t value = cm1_pop_u64();
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            cm1_push_u(cm1_store_int(addr + offset, value, kind));
            break;
         }
         case CM1_OP_LVAR_INC_DEC_INT: {
            uint16_t var_idx = *((uint16_t*)bc);
            bc += sizeof(uint16_t);
            uint8_t mode_kind = *(bc++);
            uint8_t mode = mode_kind >> 4;
            uint8_t kind = mode_kind & 15;
            uint64_t old_val = cm1_int_convert(
               cm1_stack_v[cm1_lvar_pos + var_idx].u64, kind);
            uint64_t new_val = cm1_int_convert(
               (mode & 1) ? old_val + 1 : old_val - 1, kind);
            cm1_stack_v[cm1_lvar_pos + var_idx].u64 = new_val;
            cm1_push_u((mode & 2) ? new_val : old_val);
            break;
         }
         case CM1_OP_MEM_INC_DEC_INT: {
            uint8_t mode_kind = *(bc++);
            uint8_t mode = mode_kind >> 4;
            uint8_t kind = mode_kind & 15;
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            uint64_t old_val = cm1_load_int(addr + offset, kind);
            uint64_t new_val = cm1_int_convert(
               (mode & 1) ? old_val + 1 : old_val - 1, kind);
            cm1_store_int(addr + offset, new_val, kind);
            cm1_push_u((mode & 2) ? new_val : old_val);
            break;
         }
         case CM1_OP_MEM_INC_DEC_PTR: {
            uint8_t mode = *(bc++);
            size_t element_size = cm1_pop_usz();
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            uint8_t* old_value = *(void**)(addr + offset);
            uint8_t* new_value = (mode & 1)
               ? old_value + element_size : old_value - element_size;
            *(void**)(addr + offset) = new_value;
            cm1_push_ptr((mode & 2) ? new_value : old_value);
            break;
         }
         case CM1_OP_ADDR_ADD: {
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            cm1_push_ptr(addr + offset);
            break;
         }
         case CM1_OP_ADDR_SUB: {
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            cm1_push_ptr(addr - offset);
            break;
         }
         case CM1_OP_POP: {
            cm1_stack_pos--;
            break;
         }
         case CM1_OP_JUMP_IF_ZERO: {
            int32_t offset = *((int32_t*)bc);
            bc += sizeof(int32_t);
            if (cm1_pop_i64() == 0) bc += offset;
            break;
         }
         case CM1_OP_JUMP: {
            int32_t offset = *((int32_t*)bc);
            bc += sizeof(int32_t) + offset;
            break;
         }
         case CM1_OP_DUP_PAIR: {
            cm1_stack_v[cm1_stack_pos] = cm1_stack_v[cm1_stack_pos - 2];
            cm1_stack_v[cm1_stack_pos + 1] = cm1_stack_v[cm1_stack_pos - 1];
            cm1_stack_pos += 2;
            break;
         }
         case CM1_OP_INEG: {
            uint64_t op1 = cm1_pop_u64();
            cm1_push_u(0 - op1);
            break;
         }
         case CM1_OP_BIT_NOT: {
            uint64_t op1 = cm1_pop_u64();
            cm1_push_u(~op1);
            break;
         }
         case CM1_OP_LOGICAL_NOT: {
            int64_t op1 = cm1_pop_i64();
            cm1_push_i(!op1);
            break;
         }
         case CM1_OP_PUSH_F32: {
            float value = *(const cm1_unaligned_f32*)bc;
            bc += sizeof(float);
            cm1_push_f32(value);
            break;
         }
         case CM1_OP_PUSH_F64: {
            double value = *(const cm1_unaligned_f64*)bc;
            bc += sizeof(double);
            cm1_push_f64(value);
            break;
         }
         case CM1_OP_NUM_CONVERT: {
            uint8_t kinds = *(bc++);
            uint8_t src_kind = kinds >> 4;
            uint8_t des_kind = kinds & 15;
            cm1_stack_v[cm1_stack_pos - 1] = cm1_number_convert(
               cm1_stack_v[cm1_stack_pos - 1], src_kind, des_kind);
            break;
         }
         case CM1_OP_FLOAT_BINARY: {
            uint8_t op_result = *(bc++);
            uint8_t operand_kinds = *(bc++);
            uint8_t float_op = op_result >> 4;
            uint8_t result_kind = op_result & 15;
            uint8_t op1_kind = operand_kinds >> 4;
            uint8_t op2_kind = operand_kinds & 15;
            union cm1_stack_item op2 = cm1_stack_v[--cm1_stack_pos];
            union cm1_stack_item op1 = cm1_stack_v[--cm1_stack_pos];
            bool comparison = float_op >= 10;
            bool comparison_result = false;
            if (result_kind == CM1_TYPE_F32) {
               float lhs = cm1_number_as_f32(op1, op1_kind);
               float rhs = cm1_number_as_f32(op2, op2_kind);
               float result = 0.0f;
               switch (float_op) {
                  case 0: result = lhs + rhs; break;
                  case 1: result = lhs - rhs; break;
                  case 2: result = lhs * rhs; break;
                  case 3: result = lhs / rhs; break;
                  case 10: comparison_result = lhs < rhs; break;
                  case 11: comparison_result = lhs <= rhs; break;
                  case 12: comparison_result = lhs > rhs; break;
                  case 13: comparison_result = lhs >= rhs; break;
                  case 14: comparison_result = lhs == rhs; break;
                  case 15: comparison_result = lhs != rhs; break;
                  default:
                     printf("invalid float binary op: %u\n", float_op);
                     exit(EXIT_FAILURE);
               }
               if (!comparison) cm1_push_f32(result);
            } else if (result_kind == CM1_TYPE_F64) {
               double lhs = cm1_number_as_f64(op1, op1_kind);
               double rhs = cm1_number_as_f64(op2, op2_kind);
               double result = 0.0;
               switch (float_op) {
                  case 0: result = lhs + rhs; break;
                  case 1: result = lhs - rhs; break;
                  case 2: result = lhs * rhs; break;
                  case 3: result = lhs / rhs; break;
                  case 10: comparison_result = lhs < rhs; break;
                  case 11: comparison_result = lhs <= rhs; break;
                  case 12: comparison_result = lhs > rhs; break;
                  case 13: comparison_result = lhs >= rhs; break;
                  case 14: comparison_result = lhs == rhs; break;
                  case 15: comparison_result = lhs != rhs; break;
                  default:
                     printf("invalid double binary op: %u\n", float_op);
                     exit(EXIT_FAILURE);
               }
               if (!comparison) cm1_push_f64(result);
            } else {
               printf("invalid floating result type: %u\n", result_kind);
               exit(EXIT_FAILURE);
            }
            if (comparison) cm1_push_i(comparison_result);
            break;
         }
         case CM1_OP_PUSH_MEM_FLOAT: {
            uint8_t kind = *(bc++);
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            if (kind == CM1_TYPE_F32) {
               cm1_push_f32(*(const cm1_unaligned_f32*)(addr + offset));
            } else if (kind == CM1_TYPE_F64) {
               cm1_push_f64(*(const cm1_unaligned_f64*)(addr + offset));
            } else {
               printf("invalid floating memory type: %u\n", kind);
               exit(EXIT_FAILURE);
            }
            break;
         }
         case CM1_OP_SET_MEM_FLOAT: {
            uint8_t kind = *(bc++);
            union cm1_stack_item value = cm1_stack_v[--cm1_stack_pos];
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            if (kind == CM1_TYPE_F32) {
               *(cm1_unaligned_f32*)(addr + offset) = value.f32;
            } else if (kind == CM1_TYPE_F64) {
               *(cm1_unaligned_f64*)(addr + offset) = value.f64;
            } else {
               printf("invalid floating memory type: %u\n", kind);
               exit(EXIT_FAILURE);
            }
            cm1_stack_v[cm1_stack_pos++] = value;
            break;
         }
         case CM1_OP_FNEG: {
            uint8_t kind = *(bc++);
            if (kind == CM1_TYPE_F32) {
               cm1_stack_v[cm1_stack_pos - 1].f32 =
                  -cm1_stack_v[cm1_stack_pos - 1].f32;
            } else if (kind == CM1_TYPE_F64) {
               cm1_stack_v[cm1_stack_pos - 1].f64 =
                  -cm1_stack_v[cm1_stack_pos - 1].f64;
            } else {
               printf("invalid floating negation type: %u\n", kind);
               exit(EXIT_FAILURE);
            }
            break;
         }
         case CM1_OP_FLOAT_TO_BOOL: {
            uint8_t kind = *(bc++);
            union cm1_stack_item value = cm1_stack_v[--cm1_stack_pos];
            if (kind == CM1_TYPE_F32) {
               cm1_push_i(value.f32 != 0.0f);
            } else if (kind == CM1_TYPE_F64) {
               cm1_push_i(value.f64 != 0.0);
            } else {
               printf("invalid floating condition type: %u\n", kind);
               exit(EXIT_FAILURE);
            }
            break;
         }
         case CM1_OP_LVAR_INC_DEC_FLOAT: {
            uint16_t var_idx = *((uint16_t*)bc);
            bc += sizeof(uint16_t);
            uint8_t mode_kind = *(bc++);
            uint8_t mode = mode_kind >> 4;
            uint8_t kind = mode_kind & 15;
            union cm1_stack_item old_value =
               cm1_stack_v[cm1_lvar_pos + var_idx];
            union cm1_stack_item new_value = old_value;
            if (kind == CM1_TYPE_F32) {
               new_value.f32 += (mode & 1) ? 1.0f : -1.0f;
            } else if (kind == CM1_TYPE_F64) {
               new_value.f64 += (mode & 1) ? 1.0 : -1.0;
            } else {
               printf("invalid floating increment type: %u\n", kind);
               exit(EXIT_FAILURE);
            }
            cm1_stack_v[cm1_lvar_pos + var_idx] = new_value;
            cm1_stack_v[cm1_stack_pos++] =
               (mode & 2) ? new_value : old_value;
            break;
         }
         case CM1_OP_MEM_INC_DEC_FLOAT: {
            uint8_t mode_kind = *(bc++);
            uint8_t mode = mode_kind >> 4;
            uint8_t kind = mode_kind & 15;
            ssize_t offset = cm1_pop_isz();
            uint8_t* addr = cm1_pop_ptr();
            union cm1_stack_item old_value;
            union cm1_stack_item new_value;
            old_value.u64 = 0;
            if (kind == CM1_TYPE_F32) {
               old_value.f32 = *(const cm1_unaligned_f32*)(addr + offset);
               new_value = old_value;
               new_value.f32 += (mode & 1) ? 1.0f : -1.0f;
               *(cm1_unaligned_f32*)(addr + offset) = new_value.f32;
            } else if (kind == CM1_TYPE_F64) {
               old_value.f64 = *(const cm1_unaligned_f64*)(addr + offset);
               new_value = old_value;
               new_value.f64 += (mode & 1) ? 1.0 : -1.0;
               *(cm1_unaligned_f64*)(addr + offset) = new_value.f64;
            } else {
               printf("invalid floating increment type: %u\n", kind);
               exit(EXIT_FAILURE);
            }
            cm1_stack_v[cm1_stack_pos++] =
               (mode & 2) ? new_value : old_value;
            break;
         }
         default:
            printf("invalid bytecode op: %u\n", op);
            exit(EXIT_FAILURE);
      }
   }
}
