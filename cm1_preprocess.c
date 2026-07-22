#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/* Both limits include the trailing zero byte. They may be overridden by the
 * build when a smaller or larger fixed-memory preprocessor is desired. */
#ifndef INPUT_LIMIT
#define INPUT_LIMIT (16u * 1024u * 1024u)
#endif

#ifndef OUTPUT_LIMIT
#define OUTPUT_LIMIT (16u * 1024u * 1024u)
#endif

char input_mem[INPUT_LIMIT];
void* _Tcm1_Fcfg_mem_1(uint32_t size) {
   return &input_mem[INPUT_LIMIT - size];
}
char output_mem[OUTPUT_LIMIT];

#define PP_MACRO_STORAGE_LIMIT (INPUT_LIMIT / 8u)
#define PP_MACRO_LIMIT 2048u
#define PP_EXPANSION_LIMIT 64u
#define PP_MACRO_ARGUMENT_LIMIT 128u
#define PP_INCLUDE_DEPTH_LIMIT 64u
#define PP_CONDITION_LIMIT 128u
#define PP_LINE_LIMIT (64u * 1024u)
#define PP_PATH_LIMIT 4096u
#define PP_ERROR_LIMIT (2u * PP_PATH_LIMIT + 128u)

struct pp_macro {
   size_t name_offset;
   size_t name_length;
   size_t value_offset;
   size_t value_length;
   bool defined;
   bool function_like;
};

struct pp_sink {
   char* data;
   size_t length;
   size_t limit;
};

struct pp_macro_text {
   const char* text;
   size_t length;
   const struct pp_macro_text* parameters;
   const struct pp_macro_text* arguments;
   size_t parameter_count;
};

struct pp_condition {
   bool parent_active;
   bool active;
   bool branch_taken;
   bool saw_else;
};

struct pp_expr {
   const char* text;
   size_t length;
   size_t pos;
   bool error;
};

static struct pp_macro pp_macros[PP_MACRO_LIMIT];
static struct pp_macro_text
   pp_macro_parameter_mem[PP_EXPANSION_LIMIT][PP_MACRO_ARGUMENT_LIMIT];
static struct pp_macro_text
   pp_macro_argument_mem[PP_EXPANSION_LIMIT][PP_MACRO_ARGUMENT_LIMIT];
static char pp_macro_expansion_mem[PP_EXPANSION_LIMIT][PP_LINE_LIMIT];
static size_t pp_macro_count;
static size_t pp_macro_mem_pos;
static size_t pp_file_mem_pos;
static size_t pp_output_pos;
static bool pp_failed;
static char pp_error_mem[PP_ERROR_LIMIT];
static size_t pp_error_length;

static size_t pp_include_path_count;
static const char* const* pp_include_path_list;

static bool pp_is_space(char c) {
   return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
}

static bool pp_is_ident_start(char c) {
   return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool pp_is_ident(char c) {
   return pp_is_ident_start(c) || (c >= '0' && c <= '9');
}

static bool pp_text_equal(
   const char* a, size_t a_length, const char* b, size_t b_length
) {
   return a_length == b_length && memcmp(a, b, a_length) == 0;
}

static void pp_fail(void) {
   pp_failed = true;
}

static void pp_error_append(const char* text, size_t length) {
   size_t available;
   if (PP_ERROR_LIMIT == 0 || length == 0) return;
   available = PP_ERROR_LIMIT - 1u - pp_error_length;
   if (length > available) length = available;
   memcpy(pp_error_mem + pp_error_length, text, length);
   pp_error_length += length;
   pp_error_mem[pp_error_length] = '\0';
}

static void pp_error_append_size(size_t value) {
   char digits[3u * sizeof(size_t) + 1u];
   size_t digit_count = 0;
   do {
      digits[digit_count++] = (char)('0' + value % 10u);
      value /= 10u;
   } while (value != 0);
   while (digit_count != 0) {
      pp_error_append(&digits[--digit_count], 1);
   }
}

static void pp_error_missing_include(
   const char* path, size_t line, const char* include_name
) {
   if (pp_error_length != 0) return;
   pp_error_append(path, strlen(path));
   pp_error_append(":", 1);
   pp_error_append_size(line);
   pp_error_append(
      ": error: include file '", sizeof(": error: include file '") - 1u
   );
   pp_error_append(include_name, strlen(include_name));
   pp_error_append("' was not found\n", sizeof("' was not found\n") - 1u);
}

static void pp_error_variadic_macro(
   const char* path, size_t line, const char* name, size_t name_length
) {
   if (pp_error_length != 0) return;
   pp_error_append(path, strlen(path));
   pp_error_append(":", 1);
   pp_error_append_size(line);
   pp_error_append(
      ": error: variadic macro '",
      sizeof(": error: variadic macro '") - 1u
   );
   pp_error_append(name, name_length);
   pp_error_append(
      "' is not supported\n", sizeof("' is not supported\n") - 1u
   );
}

const char* preprocess_error_message(void) {
   return pp_error_mem;
}

static bool pp_sink_write(struct pp_sink* sink, const char* data, size_t length) {
   if (pp_failed) return false;
   if (length > sink->limit - sink->length) {
      pp_fail();
      return false;
   }
   memcpy(sink->data + sink->length, data, length);
   sink->length += length;
   return true;
}

static bool pp_sink_char(struct pp_sink* sink, char c) {
   return pp_sink_write(sink, &c, 1);
}

static bool pp_output_write(const char* data, size_t length) {
   struct pp_sink sink = {
      output_mem,
      pp_output_pos,
      OUTPUT_LIMIT > 0 ? OUTPUT_LIMIT - 1u : 0u
   };
   bool result = pp_sink_write(&sink, data, length);
   pp_output_pos = sink.length;
   return result;
}

static bool pp_output_char(char c) {
   return pp_output_write(&c, 1);
}

static size_t pp_skip_space(const char* text, size_t length, size_t pos) {
   while (pos < length && pp_is_space(text[pos])) pos++;
   return pos;
}

static size_t pp_trim_end(const char* text, size_t begin, size_t end) {
   while (end > begin && pp_is_space(text[end - 1])) end--;
   return end;
}

static ptrdiff_t pp_find_macro(const char* name, size_t length) {
   for (size_t i = pp_macro_count; i != 0; i--) {
      struct pp_macro* macro = &pp_macros[i - 1];
      const char* stored_name = input_mem + macro->name_offset;
      if (pp_text_equal(name, length, stored_name, macro->name_length)) {
         return (ptrdiff_t)(i - 1);
      }
   }
   return -1;
}

static bool pp_macro_is_defined(const char* name, size_t length) {
   ptrdiff_t index = pp_find_macro(name, length);
   return index >= 0 && pp_macros[index].defined;
}

static bool pp_store_macro_text(
   const char* text, size_t length, size_t* offset_out
) {
   if (length + 1u > PP_MACRO_STORAGE_LIMIT - pp_macro_mem_pos) {
      pp_fail();
      return false;
   }
   *offset_out = pp_macro_mem_pos;
   memcpy(input_mem + pp_macro_mem_pos, text, length);
   input_mem[pp_macro_mem_pos + length] = '\0';
   pp_macro_mem_pos += length + 1u;
   return true;
}

/* Supplied definitions bypass pp_source_char(), so normalize replacement
 * text while copying it into the macro arena. */
static bool pp_store_macro_value(
   const char* text,
   size_t length,
   size_t* offset_out,
   size_t* length_out
) {
   size_t read_pos = 0;
   size_t write_pos = pp_macro_mem_pos;

   *offset_out = write_pos;
   while (read_pos < length) {
      char c = text[read_pos++];
      if (c == '\r') {
         if (read_pos < length && text[read_pos] == '\n') read_pos++;
         c = '\n';
      }
      if (write_pos + 1u >= PP_MACRO_STORAGE_LIMIT) {
         pp_fail();
         return false;
      }
      input_mem[write_pos++] = c;
   }
   if (write_pos >= PP_MACRO_STORAGE_LIMIT) {
      pp_fail();
      return false;
   }
   input_mem[write_pos] = '\0';
   *length_out = write_pos - *offset_out;
   pp_macro_mem_pos = write_pos + 1u;
   return true;
}

static bool pp_set_macro(
   const char* name,
   size_t name_length,
   const char* value,
   size_t value_length,
   bool function_like
) {
   ptrdiff_t found;
   struct pp_macro* macro;
   size_t value_offset;
   size_t stored_value_length;

   if (name_length == 0 || !pp_is_ident_start(name[0])) {
      pp_fail();
      return false;
   }
   for (size_t i = 1; i < name_length; i++) {
      if (!pp_is_ident(name[i])) {
         pp_fail();
         return false;
      }
   }

   found = pp_find_macro(name, name_length);
   if (found < 0) {
      size_t name_offset;
      if (pp_macro_count == PP_MACRO_LIMIT) {
         pp_fail();
         return false;
      }
      if (!pp_store_macro_text(name, name_length, &name_offset)) return false;
      macro = &pp_macros[pp_macro_count++];
      macro->name_offset = name_offset;
      macro->name_length = name_length;
   } else {
      macro = &pp_macros[found];
   }

   if (!pp_store_macro_value(
      value, value_length, &value_offset, &stored_value_length
   )) return false;
   macro->value_offset = value_offset;
   macro->value_length = stored_value_length;
   macro->defined = true;
   macro->function_like = function_like;
   return true;
}

static void pp_undef_macro(const char* name, size_t length) {
   ptrdiff_t index = pp_find_macro(name, length);
   if (index >= 0) pp_macros[index].defined = false;
}

static bool pp_macro_on_stack(
   size_t macro_index, const size_t* expansion_stack, size_t stack_length
) {
   for (size_t i = 0; i < stack_length; i++) {
      if (expansion_stack[i] == macro_index) return true;
   }
   return false;
}

static bool pp_is_string_prefix(const char* text, size_t length) {
   return (length == 1 && (text[0] == 'L' || text[0] == 'u' || text[0] == 'U'))
      || (length == 2 && text[0] == 'u' && text[1] == '8');
}

static bool pp_macro_signature(
   const struct pp_macro* macro,
   struct pp_macro_text* parameters,
   size_t* parameter_count,
   const char** replacement,
   size_t* replacement_length
) {
   const char* text = input_mem + macro->value_offset;
   size_t length = macro->value_length;
   size_t pos;
   size_t count = 0;

   if (length == 0 || text[0] != '(') {
      pp_fail();
      return false;
   }
   pos = pp_skip_space(text, length, 1u);
   if (pos < length && text[pos] == ')') {
      pos++;
   } else {
      for (;;) {
         size_t begin;
         size_t end;
         if (pos == length || !pp_is_ident_start(text[pos])
            || count == PP_MACRO_ARGUMENT_LIMIT) {
            pp_fail();
            return false;
         }
         begin = pos++;
         while (pos < length && pp_is_ident(text[pos])) pos++;
         end = pos;
         for (size_t i = 0; i < count; i++) {
            if (pp_text_equal(
               text + begin, end - begin,
               parameters[i].text, parameters[i].length
            )) {
               pp_fail();
               return false;
            }
         }
         parameters[count].text = text + begin;
         parameters[count].length = end - begin;
         parameters[count].parameters = NULL;
         parameters[count].arguments = NULL;
         parameters[count].parameter_count = 0;
         count++;
         pos = pp_skip_space(text, length, pos);
         if (pos < length && text[pos] == ',') {
            pos = pp_skip_space(text, length, pos + 1u);
            continue;
         }
         if (pos < length && text[pos] == ')') {
            pos++;
            break;
         }
         pp_fail();
         return false;
      }
   }
   pos = pp_skip_space(text, length, pos);
   *parameter_count = count;
   *replacement = text + pos;
   *replacement_length = length - pos;
   return true;
}

static bool pp_macro_arguments(
   const char* text,
   size_t length,
   size_t open_pos,
   size_t expected_count,
   const struct pp_macro_text* enclosing_parameters,
   const struct pp_macro_text* enclosing_arguments,
   size_t enclosing_parameter_count,
   struct pp_macro_text* arguments,
   size_t* argument_count,
   size_t* end_pos
) {
   size_t pos = open_pos + 1u;
   size_t begin = pos;
   size_t count = 0;
   size_t depth = 1;
   bool saw_comma = false;

   while (pos < length) {
      char c = text[pos];
      if (c == '"' || c == '\'') {
         char quote = c;
         pos++;
         while (pos < length) {
            c = text[pos++];
            if (c == '\\' && pos < length) pos++;
            else if (c == quote) break;
         }
         continue;
      }
      if (c == '(') {
         depth++;
         pos++;
         continue;
      }
      if (c == ')') {
         depth--;
         if (depth == 0) {
            size_t argument_begin = pp_skip_space(text, pos, begin);
            size_t argument_end = pp_trim_end(text, argument_begin, pos);
            if (saw_comma || argument_begin != argument_end
               || expected_count != 0) {
               if (count == PP_MACRO_ARGUMENT_LIMIT) {
                  pp_fail();
                  return false;
               }
               arguments[count].text = text + argument_begin;
               arguments[count].length = argument_end - argument_begin;
               arguments[count].parameters = enclosing_parameters;
               arguments[count].arguments = enclosing_arguments;
               arguments[count].parameter_count =
                  enclosing_parameter_count;
               count++;
            }
            *argument_count = count;
            *end_pos = pos + 1u;
            return true;
         }
         pos++;
         continue;
      }
      if (c == ',' && depth == 1) {
         size_t argument_begin = pp_skip_space(text, pos, begin);
         size_t argument_end = pp_trim_end(text, argument_begin, pos);
         if (count == PP_MACRO_ARGUMENT_LIMIT) {
            pp_fail();
            return false;
         }
         arguments[count].text = text + argument_begin;
         arguments[count].length = argument_end - argument_begin;
         arguments[count].parameters = enclosing_parameters;
         arguments[count].arguments = enclosing_arguments;
         arguments[count].parameter_count = enclosing_parameter_count;
         count++;
         saw_comma = true;
         pos++;
         begin = pos;
         continue;
      }
      pos++;
   }
   pp_fail();
   return false;
}

static ptrdiff_t pp_macro_parameter(
   const struct pp_macro_text* parameters,
   size_t parameter_count,
   const char* name,
   size_t name_length
) {
   for (size_t i = 0; i < parameter_count; i++) {
      if (pp_text_equal(
         name, name_length, parameters[i].text, parameters[i].length
      )) return (ptrdiff_t)i;
   }
   return -1;
}

static bool pp_expand_text(
   const char* text,
   size_t length,
   struct pp_sink* sink,
   size_t* expansion_stack,
   size_t stack_length,
   bool preserve_defined,
   const struct pp_macro_text* parameters,
   const struct pp_macro_text* arguments,
   size_t parameter_count
) {
   size_t pos = 0;

   if (stack_length > PP_EXPANSION_LIMIT) {
      pp_fail();
      return false;
   }

   while (pos < length && !pp_failed) {
      char c = text[pos];

      if (c == '"' || c == '\'') {
         char quote = c;
         if (!pp_sink_char(sink, c)) return false;
         pos++;
         while (pos < length) {
            c = text[pos++];
            if (!pp_sink_char(sink, c)) return false;
            if (c == '\\' && pos < length) {
               if (!pp_sink_char(sink, text[pos++])) return false;
            } else if (c == quote) {
               break;
            }
         }
         continue;
      }

      if ((c >= '0' && c <= '9')
         || (c == '.' && pos + 1 < length
            && text[pos + 1] >= '0' && text[pos + 1] <= '9')) {
         size_t begin = pos++;
         while (pos < length) {
            char number_c = text[pos];
            if (pp_is_ident(number_c) || number_c == '.') {
               pos++;
            } else if ((number_c == '+' || number_c == '-') && pos > begin
               && (text[pos - 1] == 'e' || text[pos - 1] == 'E'
                  || text[pos - 1] == 'p' || text[pos - 1] == 'P')) {
               pos++;
            } else {
               break;
            }
         }
         if (!pp_sink_write(sink, text + begin, pos - begin)) return false;
         continue;
      }

      if (pp_is_ident_start(c)) {
         size_t begin = pos++;
         ptrdiff_t macro_index;
         while (pos < length && pp_is_ident(text[pos])) pos++;

         if (parameters != NULL) {
            ptrdiff_t parameter_index = pp_macro_parameter(
               parameters, parameter_count, text + begin, pos - begin
            );
            if (parameter_index >= 0) {
               const struct pp_macro_text* argument =
                  &arguments[parameter_index];
               if (!pp_expand_text(
                  argument->text,
                  argument->length,
                  sink,
                  expansion_stack,
                  stack_length,
                  preserve_defined,
                  argument->parameters,
                  argument->arguments,
                  argument->parameter_count
               )) return false;
               continue;
            }
         }

         if (pos < length && (text[pos] == '"' || text[pos] == '\'')
            && pp_is_string_prefix(text + begin, pos - begin)) {
            if (!pp_sink_write(sink, text + begin, pos - begin)) return false;
            continue;
         }

         if (preserve_defined
            && pp_text_equal(text + begin, pos - begin, "defined", 7)) {
            size_t raw_end = pos;
            if (!pp_sink_write(sink, text + begin, pos - begin)) return false;
            while (raw_end < length && pp_is_space(text[raw_end])) raw_end++;
            if (raw_end > pos
               && !pp_sink_write(sink, text + pos, raw_end - pos)) return false;
            pos = raw_end;
            if (pos < length && text[pos] == '(') {
               if (!pp_sink_char(sink, text[pos++])) return false;
               raw_end = pos;
               while (raw_end < length && pp_is_space(text[raw_end])) raw_end++;
               if (raw_end > pos
                  && !pp_sink_write(sink, text + pos, raw_end - pos)) return false;
               pos = raw_end;
            }
            if (pos < length && pp_is_ident_start(text[pos])) {
               raw_end = pos + 1;
               while (raw_end < length && pp_is_ident(text[raw_end])) raw_end++;
               if (!pp_sink_write(sink, text + pos, raw_end - pos)) return false;
               pos = raw_end;
            }
            continue;
         }

         macro_index = pp_find_macro(text + begin, pos - begin);
         if (macro_index >= 0 && pp_macros[macro_index].defined
            && !pp_macro_on_stack(
               (size_t)macro_index, expansion_stack, stack_length
            )) {
            struct pp_macro* macro = &pp_macros[macro_index];
            if (stack_length == PP_EXPANSION_LIMIT) {
               pp_fail();
               return false;
            }
            if (macro->function_like) {
               size_t call_pos = pp_skip_space(text, length, pos);
               if (call_pos < length && text[call_pos] == '(') {
                  struct pp_macro_text* macro_parameters =
                     pp_macro_parameter_mem[stack_length];
                  struct pp_macro_text* macro_arguments =
                     pp_macro_argument_mem[stack_length];
                  const char* replacement;
                  size_t replacement_length;
                  size_t macro_parameter_count;
                  size_t macro_argument_count;
                  size_t call_end;
                  if (!pp_macro_signature(
                     macro,
                     macro_parameters,
                     &macro_parameter_count,
                     &replacement,
                     &replacement_length
                  )) return false;
                  if (!pp_macro_arguments(
                     text,
                     length,
                     call_pos,
                     macro_parameter_count,
                     parameters,
                     arguments,
                     parameter_count,
                     macro_arguments,
                     &macro_argument_count,
                     &call_end
                  )) return false;
                  if (macro_argument_count != macro_parameter_count) {
                     pp_fail();
                     return false;
                  }
                  {
                     struct pp_sink argument_sink = {
                        pp_macro_expansion_mem[stack_length],
                        0,
                        PP_LINE_LIMIT - 1u
                     };
                     /* Function arguments are expanded before the invoked
                      * macro itself is disabled. Reserve this depth with a
                      * sentinel so nested calls use the next scratch level. */
                     expansion_stack[stack_length] = (size_t)-1;
                     for (size_t i = 0; i < macro_argument_count; i++) {
                        struct pp_macro_text raw_argument = macro_arguments[i];
                        size_t argument_begin = argument_sink.length;
                        if (!pp_expand_text(
                           raw_argument.text,
                           raw_argument.length,
                           &argument_sink,
                           expansion_stack,
                           stack_length + 1u,
                           preserve_defined,
                           raw_argument.parameters,
                           raw_argument.arguments,
                           raw_argument.parameter_count
                        )) return false;
                        macro_arguments[i].text = argument_sink.data
                           + argument_begin;
                        macro_arguments[i].length = argument_sink.length
                           - argument_begin;
                        macro_arguments[i].parameters = NULL;
                        macro_arguments[i].arguments = NULL;
                        macro_arguments[i].parameter_count = 0;
                     }
                  }
                  expansion_stack[stack_length] = (size_t)macro_index;
                  pos = call_end;
                  if (!pp_expand_text(
                     replacement,
                     replacement_length,
                     sink,
                     expansion_stack,
                     stack_length + 1u,
                     preserve_defined,
                     macro_parameters,
                     macro_arguments,
                     macro_parameter_count
                  )) return false;
                  continue;
               }
            } else {
               expansion_stack[stack_length] = (size_t)macro_index;
               if (!pp_expand_text(
                  input_mem + macro->value_offset,
                  macro->value_length,
                  sink,
                  expansion_stack,
                  stack_length + 1u,
                  preserve_defined,
                  NULL,
                  NULL,
                  0
               )) return false;
               continue;
            }
            if (!pp_sink_write(sink, text + begin, pos - begin)) return false;
         } else if (!pp_sink_write(sink, text + begin, pos - begin)) {
            return false;
         }
         continue;
      }

      if (!pp_sink_char(sink, c)) return false;
      pos++;
   }
   return !pp_failed;
}

static bool pp_expand_to_output(const char* text, size_t length) {
   size_t expansion_stack[PP_EXPANSION_LIMIT];
   struct pp_sink sink = {
      output_mem,
      pp_output_pos,
      OUTPUT_LIMIT > 0 ? OUTPUT_LIMIT - 1u : 0u
   };
   bool result = pp_expand_text(
      text, length, &sink, expansion_stack, 0, false, NULL, NULL, 0
   );
   pp_output_pos = sink.length;
   return result;
}

static void pp_expr_space(struct pp_expr* expr) {
   while (expr->pos < expr->length && pp_is_space(expr->text[expr->pos])) {
      expr->pos++;
   }
}

static bool pp_expr_match(struct pp_expr* expr, const char* op, size_t length) {
   pp_expr_space(expr);
   if (length <= expr->length - expr->pos
      && memcmp(expr->text + expr->pos, op, length) == 0) {
      expr->pos += length;
      return true;
   }
   return false;
}

static int pp_digit_value(char c) {
   if (c >= '0' && c <= '9') return c - '0';
   if (c >= 'a' && c <= 'f') return c - 'a' + 10;
   if (c >= 'A' && c <= 'F') return c - 'A' + 10;
   return -1;
}

static int64_t pp_expr_comma(struct pp_expr* expr);

static int64_t pp_expr_primary(struct pp_expr* expr) {
   uint64_t value = 0;
   unsigned base = 10;
   bool found_digit = false;

   pp_expr_space(expr);
   if (expr->pos == expr->length) {
      expr->error = true;
      return 0;
   }

   if (expr->text[expr->pos] == '(') {
      int64_t result;
      expr->pos++;
      result = pp_expr_comma(expr);
      if (!pp_expr_match(expr, ")", 1)) expr->error = true;
      return result;
   }

   if (expr->text[expr->pos] == '\'') {
      int result;
      expr->pos++;
      if (expr->pos == expr->length) {
         expr->error = true;
         return 0;
      }
      if (expr->text[expr->pos] == '\\') {
         expr->pos++;
         if (expr->pos == expr->length) {
            expr->error = true;
            return 0;
         }
         switch (expr->text[expr->pos++]) {
            case 'n': result = '\n'; break;
            case 'r': result = '\r'; break;
            case 't': result = '\t'; break;
            case 'v': result = '\v'; break;
            case 'f': result = '\f'; break;
            case 'a': result = '\a'; break;
            case 'b': result = '\b'; break;
            default: result = (unsigned char)expr->text[expr->pos - 1]; break;
         }
      } else {
         result = (unsigned char)expr->text[expr->pos++];
      }
      if (expr->pos >= expr->length || expr->text[expr->pos] != '\'') {
         expr->error = true;
      } else {
         expr->pos++;
      }
      return result;
   }

   if (pp_is_ident_start(expr->text[expr->pos])) {
      size_t begin = expr->pos++;
      while (expr->pos < expr->length && pp_is_ident(expr->text[expr->pos])) {
         expr->pos++;
      }
      if (pp_text_equal(expr->text + begin, expr->pos - begin, "defined", 7)) {
         bool parenthesized;
         size_t name_begin;
         size_t name_end;
         pp_expr_space(expr);
         parenthesized = pp_expr_match(expr, "(", 1);
         pp_expr_space(expr);
         if (expr->pos == expr->length
            || !pp_is_ident_start(expr->text[expr->pos])) {
            expr->error = true;
            return 0;
         }
         name_begin = expr->pos++;
         while (expr->pos < expr->length
            && pp_is_ident(expr->text[expr->pos])) expr->pos++;
         name_end = expr->pos;
         if (parenthesized && !pp_expr_match(expr, ")", 1)) expr->error = true;
         return pp_macro_is_defined(
            expr->text + name_begin, name_end - name_begin
         );
      }
      /* Undefined identifiers in a preprocessing expression have value zero. */
      return 0;
   }

   if (expr->text[expr->pos] == '0' && expr->pos + 1 < expr->length) {
      char prefix = expr->text[expr->pos + 1];
      if (prefix == 'x' || prefix == 'X') {
         base = 16;
         expr->pos += 2;
      } else if (prefix == 'b' || prefix == 'B') {
         base = 2;
         expr->pos += 2;
      } else {
         base = 8;
         expr->pos++;
         found_digit = true;
      }
   }

   while (expr->pos < expr->length) {
      int digit = pp_digit_value(expr->text[expr->pos]);
      if (digit < 0 || (unsigned)digit >= base) break;
      found_digit = true;
      value = value * base + (unsigned)digit;
      expr->pos++;
   }
   if (!found_digit) {
      expr->error = true;
      return 0;
   }
   while (expr->pos < expr->length) {
      char suffix = expr->text[expr->pos];
      if (suffix != 'u' && suffix != 'U' && suffix != 'l' && suffix != 'L') break;
      expr->pos++;
   }
   return (int64_t)value;
}

static int64_t pp_expr_unary(struct pp_expr* expr) {
   if (pp_expr_match(expr, "+", 1)) return pp_expr_unary(expr);
   if (pp_expr_match(expr, "-", 1)) {
      return (int64_t)(UINT64_C(0) - (uint64_t)pp_expr_unary(expr));
   }
   if (pp_expr_match(expr, "!", 1)) return !pp_expr_unary(expr);
   if (pp_expr_match(expr, "~", 1)) return ~pp_expr_unary(expr);
   return pp_expr_primary(expr);
}

static int64_t pp_expr_multiply(struct pp_expr* expr) {
   int64_t value = pp_expr_unary(expr);
   for (;;) {
      if (pp_expr_match(expr, "*", 1)) {
         value = (int64_t)((uint64_t)value
            * (uint64_t)pp_expr_unary(expr));
      } else if (pp_expr_match(expr, "/", 1)) {
         int64_t right = pp_expr_unary(expr);
         if (right == 0) value = 0;
         else if (value == INT64_MIN && right == -1) value = INT64_MIN;
         else value /= right;
      } else if (pp_expr_match(expr, "%", 1)) {
         int64_t right = pp_expr_unary(expr);
         if (right == 0 || (value == INT64_MIN && right == -1)) value = 0;
         else value %= right;
      } else {
         return value;
      }
   }
}

static int64_t pp_expr_add(struct pp_expr* expr) {
   int64_t value = pp_expr_multiply(expr);
   for (;;) {
      if (pp_expr_match(expr, "+", 1)) {
         value = (int64_t)((uint64_t)value
            + (uint64_t)pp_expr_multiply(expr));
      } else if (pp_expr_match(expr, "-", 1)) {
         value = (int64_t)((uint64_t)value
            - (uint64_t)pp_expr_multiply(expr));
      }
      else return value;
   }
}

static int64_t pp_expr_shift(struct pp_expr* expr) {
   int64_t value = pp_expr_add(expr);
   for (;;) {
      if (pp_expr_match(expr, "<<", 2)) {
         int64_t right = pp_expr_add(expr);
         value = right < 0 || right >= 64 ? 0 : (int64_t)((uint64_t)value << right);
      } else if (pp_expr_match(expr, ">>", 2)) {
         int64_t right = pp_expr_add(expr);
         value = right < 0 || right >= 64 ? 0 : value >> right;
      } else {
         return value;
      }
   }
}

static int64_t pp_expr_relation(struct pp_expr* expr) {
   int64_t value = pp_expr_shift(expr);
   for (;;) {
      if (pp_expr_match(expr, "<=", 2)) value = value <= pp_expr_shift(expr);
      else if (pp_expr_match(expr, ">=", 2)) value = value >= pp_expr_shift(expr);
      else if (pp_expr_match(expr, "<", 1)) value = value < pp_expr_shift(expr);
      else if (pp_expr_match(expr, ">", 1)) value = value > pp_expr_shift(expr);
      else return value;
   }
}

static int64_t pp_expr_equal(struct pp_expr* expr) {
   int64_t value = pp_expr_relation(expr);
   for (;;) {
      if (pp_expr_match(expr, "==", 2)) value = value == pp_expr_relation(expr);
      else if (pp_expr_match(expr, "!=", 2)) value = value != pp_expr_relation(expr);
      else return value;
   }
}

static int64_t pp_expr_bit_and(struct pp_expr* expr) {
   int64_t value = pp_expr_equal(expr);
   for (;;) {
      pp_expr_space(expr);
      if (expr->pos < expr->length && expr->text[expr->pos] == '&'
         && (expr->pos + 1 == expr->length || expr->text[expr->pos + 1] != '&')) {
         expr->pos++;
         value &= pp_expr_equal(expr);
      } else {
         return value;
      }
   }
}

static int64_t pp_expr_bit_xor(struct pp_expr* expr) {
   int64_t value = pp_expr_bit_and(expr);
   while (pp_expr_match(expr, "^", 1)) value ^= pp_expr_bit_and(expr);
   return value;
}

static int64_t pp_expr_bit_or(struct pp_expr* expr) {
   int64_t value = pp_expr_bit_xor(expr);
   for (;;) {
      pp_expr_space(expr);
      if (expr->pos < expr->length && expr->text[expr->pos] == '|'
         && (expr->pos + 1 == expr->length || expr->text[expr->pos + 1] != '|')) {
         expr->pos++;
         value |= pp_expr_bit_xor(expr);
      } else {
         return value;
      }
   }
}

static int64_t pp_expr_logical_and(struct pp_expr* expr) {
   int64_t value = pp_expr_bit_or(expr);
   while (pp_expr_match(expr, "&&", 2)) {
      int64_t right = pp_expr_bit_or(expr);
      value = value && right;
   }
   return value;
}

static int64_t pp_expr_logical_or(struct pp_expr* expr) {
   int64_t value = pp_expr_logical_and(expr);
   while (pp_expr_match(expr, "||", 2)) {
      int64_t right = pp_expr_logical_and(expr);
      value = value || right;
   }
   return value;
}

static int64_t pp_expr_conditional(struct pp_expr* expr) {
   int64_t condition = pp_expr_logical_or(expr);
   if (pp_expr_match(expr, "?", 1)) {
      int64_t true_value = pp_expr_comma(expr);
      int64_t false_value;
      if (!pp_expr_match(expr, ":", 1)) {
         expr->error = true;
         return 0;
      }
      false_value = pp_expr_conditional(expr);
      return condition ? true_value : false_value;
   }
   return condition;
}

static int64_t pp_expr_comma(struct pp_expr* expr) {
   int64_t value = pp_expr_conditional(expr);
   while (pp_expr_match(expr, ",", 1)) value = pp_expr_conditional(expr);
   return value;
}

static bool pp_evaluate_condition(const char* text, size_t length) {
   char expanded[PP_LINE_LIMIT];
   size_t expansion_stack[PP_EXPANSION_LIMIT];
   struct pp_sink sink = { expanded, 0, sizeof(expanded) - 1u };
   struct pp_expr expr;
   int64_t value;

   if (!pp_expand_text(
      text, length, &sink, expansion_stack, 0, true, NULL, NULL, 0
   )) return false;
   expanded[sink.length] = '\0';
   expr.text = expanded;
   expr.length = sink.length;
   expr.pos = 0;
   expr.error = false;
   value = pp_expr_comma(&expr);
   pp_expr_space(&expr);
   if (expr.error || expr.pos != expr.length) {
      pp_fail();
      return false;
   }
   return value != 0;
}

static int pp_open_read(const char* path) {
#ifdef _WIN32
   return open(path, O_RDONLY | O_BINARY);
#else
   return open(path, O_RDONLY);
#endif
}

static bool pp_path_is_absolute(const char* path) {
   if (path[0] == '\0') return false;
   if (path[0] == '/' || path[0] == '\\') return true;
#ifdef _WIN32
   if (path[1] != '\0' && ((path[0] >= 'a' && path[0] <= 'z')
      || (path[0] >= 'A' && path[0] <= 'Z')) && path[1] == ':') return true;
#endif
   return false;
}

static bool pp_copy_path(char* out, const char* path) {
   size_t length = strlen(path);
   if (length + 1u > PP_PATH_LIMIT) return false;
   memcpy(out, path, length + 1u);
   return true;
}

static bool pp_join_path(char* out, const char* directory, const char* name) {
   size_t directory_length = strlen(directory);
   size_t name_length = strlen(name);
   bool separator = directory_length != 0
      && directory[directory_length - 1] != '/'
      && directory[directory_length - 1] != '\\';
   if (directory_length + separator + name_length + 1u > PP_PATH_LIMIT) {
      return false;
   }
   memmove(out, directory, directory_length);
   if (separator) out[directory_length++] = '/';
   memcpy(out + directory_length, name, name_length + 1u);
   return true;
}

static int pp_open_include_candidate(
   const char* candidate, char* resolved_path
) {
   int fd;
   if (!pp_copy_path(resolved_path, candidate)) return -1;
   fd = pp_open_read(resolved_path);
   return fd;
}

static int pp_open_include(
   const char* name,
   bool quoted,
   const char* current_path,
   char* resolved_path
) {
   char candidate[PP_PATH_LIMIT];

   if (pp_path_is_absolute(name)) {
      return pp_open_include_candidate(name, resolved_path);
   }

   if (quoted) {
      const char* last_separator = NULL;
      for (const char* p = current_path; *p; p++) {
         if (*p == '/' || *p == '\\') last_separator = p;
      }
      if (last_separator != NULL) {
         size_t directory_length = (size_t)(last_separator - current_path);
         if (directory_length == 0) directory_length = 1;
         if (directory_length + 1u < sizeof(candidate)) {
            memcpy(candidate, current_path, directory_length);
            candidate[directory_length] = '\0';
            if (pp_join_path(candidate, candidate, name)) {
               int fd = pp_open_include_candidate(candidate, resolved_path);
               if (fd >= 0) return fd;
            }
         }
      } else {
         int fd = pp_open_include_candidate(name, resolved_path);
         if (fd >= 0) return fd;
      }
   }

   for (size_t i = 0; i < pp_include_path_count; i++) {
      int fd;
      if (pp_include_path_list[i] == NULL) continue;
      if (!pp_join_path(candidate, pp_include_path_list[i], name)) continue;
      fd = pp_open_include_candidate(candidate, resolved_path);
      if (fd >= 0) return fd;
   }
   return -1;
}

static bool pp_process_fd(int fd, const char* path, size_t depth);

static bool pp_process_include(
   const char* text,
   size_t length,
   const char* current_path,
   size_t source_line,
   size_t depth
) {
   char expanded[PP_PATH_LIMIT];
   char include_name[PP_PATH_LIMIT];
   char resolved_path[PP_PATH_LIMIT];
   size_t expansion_stack[PP_EXPANSION_LIMIT];
   struct pp_sink sink = { expanded, 0, sizeof(expanded) - 1u };
   size_t pos;
   size_t begin;
   char closing;
   int fd;

   pos = pp_skip_space(text, length, 0);
   if (pos < length && (text[pos] == '"' || text[pos] == '<')) {
      if (!pp_sink_write(&sink, text, length)) return false;
   } else if (!pp_expand_text(
      text, length, &sink, expansion_stack, 0, false, NULL, NULL, 0
   )) return false;
   expanded[sink.length] = '\0';
   pos = pp_skip_space(expanded, sink.length, 0);
   if (pos == sink.length) {
      pp_fail();
      return false;
   }
   if (expanded[pos] == '"') closing = '"';
   else if (expanded[pos] == '<') closing = '>';
   else {
      pp_fail();
      return false;
   }
   pos++;
   begin = pos;
   while (pos < sink.length && expanded[pos] != closing) pos++;
   if (pos == sink.length || pos - begin + 1u > sizeof(include_name)) {
      pp_fail();
      return false;
   }
   if (pos == begin) {
      pp_fail();
      return false;
   }
   memcpy(include_name, expanded + begin, pos - begin);
   include_name[pos - begin] = '\0';
   pos = pp_skip_space(expanded, sink.length, pos + 1u);
   if (pos != sink.length) {
      pp_fail();
      return false;
   }

   fd = pp_open_include(
      include_name, closing == '"', current_path, resolved_path
   );
   if (fd < 0) {
      pp_error_missing_include(current_path, source_line, include_name);
      pp_fail();
      return false;
   }
   return pp_process_fd(fd, resolved_path, depth + 1u);
}

static bool pp_current_active(
   const struct pp_condition* conditions, size_t condition_count
) {
   return condition_count == 0 || conditions[condition_count - 1].active;
}

static bool pp_directive_is(
   const char* line,
   size_t directive_begin,
   size_t directive_end,
   const char* expected,
   size_t expected_length
) {
   return pp_text_equal(
      line + directive_begin,
      directive_end - directive_begin,
      expected,
      expected_length
   );
}

static bool pp_emit_newlines(size_t count) {
   while (count-- != 0) {
      if (!pp_output_char('\n')) return false;
   }
   return true;
}

static bool pp_emit_line_marker(
   size_t line_number, const char* path, unsigned flag
) {
   char digits[3u * sizeof(size_t) + 1u];
   size_t digit_count = 0;

   if (pp_output_pos != 0 && output_mem[pp_output_pos - 1u] != '\n') {
      if (!pp_output_char('\n')) return false;
   }
   if (!pp_output_write("# ", 2)) return false;
   do {
      digits[digit_count++] = (char)('0' + line_number % 10u);
      line_number /= 10u;
   } while (line_number != 0);
   while (digit_count != 0) {
      if (!pp_output_char(digits[--digit_count])) return false;
   }
   if (!pp_output_write(" \"", 2)) return false;
   for (const char* p = path; *p != '\0'; p++) {
      if (*p == '"' || *p == '\r' || *p == '\n') {
         pp_fail();
         return false;
      }
      if (!pp_output_char(*p)) return false;
   }
   if (!pp_output_char('"')) return false;
   if (flag != 0) {
      if (!pp_output_char(' ')) return false;
      if (!pp_output_char((char)('0' + flag))) return false;
   }
   return pp_output_char('\n');
}

static bool pp_process_line(
   const char* line,
   size_t length,
   size_t newline_count,
   size_t source_line,
   const char* current_path,
   size_t depth,
   struct pp_condition* conditions,
   size_t* condition_count
) {
   size_t pos = pp_skip_space(line, length, 0);
   bool active = pp_current_active(conditions, *condition_count);
   size_t directive_begin;
   size_t directive_end;

   if (pos == length || line[pos] != '#') {
      if (active && !pp_expand_to_output(line, length)) return false;
      return pp_emit_newlines(newline_count);
   }

   pos = pp_skip_space(line, length, pos + 1u);
   directive_begin = pos;
   if (pos < length && pp_is_ident_start(line[pos])) {
      pos++;
      while (pos < length && pp_is_ident(line[pos])) pos++;
   }
   directive_end = pos;
   pos = pp_skip_space(line, length, pos);

   if (directive_begin == directive_end) {
      if (pos != length) {
         pp_fail();
         return false;
      }
      return pp_emit_newlines(newline_count);
   }

   if (pp_directive_is(line, directive_begin, directive_end, "if", 2)
      || pp_directive_is(line, directive_begin, directive_end, "ifdef", 5)
      || pp_directive_is(line, directive_begin, directive_end, "ifndef", 6)) {
      struct pp_condition* condition;
      bool result = false;
      bool is_if = pp_directive_is(
         line, directive_begin, directive_end, "if", 2
      );
      bool is_ifndef = pp_directive_is(
         line, directive_begin, directive_end, "ifndef", 6
      );
      if (*condition_count == PP_CONDITION_LIMIT) {
         pp_fail();
         return false;
      }
      if (active) {
         if (is_if) {
            result = pp_evaluate_condition(line + pos, length - pos);
         } else {
            size_t name_begin = pos;
            size_t name_end;
            if (name_begin == length || !pp_is_ident_start(line[name_begin])) {
               pp_fail();
               return false;
            }
            name_end = name_begin + 1u;
            while (name_end < length && pp_is_ident(line[name_end])) name_end++;
            if (pp_skip_space(line, length, name_end) != length) {
               pp_fail();
               return false;
            }
            result = pp_macro_is_defined(line + name_begin, name_end - name_begin);
            if (is_ifndef) result = !result;
         }
      }
      condition = &conditions[(*condition_count)++];
      condition->parent_active = active;
      condition->active = active && result;
      condition->branch_taken = condition->active;
      condition->saw_else = false;
      return pp_emit_newlines(newline_count);
   }

   if (pp_directive_is(line, directive_begin, directive_end, "elif", 4)) {
      struct pp_condition* condition;
      bool take = false;
      if (*condition_count == 0) {
         pp_fail();
         return false;
      }
      condition = &conditions[*condition_count - 1u];
      if (condition->saw_else) {
         pp_fail();
         return false;
      }
      if (condition->parent_active && !condition->branch_taken) {
         take = pp_evaluate_condition(line + pos, length - pos);
      }
      condition->active = condition->parent_active
         && !condition->branch_taken && take;
      if (condition->active) condition->branch_taken = true;
      return pp_emit_newlines(newline_count);
   }

   if (pp_directive_is(line, directive_begin, directive_end, "else", 4)) {
      struct pp_condition* condition;
      if (*condition_count == 0 || pos != length) {
         pp_fail();
         return false;
      }
      condition = &conditions[*condition_count - 1u];
      if (condition->saw_else) {
         pp_fail();
         return false;
      }
      condition->saw_else = true;
      condition->active = condition->parent_active && !condition->branch_taken;
      if (condition->active) condition->branch_taken = true;
      return pp_emit_newlines(newline_count);
   }

   if (pp_directive_is(line, directive_begin, directive_end, "endif", 5)) {
      if (*condition_count == 0 || pos != length) {
         pp_fail();
         return false;
      }
      (*condition_count)--;
      return pp_emit_newlines(newline_count);
   }

   if (!active) return pp_emit_newlines(newline_count);

   if (pp_directive_is(line, directive_begin, directive_end, "define", 6)) {
      size_t name_begin = pos;
      size_t name_end;
      size_t value_begin;
      size_t value_end;
      bool function_like;
      if (name_begin == length || !pp_is_ident_start(line[name_begin])) {
         pp_fail();
         return false;
      }
      name_end = name_begin + 1u;
      while (name_end < length && pp_is_ident(line[name_end])) name_end++;
      function_like = name_end < length && line[name_end] == '(';
      if (function_like) {
         size_t parameter_pos = name_end + 1u;
         while (parameter_pos < length && line[parameter_pos] != ')') {
            if (parameter_pos + 2u < length
               && line[parameter_pos] == '.'
               && line[parameter_pos + 1u] == '.'
               && line[parameter_pos + 2u] == '.') {
               pp_error_variadic_macro(
                  current_path,
                  source_line,
                  line + name_begin,
                  name_end - name_begin
               );
               pp_fail();
               return false;
            }
            parameter_pos++;
         }
      }
      value_begin = pp_skip_space(line, length, name_end);
      value_end = pp_trim_end(line, value_begin, length);
      if (!pp_set_macro(
         line + name_begin,
         name_end - name_begin,
         line + value_begin,
         value_end - value_begin,
         function_like
      )) return false;
      return pp_emit_newlines(newline_count);
   }

   if (pp_directive_is(line, directive_begin, directive_end, "undef", 5)) {
      size_t name_begin = pos;
      size_t name_end;
      if (name_begin == length || !pp_is_ident_start(line[name_begin])) {
         pp_fail();
         return false;
      }
      name_end = name_begin + 1u;
      while (name_end < length && pp_is_ident(line[name_end])) name_end++;
      if (pp_skip_space(line, length, name_end) != length) {
         pp_fail();
         return false;
      }
      pp_undef_macro(line + name_begin, name_end - name_begin);
      return pp_emit_newlines(newline_count);
   }

   if (pp_directive_is(line, directive_begin, directive_end, "include", 7)) {
      size_t resume_line = source_line
         + (newline_count == 0 ? 1u : newline_count);
      if (!pp_process_include(
         line + pos, length - pos, current_path, source_line, depth
      )) return false;
      return pp_emit_line_marker(resume_line, current_path, 2);
   }

   /* Leave unsupported active directives for the next compiler stage. */
   if (!pp_output_write(line, length)) return false;
   return pp_emit_newlines(newline_count);
}

static bool pp_line_char(char* line, size_t* length, char c) {
   if (*length == PP_LINE_LIMIT - 1u) {
      pp_fail();
      return false;
   }
   line[(*length)++] = c;
   return true;
}

static bool pp_consume_source_newline(
   const char* input, size_t input_length, size_t* input_pos
) {
   if (*input_pos >= input_length) return false;
   if (input[*input_pos] == '\n') {
      (*input_pos)++;
      return true;
   }
   if (input[*input_pos] == '\r') {
      (*input_pos)++;
      if (*input_pos < input_length && input[*input_pos] == '\n') {
         (*input_pos)++;
      }
      return true;
   }
   return false;
}

static char pp_source_char(
   const char* input, size_t input_length, size_t* input_pos
) {
   char c = input[(*input_pos)++];
   if (c == '\r') {
      if (*input_pos < input_length && input[*input_pos] == '\n') {
         (*input_pos)++;
      }
      return '\n';
   }
   return c;
}

static bool pp_process_buffer(
   const char* input, size_t input_length, const char* path, size_t depth
) {
   struct pp_condition conditions[PP_CONDITION_LIMIT];
   size_t condition_count = 0;
   size_t input_pos = 0;
   size_t source_line = 1;
   bool block_comment = false;

   while (input_pos < input_length && !pp_failed) {
      char line[PP_LINE_LIMIT];
      size_t line_length = 0;
      size_t newline_count = 0;
      char quote = 0;
      bool escaped = false;
      bool line_done = false;

      while (input_pos < input_length && !line_done && !pp_failed) {
         char c = pp_source_char(input, input_length, &input_pos);

         /* Translation phase two: remove backslash-newline pairs before
          * recognizing comments or directives. */
         if (c == '\\' && pp_consume_source_newline(
            input, input_length, &input_pos
         )) {
            newline_count++;
            continue;
         }

         if (block_comment) {
            if (c == '*' && input_pos < input_length && input[input_pos] == '/') {
               input_pos++;
               block_comment = false;
            } else if (c == '\n') {
               newline_count++;
               line_done = true;
            }
            continue;
         }

         if (quote != 0) {
            if (c == '\n') {
               newline_count++;
               line_done = true;
               continue;
            }
            if (!pp_line_char(line, &line_length, c)) return false;
            if (escaped) {
               escaped = false;
            } else if (c == '\\') {
               escaped = true;
            } else if (c == quote) {
               quote = 0;
            }
            continue;
         }

         if (c == '/' && input_pos < input_length && input[input_pos] == '*') {
            input_pos++;
            block_comment = true;
            if (!pp_line_char(line, &line_length, ' ')) return false;
            continue;
         }
         if (c == '/' && input_pos < input_length && input[input_pos] == '/') {
            input_pos++;
            while (input_pos < input_length) {
               c = pp_source_char(input, input_length, &input_pos);
               if (c == '\\' && pp_consume_source_newline(
                  input, input_length, &input_pos
               )) {
                  newline_count++;
                  continue;
               }
               if (c == '\n') {
                  newline_count++;
                  break;
               }
            }
            line_done = true;
            continue;
         }
         if (c == '\n') {
            newline_count++;
            line_done = true;
            continue;
         }
         if (c == '"' || c == '\'') quote = c;
         if (!pp_line_char(line, &line_length, c)) return false;
      }

      if (!pp_process_line(
         line,
         line_length,
         newline_count,
         source_line,
         path,
         depth,
         conditions,
         &condition_count
      )) return false;
      source_line += newline_count;
   }

   if (block_comment || condition_count != 0) {
      pp_fail();
      return false;
   }
   return true;
}

static bool pp_process_fd(int fd, const char* path, size_t depth) {
   size_t base = pp_file_mem_pos;
   size_t length = 0;
   bool result;

   if (depth > PP_INCLUDE_DEPTH_LIMIT) {
      close(fd);
      pp_fail();
      return false;
   }

   for (;;) {
      size_t remaining = INPUT_LIMIT - base - length;
      size_t request;
      int bytes_read;
      if (remaining <= 1u) {
         char overflow_byte;
         do {
            bytes_read = (int)read(fd, &overflow_byte, 1);
         } while (bytes_read < 0 && errno == EINTR);
         if (bytes_read != 0) pp_fail();
         break;
      }
      request = remaining - 1u;
      if (request > 1024u * 1024u) request = 1024u * 1024u;
      do {
         bytes_read = (int)read(fd, input_mem + base + length, request);
      } while (bytes_read < 0 && errno == EINTR);
      if (bytes_read < 0) {
         pp_fail();
         break;
      }
      if (bytes_read == 0) break;
      length += (size_t)bytes_read;
   }
   close(fd);
   if (pp_failed) {
      pp_file_mem_pos = base;
      return false;
   }

   input_mem[base + length] = '\0';
   pp_file_mem_pos = base + length + 1u;
   result = pp_emit_line_marker(1, path, depth == 0 ? 0 : 1)
      && pp_process_buffer(input_mem + base, length, path, depth);
   pp_file_mem_pos = base;
   return result;
}

/*
 * Preprocess input_path into output_mem and return the byte count, excluding
 * output_mem's trailing zero byte. include_path_list, define_name and
 * define_value are parallel arrays. A null define value is equivalent to 1.
 * GCC-style line markers preserve source paths and line numbers across nested
 * includes.
 *
 * Object-like and function-like macros are recursively expanded. Function
 * macros support ordinary parameter substitution; stringizing (#), token
 * pasting (##), and variadic parameters are not supported. On an I/O, syntax,
 * nesting, or fixed-buffer-limit error, this function returns zero and clears
 * output_mem.
 */
size_t preprocess(
   const char* input_path,
   size_t include_path_count,
   const char* const include_path_list[],
   size_t define_count,
   const char* const define_name[],
   const char* const define_value[]
) {
   int fd;

   pp_macro_count = 0;
   pp_macro_mem_pos = 0;
   pp_file_mem_pos = PP_MACRO_STORAGE_LIMIT;
   pp_output_pos = 0;
   pp_failed = false;
   pp_error_length = 0;
   pp_error_mem[0] = '\0';
   pp_include_path_count = include_path_count;
   pp_include_path_list = include_path_list;
   if (OUTPUT_LIMIT != 0) output_mem[0] = '\0';

   if (input_path == NULL || OUTPUT_LIMIT == 0
      || (include_path_count != 0 && include_path_list == NULL)
      || (define_count != 0 && define_name == NULL)) {
      pp_fail();
   }

   for (size_t i = 0; i < define_count && !pp_failed; i++) {
      const char* value;
      if (define_name[i] == NULL) {
         pp_fail();
         break;
      }
      value = define_value == NULL || define_value[i] == NULL
         ? "1" : define_value[i];
      pp_set_macro(
         define_name[i], strlen(define_name[i]), value, strlen(value), false
      );
   }

   if (!pp_failed) {
      fd = pp_open_read(input_path);
      if (fd < 0) pp_fail();
      else pp_process_fd(fd, input_path, 0);
   }

   if (pp_failed) {
      pp_output_pos = 0;
      if (OUTPUT_LIMIT != 0) output_mem[0] = '\0';
      if (pp_error_length == 0) {
         pp_error_append(
            "error: preprocessing failed\n",
            sizeof("error: preprocessing failed\n") - 1u
         );
      }
      return 0;
   }
   output_mem[pp_output_pos] = '\0';
   return pp_output_pos;
}
