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

static void cm1_push_f32(float value) {
   cm1_stack_v[cm1_stack_pos].u64 = 0;
   cm1_stack_v[cm1_stack_pos++].f32 = value;
}

static void cm1_push_f64(double value) {
   cm1_stack_v[cm1_stack_pos++].f64 = value;
}

void cm1_init(const char* cm1_path);
void cm1_run(uint16_t func_idx);
