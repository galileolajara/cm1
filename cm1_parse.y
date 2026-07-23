%include {
char string_mem[1];
void stdout_then_print_error(char*, uint32_t);
#define PATH(t) (void*)&t.ptr.path
#define CM1_ARRAY_DIM_STACK_LIMIT 256
static uint32_t cm1_array_dim_stack[CM1_ARRAY_DIM_STACK_LIMIT];
static uint32_t cm1_array_dim_stack_pos;
void _Tcm1_Fclear_memory_parser_0() {
   cm1_array_dim_stack_pos = 0;
}
static void cm1_array_dim_push(uint32_t dim) {
   if (cm1_array_dim_stack_pos == CM1_ARRAY_DIM_STACK_LIMIT) {
      static char error[] = "Too many dimensions in array declarator\n";
      stdout_then_print_error(error, sizeof(error) - 1);
      exit(EXIT_FAILURE);
   }
   cm1_array_dim_stack[cm1_array_dim_stack_pos++] = dim;
}
static void cm1_array_const_error(const char *path, uint32_t row, uint32_t col,
                                  const char *message) {
   char error[1024];
   int len = sprintf(error, "%s:%u:%u: %s\n", path, row, col, message);
   stdout_then_print_error(error, len);
   exit(EXIT_FAILURE);
}
static int64_t cm1_array_const_add(int64_t lhs, int64_t rhs,
                                   const union _Tcm1_Ttoken_data *token) {
   if ((rhs > 0 && lhs > INT64_MAX - rhs) ||
       (rhs < 0 && lhs < INT64_MIN - rhs)) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Integer overflow in constant expression");
   }
   return lhs + rhs;
}
static int64_t cm1_array_const_sub(int64_t lhs, int64_t rhs,
                                   const union _Tcm1_Ttoken_data *token) {
   if ((rhs < 0 && lhs > INT64_MAX + rhs) ||
       (rhs > 0 && lhs < INT64_MIN + rhs)) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Integer overflow in constant expression");
   }
   return lhs - rhs;
}
static int64_t cm1_array_const_mul(int64_t lhs, int64_t rhs,
                                   const union _Tcm1_Ttoken_data *token) {
   if (lhs != 0 &&
       ((lhs == -1 && rhs == INT64_MIN) ||
        (rhs == -1 && lhs == INT64_MIN) ||
        (lhs > 0 && rhs > 0 && lhs > INT64_MAX / rhs) ||
        (lhs > 0 && rhs < 0 && rhs < INT64_MIN / lhs) ||
        (lhs < 0 && rhs > 0 && lhs < INT64_MIN / rhs) ||
        (lhs < 0 && rhs < 0 && lhs < INT64_MAX / rhs))) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Integer overflow in constant expression");
   }
   return lhs * rhs;
}
static int64_t cm1_array_const_div(int64_t lhs, int64_t rhs,
                                   const union _Tcm1_Ttoken_data *token) {
   if (rhs == 0) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Division by zero in constant expression");
   }
   if (lhs == INT64_MIN && rhs == -1) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Integer overflow in constant expression");
   }
   return lhs / rhs;
}
static int64_t cm1_array_const_shift(int64_t lhs, int64_t rhs, bool left,
                                     const union _Tcm1_Ttoken_data *token) {
   if (lhs < 0 || rhs < 0 || rhs >= 32) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Integer constant shifts require a nonnegative value and a shift from 0 to 31");
   }
   if (left && (uint64_t)lhs > (UINT32_MAX >> rhs)) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Integer overflow in constant expression");
   }
   return left ? (int64_t)((uint32_t)lhs << rhs) : (lhs >> rhs);
}
static uint32_t cm1_array_const_dimension(
   int64_t value, const union _Tcm1_Ttoken_data *token) {
   if (value <= 0) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Array length must be greater than zero");
   }
   if ((uint64_t)value > UINT32_MAX) {
      cm1_array_const_error(token->u64.path, token->u64.row, token->u64.col,
                            "Array length is too large");
   }
   return (uint32_t)value;
}
#define ARRAY_CONST_LOCATION(out, in) do { \
   (out).u64.path = (in).u64.path; \
   (out).u64.row = (in).u64.row; \
   (out).u64.col = (in).u64.col; \
} while (0)
#define ARRAY_DIM_START(a) ((uint32_t)(a).basic.id)
#define ARRAY_DIM_V(a) (&cm1_array_dim_stack[ARRAY_DIM_START(a)])
#define ARRAY_DIM_C(a) ((uint32_t)(a).basic.id2)
#define ARRAY_DIM_POP(a) (cm1_array_dim_stack_pos = ARRAY_DIM_START(a))
}
%name cm1Parse
%token_prefix CM1_TOKEN_
%stack_size 1024

%token_type {union _Tcm1_Ttoken_data}

%syntax_error{
   int n = YYNTOKEN;
   bool first = true;
   uint8_t first_expect = 0;
   uint8_t second_expect = 0;
   char errbuf[1024];
   int ibuf;
   for (int i = 0; i < n; ++i) {
      int a = yy_find_shift_action((YYCODETYPE)i, yypParser->yytos->stateno);
      if (a != YY_ERROR_ACTION) {
         if (first) {
            first = false;
            first_expect = i;
            if (_Tcm1_Glast_token == CM1_TOKEN_END) {
               if (string_mem[0] == 0) {
                  ibuf = sprintf(errbuf, "%s:%u:%u: syntax error, got #end-of-file but expected tokens are: #%s", input_path, _Tcm1_Grow, _Tcm1_Gcol, _Tcm1_Ftoken_name_1(i));
               } else {
                  ibuf = sprintf(errbuf, "%s:%u:%u: syntax error, got token '%c' but expected tokens are: #%s", input_path, _Tcm1_Grow, _Tcm1_Gcol, string_mem[0], _Tcm1_Ftoken_name_1(i));
               }
            } else {
               ibuf = sprintf(errbuf, "%s:%u:%u: syntax error, got token #%s but expected tokens are: #%s", input_path, _Tcm1_Grow, _Tcm1_Gcol, _Tcm1_Ftoken_name_1(_Tcm1_Glast_token), _Tcm1_Ftoken_name_1(i));
            }
         } else {
            if (second_expect == 0) {
               second_expect = i;
            }
            ibuf += sprintf(errbuf + ibuf, ", #%s", _Tcm1_Ftoken_name_1(i));
         }
      }
   }
   if (!first) {
      errbuf[ibuf++] = '\n';
      stdout_then_print_error(errbuf, ibuf);
   }
   exit(EXIT_FAILURE);
   return;
}

cm1 ::= decls END.
cm1 ::= END.

decl ::= decl_func_c.
decl ::= decl_func_cm1.
decl ::= decl_gvar.
decl ::= decl_aggregate.
decl ::= decl_enum.
decl ::= decl_typedef.
decl ::= decl_preprocessor.

decl_preprocessor ::= PREPROCESSOR(t).
{ _Tcm1_Fdecl_preprocessor_2(t.ptr.ptr, (size_t)t.ptr.ptr2); }

decl_typedef_type ::= TYPE(t).
{ _Tcm1_Ftypedef_begin_1(t.ptr.ptr); }
decl_typedef_type ::= aggregate_space_id(t).
{ _Tcm1_Ftypedef_begin_1(t.ptr.ptr); }
decl_typedef_type ::= enum_space_id(t).
{ _Tcm1_Ftypedef_begin_1(t.ptr.ptr); }
decl_typedef ::= TYPEDEF decl_typedef_type typedef_ids SEMICOLON.
field_declarator_first ::= type(t) ID(i) array_dims(a).
{
   _Tcm1_Fdeclarator_begin_2(t.ptr.ptr, (size_t)t.ptr.ptr3 == 1);
   _Tcm1_Fdeclarator_field_5((size_t)t.ptr.ptr2, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), PATH(i));
   ARRAY_DIM_POP(a);
}
field_declarator ::= COMMA stars_for_ids(s) ID(i) array_dims(a).
{
   _Tcm1_Fdeclarator_field_5(s.basic.id, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), PATH(i));
   ARRAY_DIM_POP(a);
}
field_declarators ::= .
field_declarators ::= field_declarators field_declarator.
field ::= field_declarator_first field_declarators SEMICOLON.
fields ::= field.
fields ::= fields field.
aggregate_body ::= CLOSE_CURLY_BRACE.
aggregate_body ::= fields CLOSE_CURLY_BRACE.
aggregate_body_end ::= aggregate_body.
{ _Tcm1_Faggregate_end_0(); }

aggregate_space_id ::= STRUCT_SPACE_ID.
aggregate_space_id ::= UNION_SPACE_ID.

aggregate_keyword(out) ::= STRUCT(s).
{
   out.basic.path = s.basic.path;
   out.basic.row = s.basic.row;
   out.basic.col = s.basic.col;
   out.ptr.ptr = NULL;
   out.ptr.ptr2 = NULL;
}
aggregate_keyword(out) ::= UNION(u).
{
   out.basic.path = u.basic.path;
   out.basic.row = u.basic.row;
   out.basic.col = u.basic.col;
   out.ptr.ptr = NULL;
   out.ptr.ptr2 = (void*)1;
}
aggregate_keyword(out) ::= STRUCT_SPACE_ID(s).
{
   out.basic.path = s.basic.path;
   out.basic.row = s.basic.row;
   out.basic.col = s.basic.col;
   out.ptr.ptr = s.ptr.ptr;
   out.ptr.ptr2 = NULL;
}
aggregate_keyword(out) ::= UNION_SPACE_ID(u).
{
   out.basic.path = u.basic.path;
   out.basic.row = u.basic.row;
   out.basic.col = u.basic.col;
   out.ptr.ptr = u.ptr.ptr;
   out.ptr.ptr2 = (void*)1;
}

decl_aggregate_begin ::= aggregate_space_id(i).
{ _Tcm1_Faggregate_begin_4(i.ptr.ptr, 0, PATH(i), 0); }
aggregate_gvar ::= stars_for_ids(s) ID(i) array_dims(a).
{
   _Tcm1_Fgvar_decl_7(current_type, s.basic.id, 0, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), PATH(i));
   ARRAY_DIM_POP(a);
}
aggregate_gvars ::= aggregate_gvar.
aggregate_gvars ::= aggregate_gvars COMMA aggregate_gvar.
aggregate_gvars_or_none ::= .
aggregate_gvars_or_none ::= aggregate_gvars.
decl_aggregate ::= decl_aggregate_begin OPEN_CURLY_BRACE aggregate_body_end aggregate_gvars_or_none SEMICOLON.
decl_aggregate ::= aggregate_space_id(i) SEMICOLON.
{ _Tcm1_Faggregate_forward_1(i.ptr.ptr); }
typedef_ids ::= stars_for_ids(s) ID(i).
{ _Tcm1_Ftypedef_name_3(i.basic.id, s.basic.id, PATH(i)); }
typedef_ids ::= stars_for_ids TYPE.
typedef_ids ::= typedef_ids COMMA stars_for_ids(s) ID(i).
{ _Tcm1_Ftypedef_name_3(i.basic.id, s.basic.id, PATH(i)); }
typedef_ids ::= typedef_ids COMMA stars_for_ids TYPE.
decl_aggregate_typedef_begin ::= aggregate_keyword(k).
{ _Tcm1_Faggregate_begin_4(k.ptr.ptr, (size_t)k.ptr.ptr2, PATH(k), 1); }
decl_aggregate ::= TYPEDEF decl_aggregate_typedef_begin OPEN_CURLY_BRACE aggregate_body typedef_ids SEMICOLON.
{ _Tcm1_Faggregate_end_0(); }

enum_space_id ::= ENUM_SPACE_ID.

enum_keyword(out) ::= ENUM(e).
{
   out.basic.path = e.basic.path;
   out.basic.row = e.basic.row;
   out.basic.col = e.basic.col;
   out.ptr.ptr = NULL;
}
enum_keyword(out) ::= ENUM_SPACE_ID(e).
{
   out.basic.path = e.basic.path;
   out.basic.row = e.basic.row;
   out.basic.col = e.basic.col;
   out.ptr.ptr = e.ptr.ptr;
}

enum_member ::= ID(i).
{ _Tcm1_Fenum_member_4(i.basic.id, 0, 0, PATH(i)); }
enum_member ::= ID(i) ASSIGN array_const_expr(e).
{ _Tcm1_Fenum_member_4(i.basic.id, (int64_t)e.u64.u64, 1, PATH(i)); }
enum_members ::= enum_member.
enum_members ::= enum_members COMMA enum_member.
enum_body ::= enum_members CLOSE_CURLY_BRACE.
enum_body ::= enum_members COMMA CLOSE_CURLY_BRACE.
enum_body_end ::= enum_body.
{ _Tcm1_Fenum_end_0(); }

decl_enum_begin ::= enum_space_id(e).
{ _Tcm1_Fenum_begin_3(e.ptr.ptr, 0, PATH(e)); }
decl_enum ::= decl_enum_begin OPEN_CURLY_BRACE enum_body_end aggregate_gvars_or_none SEMICOLON.

decl_enum_typedef_begin ::= enum_keyword(e).
{ _Tcm1_Fenum_begin_3(e.ptr.ptr, 1, PATH(e)); }
decl_enum ::= TYPEDEF decl_enum_typedef_begin OPEN_CURLY_BRACE enum_body_end typedef_ids SEMICOLON.

type_or_aggregate ::= TYPE.
type_or_aggregate ::= INT64.
type_or_aggregate ::= INT32.
type_or_aggregate ::= INT16.
type_or_aggregate ::= INT8.
type_or_aggregate ::= UINT64.
type_or_aggregate ::= UINT32.
type_or_aggregate ::= UINT16.
type_or_aggregate ::= UINT8.
type_or_aggregate ::= F32.
type_or_aggregate ::= F64.
type_or_aggregate ::= VOID.
type_or_aggregate ::= aggregate_space_id.
type_or_aggregate ::= enum_space_id.
type(out) ::= type_or_aggregate(t) stars_or_none(s).
{
   out.ptr.path = t.ptr.path;
   out.ptr.row = t.ptr.row;
   out.ptr.col = t.ptr.col;
   out.ptr.ptr = t.ptr.ptr;
   out.ptr.ptr2 = (void*)(size_t)s.basic.id;
   out.ptr.ptr3 = NULL;
}
type(out) ::= EXTERN type_or_aggregate(t) stars_or_none(s).
{
   out.ptr.path = t.ptr.path;
   out.ptr.row = t.ptr.row;
   out.ptr.col = t.ptr.col;
   out.ptr.ptr = t.ptr.ptr;
   out.ptr.ptr2 = (void*)(size_t)s.basic.id;
   out.ptr.ptr3 = (void*)1;
}

gvar_declarator_first ::= type(t) ID(i) array_dims(a).
{
   _Tcm1_Fdeclarator_begin_2(t.ptr.ptr, (size_t)t.ptr.ptr3 == 1);
   _Tcm1_Fdeclarator_gvar_5((size_t)t.ptr.ptr2, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), PATH(i));
   ARRAY_DIM_POP(a);
}
gvar_declarator ::= COMMA stars_for_ids(s) ID(i) array_dims(a).
{
   _Tcm1_Fdeclarator_gvar_5(s.basic.id, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), PATH(i));
   ARRAY_DIM_POP(a);
}
gvar_declarators ::= .
gvar_declarators ::= gvar_declarators gvar_declarator.
decl_gvar ::= gvar_declarator_first gvar_declarators SEMICOLON.

lvar_declarator_first ::= type(t) ID(i) array_dims(a).
{
   _Tcm1_Fdeclarator_begin_2(t.ptr.ptr, (size_t)t.ptr.ptr3 == 1);
   _Tcm1_Fdeclarator_lvar_6((size_t)t.ptr.ptr2, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), NULL, PATH(i));
   ARRAY_DIM_POP(a);
}
lvar_declarator_first ::= type(t) ID(i) array_dims(a) ASSIGN expr(e).
{
   _Tcm1_Fdeclarator_begin_2(t.ptr.ptr, (size_t)t.ptr.ptr3 == 1);
   _Tcm1_Fdeclarator_lvar_6((size_t)t.ptr.ptr2, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), e.ptr.ptr, PATH(i));
   ARRAY_DIM_POP(a);
}
lvar_declarator_first ::= type(t) ID(i) array_dims(a) ASSIGN initializer.
{
   void *src = _Tcm1_Fexpr_initializer_final_4(
      t.ptr.ptr, (size_t)t.ptr.ptr2, ARRAY_DIM_C(a), PATH(i));
   _Tcm1_Fdeclarator_begin_2(t.ptr.ptr, (size_t)t.ptr.ptr3 == 1);
   _Tcm1_Fdeclarator_lvar_6((size_t)t.ptr.ptr2, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), src, PATH(i));
   ARRAY_DIM_POP(a);
}
lvar_declarator ::= COMMA stars_for_ids(s) ID(i) array_dims(a).
{
   _Tcm1_Fdeclarator_lvar_6(s.basic.id, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), NULL, PATH(i));
   ARRAY_DIM_POP(a);
}
lvar_declarator ::= COMMA stars_for_ids(s) ID(i) array_dims(a) ASSIGN expr(e).
{
   _Tcm1_Fdeclarator_lvar_6(s.basic.id, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), e.ptr.ptr, PATH(i));
   ARRAY_DIM_POP(a);
}
lvar_declarator ::= COMMA stars_for_ids(s) ID(i) array_dims(a) ASSIGN initializer.
{
   void *src = _Tcm1_Fdeclarator_initializer_3(
      s.basic.id, ARRAY_DIM_C(a), PATH(i));
   _Tcm1_Fdeclarator_lvar_6(s.basic.id, i.basic.id, ARRAY_DIM_V(a), ARRAY_DIM_C(a), src, PATH(i));
   ARRAY_DIM_POP(a);
}
lvar_declarators ::= .
lvar_declarators ::= lvar_declarators lvar_declarator.
stmt_var ::= lvar_declarator_first lvar_declarators SEMICOLON.

initializer_begin ::= OPEN_CURLY_BRACE(b).
{ _Tcm1_Fexpr_initializer_begin_1(PATH(b)); }
initializer_values ::= expr_assign(e).
{ _Tcm1_Fexpr_initializer_arg_1(e.ptr.ptr); }
initializer_values ::= initializer_values COMMA expr_assign(e).
{ _Tcm1_Fexpr_initializer_arg_1(e.ptr.ptr); }
initializer_contents ::= .
initializer_contents ::= initializer_values.
initializer_contents ::= initializer_values COMMA.
initializer ::= initializer_begin initializer_contents CLOSE_CURLY_BRACE.

decl_func_args_list ::= rparen_or_curly.
decl_func_args_list ::= VOID rparen_or_curly.
decl_func_args_list ::= decl_func_args rparen_or_curly.

rparen_or_curly ::= RPAREN.
rparen_or_curly ::= RPAREN_THEN_CURLY.

decl_func_arg ::= type(t) ID(i) func_arg_brackets(a).
{ _Tcm1_Ffunc_decl_arg_4(t.ptr.ptr, (size_t)t.ptr.ptr2 + a.basic.id, i.basic.id, PATH(t)); }
decl_func_args ::= decl_func_arg.
decl_func_args ::= decl_func_args COMMA decl_func_arg.

func_arg_brackets(out) ::= .
{ out.basic.id = 0; }
func_arg_brackets(out) ::= func_arg_brackets(a) LBRACKET RBRACKET.
{ out.basic.id = a.basic.id + 1; }
func_arg_brackets(out) ::= func_arg_brackets(a) LBRACKET array_const_expr RBRACKET.
{ out.basic.id = a.basic.id + 1; }

stars(out) ::= STAR.
{ out.basic.id = 1; }

stars(out) ::= stars(s) STAR.
{ out.basic.id = s.basic.id + 1; }

stars_or_none(out) ::= .
{ out.basic.id = 0; }
stars_or_none(out) ::= stars(s).
{ out.basic.id = s.basic.id; }

stars_for_ids(out) ::= .
{ out.basic.id = 0; }
stars_for_ids(out) ::= stars(s).
{ out.basic.id = s.basic.id; }

array_dims(out) ::= .
{
   out.basic.id = cm1_array_dim_stack_pos;
   out.basic.id2 = 0;
}
array_dims(out) ::= array_dims(a) LBRACKET array_const_expr(e) RBRACKET.
{
   cm1_array_dim_push(cm1_array_const_dimension((int64_t)e.u64.u64, &e));
   out.basic.id = a.basic.id;
   out.basic.id2 = a.basic.id2 + 1;
}

/*
Array lengths deliberately use a separate constant-expression grammar.  This
keeps names, calls, floating-point literals, assignments, and other runtime
expressions out while preserving the usual C precedence of the supported
integer operators.
*/
array_const_expr(out) ::= array_const_and_expr(e).
{
   ARRAY_CONST_LOCATION(out, e);
   out.u64.u64 = e.u64.u64;
}

array_const_and_expr(out) ::= array_const_shift_expr(e).
{
   ARRAY_CONST_LOCATION(out, e);
   out.u64.u64 = e.u64.u64;
}
array_const_and_expr(out) ::= array_const_and_expr(lhs) AMP array_const_shift_expr(rhs).
{
   ARRAY_CONST_LOCATION(out, lhs);
   out.u64.u64 = (uint64_t)((int64_t)lhs.u64.u64 & (int64_t)rhs.u64.u64);
}

array_const_shift_expr(out) ::= array_const_add_expr(e).
{
   ARRAY_CONST_LOCATION(out, e);
   out.u64.u64 = e.u64.u64;
}
array_const_shift_expr(out) ::= array_const_shift_expr(lhs) SHL array_const_add_expr(rhs).
{
   ARRAY_CONST_LOCATION(out, lhs);
   out.u64.u64 = (uint64_t)cm1_array_const_shift(
      (int64_t)lhs.u64.u64, (int64_t)rhs.u64.u64, true, &lhs);
}
array_const_shift_expr(out) ::= array_const_shift_expr(lhs) SHR array_const_add_expr(rhs).
{
   ARRAY_CONST_LOCATION(out, lhs);
   out.u64.u64 = (uint64_t)cm1_array_const_shift(
      (int64_t)lhs.u64.u64, (int64_t)rhs.u64.u64, false, &lhs);
}

array_const_add_expr(out) ::= array_const_mul_expr(e).
{
   ARRAY_CONST_LOCATION(out, e);
   out.u64.u64 = e.u64.u64;
}
array_const_add_expr(out) ::= array_const_add_expr(lhs) PLUS array_const_mul_expr(rhs).
{
   ARRAY_CONST_LOCATION(out, lhs);
   out.u64.u64 = (uint64_t)cm1_array_const_add(
      (int64_t)lhs.u64.u64, (int64_t)rhs.u64.u64, &lhs);
}
array_const_add_expr(out) ::= array_const_add_expr(lhs) MINUS array_const_mul_expr(rhs).
{
   ARRAY_CONST_LOCATION(out, lhs);
   out.u64.u64 = (uint64_t)cm1_array_const_sub(
      (int64_t)lhs.u64.u64, (int64_t)rhs.u64.u64, &lhs);
}

array_const_mul_expr(out) ::= array_const_primary(e).
{
   ARRAY_CONST_LOCATION(out, e);
   out.u64.u64 = e.u64.u64;
}
array_const_mul_expr(out) ::= array_const_mul_expr(lhs) STAR array_const_primary(rhs).
{
   ARRAY_CONST_LOCATION(out, lhs);
   out.u64.u64 = (uint64_t)cm1_array_const_mul(
      (int64_t)lhs.u64.u64, (int64_t)rhs.u64.u64, &lhs);
}
array_const_mul_expr(out) ::= array_const_mul_expr(lhs) SLASH array_const_primary(rhs).
{
   ARRAY_CONST_LOCATION(out, lhs);
   out.u64.u64 = (uint64_t)cm1_array_const_div(
      (int64_t)lhs.u64.u64, (int64_t)rhs.u64.u64, &lhs);
}

array_const_primary(out) ::= ZERO(z).
{
   ARRAY_CONST_LOCATION(out, z);
   out.u64.u64 = 0;
}
array_const_primary(out) ::= I32(i).
{
   ARRAY_CONST_LOCATION(out, i);
   out.u64.u64 = (uint32_t)i.basic.id;
}
array_const_primary(out) ::= U32(i).
{
   ARRAY_CONST_LOCATION(out, i);
   out.u64.u64 = i.u64.u64;
}
array_const_primary(out) ::= I64(i).
{
   ARRAY_CONST_LOCATION(out, i);
   out.u64.u64 = i.u64.u64;
}
array_const_primary(out) ::= U64(i).
{
   ARRAY_CONST_LOCATION(out, i);
   out.u64.u64 = i.u64.u64;
}
array_const_primary(out) ::= SIZEOF LPAREN type(t) RPAREN.
{
   ARRAY_CONST_LOCATION(out, t);
   out.u64.u64 = _Tcm1_Farray_const_sizeof_3(
      t.ptr.ptr, (size_t)t.ptr.ptr2, PATH(t));
}
array_const_primary(out) ::= LPAREN array_const_expr(e) RPAREN.
{
   ARRAY_CONST_LOCATION(out, e);
   out.u64.u64 = e.u64.u64;
}

decl_func_begin_c ::= type(t) FUNC_C.
{ _Tcm1_Ffunc_decl_4(t.ptr.ptr, (size_t)t.ptr.ptr2, PATH(t), (size_t)t.ptr.ptr3 == 1); }
decl_func_c ::= decl_func_begin_c decl_func_args_list C_CODE(c).
{ _Tcm1_Ffunc_code_c_2(c.ptr.ptr, c.ptr.ptr2 - c.ptr.ptr); }
decl_func_c ::= decl_func_begin_c decl_func_args_list SEMICOLON.
{ _Tcm1_Ffunc_decl_0(); }

decl_func_begin_cm1 ::= type(t) FUNC_BC.
{ _Tcm1_Ffunc_decl_4(t.ptr.ptr, (size_t)t.ptr.ptr2, PATH(t), (size_t)t.ptr.ptr3 == 1); }
decl_func_cm1 ::= decl_func_begin_cm1 decl_func_args_list space_begin(space) space.
{ _Tcm1_Ffunc_code_bc_1(space.ptr.ptr); }
decl_func_cm1 ::= decl_func_begin_cm1 decl_func_args_list SEMICOLON.
{ _Tcm1_Ffunc_decl_0(); }

decls ::= decl.
decls ::= decls decl.

expr_call_begin(out) ::= FUNC(f).
{
   out.basic.path = f.basic.path;
   out.basic.row = f.basic.row;
   out.basic.col = f.basic.col;
   out.basic.id = f.basic.id;
   _Tcm1_Fexpr_call_begin_0();
}

expr_call_arg ::= expr(e).
{ _Tcm1_Fexpr_call_arg_1(e.ptr.ptr); }

expr_call_args ::= expr_call_arg.
expr_call_args ::= expr_call_args COMMA expr_call_arg.

expr_call_args_list ::= expr_call_args RPAREN.
expr_call_args_list ::= RPAREN.

stmt_return ::= RETURN(e) SEMICOLON.
{ _Tcm1_Fstmt_return_2(NULL, PATH(e)); }

stmt_return ::= RETURN expr(e) SEMICOLON.
{ _Tcm1_Fstmt_return_2(e.ptr.ptr, NULL); }

stmt_if_head ::= if_begin RPAREN_THEN_CURLY space_begin(s) space.
{
   _Tcm1_Fstmt_if_space_1(s.ptr.ptr);
}

rparen_space_begin(space) ::= RPAREN.
{ space.ptr.ptr = _Tcm1_Fspace_begin_0(); }

space_begin_only(space) ::= .
{ space.ptr.ptr = _Tcm1_Fspace_begin_0(); }

space_end ::= .
{ _Tcm1_Fspace_end_0(); }

if_begin ::= IF LPAREN expr(e).
{ _Tcm1_Fstmt_if_begin_1(e.ptr.ptr); }

stmt_if_head ::= if_begin rparen_space_begin(s) stmt_no_if space_end.
{
   _Tcm1_Fstmt_if_space_1(s.ptr.ptr);
}

stmt_if ::= stmt_if_head stmt_if_tail.
{
   _Tcm1_Fstmt_if_end_0();
}

stmt_if_tail ::= .
{
}

stmt_if_tail ::= ELSE(e) space_begin(s) space.
{
   _Tcm1_Fstmt_else_2(s.ptr.ptr, PATH(e));
}

stmt_if_tail ::= ELSE(e) space_begin_only(s) stmt_no_if space_end.
{
   _Tcm1_Fstmt_else_2(s.ptr.ptr, PATH(e));
}

else_if_begin ::= ELSE IF LPAREN expr(e).
{ _Tcm1_Fstmt_elif_begin_1(e.ptr.ptr); }

stmt_elif ::= else_if_begin RPAREN_THEN_CURLY space_begin(s) space.
{
   _Tcm1_Fstmt_elif_end_1(s.ptr.ptr);
}

stmt_elif ::= else_if_begin rparen_space_begin(s) stmt_no_if space_end.
{
   _Tcm1_Fstmt_elif_end_1(s.ptr.ptr);
}

stmt_if_tail ::= stmt_elif stmt_if_tail.

stmt_while ::= WHILE LPAREN expr(e) RPAREN_THEN_CURLY space_begin(s) space.
{ _Tcm1_Fstmt_while_2(e.ptr.ptr, s.ptr.ptr); }
stmt_while ::= WHILE LPAREN expr(e) rparen_space_begin(s) stmt_no_if space_end.
{ _Tcm1_Fstmt_while_2(e.ptr.ptr, s.ptr.ptr); }

stmt_do_while ::= DO(d) space_begin(s) space WHILE LPAREN expr(e) RPAREN SEMICOLON.
{ _Tcm1_Fstmt_do_while_3(e.ptr.ptr, s.ptr.ptr, PATH(d)); }
stmt_do_while ::= DO(d) space_begin_only(s) stmt_no_if space_end WHILE LPAREN expr(e) RPAREN SEMICOLON.
{ _Tcm1_Fstmt_do_while_3(e.ptr.ptr, s.ptr.ptr, PATH(d)); }

for_begin(out) ::= FOR(f) LPAREN.
{
   out.ptr.path = f.ptr.path;
   out.ptr.row = f.ptr.row;
   out.ptr.col = f.ptr.col;
   out.ptr.ptr = _Tcm1_Fspace_begin_0();
}
for_init ::= SEMICOLON.
for_init ::= expr(e) SEMICOLON.
{ _Tcm1_Fstmt_expr_1(e.ptr.ptr); }
for_init ::= stmt_var.
for_condition(out) ::= SEMICOLON.
{ out.ptr.ptr = NULL; }
for_condition(out) ::= expr(e) SEMICOLON.
{ out.ptr.ptr = e.ptr.ptr; }
for_iteration_curly(out) ::= RPAREN_THEN_CURLY.
{ out.ptr.ptr = NULL; }
for_iteration_curly(out) ::= expr(e) RPAREN_THEN_CURLY.
{ out.ptr.ptr = e.ptr.ptr; }
for_iteration_rparen(out) ::= RPAREN.
{ out.ptr.ptr = NULL; }
for_iteration_rparen(out) ::= expr(e) RPAREN.
{ out.ptr.ptr = e.ptr.ptr; }
stmt_for ::= for_begin(f) for_init for_condition(c) for_iteration_curly(i) space_begin(s) space.
{ _Tcm1_Fstmt_for_5(c.ptr.ptr, i.ptr.ptr, s.ptr.ptr, f.ptr.ptr, PATH(f)); }
stmt_for ::= for_begin(f) for_init for_condition(c) for_iteration_rparen(i) space_begin_only(s) stmt_no_if space_end.
{ _Tcm1_Fstmt_for_5(c.ptr.ptr, i.ptr.ptr, s.ptr.ptr, f.ptr.ptr, PATH(f)); }

stmt_switch ::= SWITCH(w) LPAREN expr(e) RPAREN_THEN_CURLY space_begin(s) space.
{ _Tcm1_Fstmt_switch_3(e.ptr.ptr, s.ptr.ptr, PATH(w)); }
stmt_switch ::= SWITCH(w) LPAREN expr(e) rparen_space_begin(s) stmt_no_if space_end.
{ _Tcm1_Fstmt_switch_3(e.ptr.ptr, s.ptr.ptr, PATH(w)); }

stmt_case ::= CASE(c) expr(e) COLON.
{ _Tcm1_Fstmt_case_2(e.ptr.ptr, PATH(c)); }
stmt_default ::= DEFAULT(d) COLON.
{ _Tcm1_Fstmt_default_1(PATH(d)); }
stmt_break ::= BREAK(b) SEMICOLON.
{ _Tcm1_Fstmt_break_1(PATH(b)); }
stmt_continue ::= CONTINUE(c) SEMICOLON.
{ _Tcm1_Fstmt_continue_1(PATH(c)); }
stmt_named_label ::= ID(i) COLON.
{ _Tcm1_Fstmt_named_label_2(i.basic.id, PATH(i)); }
stmt_goto ::= GOTO(g) ID(i) SEMICOLON.
{ _Tcm1_Fstmt_goto_2(i.basic.id, PATH(g)); }

dot_or_arrow(out) ::= DOT.
{ out.basic.id = 0; }

dot_or_arrow(out) ::= ARROW.
{ out.basic.id = 1; }

expr_field(out) ::= postfix_expr(e) dot_or_arrow(d) ID(i).
{ out.ptr.ptr = _Tcm1_Fexpr_field_3(e.ptr.ptr, d.basic.id, i.basic.id); }
expr_var(out) ::= ID(i).
{ out.ptr.ptr = _Tcm1_Fexpr_var_2(i.basic.id, PATH(i)); }
expr_call(out) ::= expr_call_begin(name) expr_call_args_list.
{ out.ptr.ptr = _Tcm1_Fexpr_call_2(name.basic.id, PATH(name)); }

expr_i32(out) ::= ZERO(z).
{ out.ptr.ptr = _Tcm1_Fexpr_i32_2(0, PATH(z)); }

expr_i32(out) ::= I32(i).
{ out.ptr.ptr = _Tcm1_Fexpr_i32_2(i.basic.id, PATH(i)); }

expr_u32(out) ::= U32(i).
{ out.ptr.ptr = _Tcm1_Fexpr_u32_2((uint32_t)i.u64.u64, PATH(i)); }

expr_i64(out) ::= I64(i).
{ out.ptr.ptr = _Tcm1_Fexpr_i64_2((int64_t)i.u64.u64, PATH(i)); }

expr_u64(out) ::= U64(i).
{ out.ptr.ptr = _Tcm1_Fexpr_u64_2(i.u64.u64, PATH(i)); }

string_literals(out) ::= STRING(s).
{
   out.u64.path = s.u64.path;
   out.u64.row = s.u64.row;
   out.u64.col = s.u64.col;
   out.u64.u64 = _Tcm1_Fexpr_string_concat_begin_3(
      s.ptr.ptr, s.ptr.ptr2 - s.ptr.ptr, PATH(s));
}
string_literals(out) ::= string_literals(a) STRING(s).
{
   out.u64.path = a.u64.path;
   out.u64.row = a.u64.row;
   out.u64.col = a.u64.col;
   out.u64.u64 = _Tcm1_Fexpr_string_concat_append_4(
      a.u64.u64, s.ptr.ptr, s.ptr.ptr2 - s.ptr.ptr, PATH(s));
}
expr_string(out) ::= string_literals(s).
{ out.ptr.ptr = _Tcm1_Fexpr_string_concat_end_2(s.u64.u64, PATH(s)); }
stmt_expr ::= expr.
stmt_no_if ::= stmt_return.
stmt_no_if ::= stmt_while.
stmt_no_if ::= stmt_do_while.
stmt_no_if ::= stmt_for.
stmt_no_if ::= stmt_switch.
stmt_no_if ::= stmt_case.
stmt_no_if ::= stmt_default.
stmt_no_if ::= stmt_break.
stmt_no_if ::= stmt_continue.
stmt_no_if ::= stmt_named_label.
stmt_no_if ::= stmt_goto.
stmt_no_if ::= SEMICOLON.
stmt ::= stmt_if.
stmt ::= stmt_no_if.
stmt_no_if ::= stmt_expr(e) SEMICOLON.
{ _Tcm1_Fstmt_expr_1(e.ptr.ptr); }
stmt_no_if ::= stmt_var.
stmts ::= stmt.
stmts ::= stmts stmt.
space ::= stmts CLOSE_CURLY_BRACE.
{ _Tcm1_Fspace_end_0(); }
space ::= CLOSE_CURLY_BRACE.
{ _Tcm1_Fspace_end_0(); }
space_begin(space) ::= OPEN_CURLY_BRACE.
{ space.ptr.ptr = _Tcm1_Fspace_begin_0(); }

/*
List of all operators:
- LPAREN RPAREN LBRACKET RBRACKET
- DOT ARROW
- INC DEC
- AMP STAR PLUS MINUS TILDE BANG
- SLASH PERCENT
- SHL SHR
- LT LE GT GE
- EQ NE
- CARET PIPE
- LAND LOR
- QUESTION COLON
- ASSIGN
- MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
- ADD_ASSIGN SUB_ASSIGN
- SHL_ASSIGN SHR_ASSIGN
- AND_ASSIGN XOR_ASSIGN OR_ASSIGN
- COMMA
- SIZEOF ALIGNOF
*/

expr_single(out) ::= expr_var(e).
{ out.ptr.ptr = e.ptr.ptr; }

expr_single(out) ::= expr_i32(e).
{ out.ptr.ptr = e.ptr.ptr; }

expr_single(out) ::= expr_u32(e).
{ out.ptr.ptr = e.ptr.ptr; }

expr_single(out) ::= expr_i64(e).
{ out.ptr.ptr = e.ptr.ptr; }

expr_single(out) ::= expr_u64(e).
{ out.ptr.ptr = e.ptr.ptr; }

expr_single(out) ::= expr_string(e).
{ out.ptr.ptr = e.ptr.ptr; }

expr_single(out) ::= F32_NUM(e).
{ out.ptr.ptr = _Tcm1_Fexpr_f32_2(e.f32.f32, PATH(e)); }

expr_single(out) ::= F64_NUM(e).
{ out.ptr.ptr = _Tcm1_Fexpr_f64_2(e.f64.f64, PATH(e)); }

expr_single(out) ::= LPAREN expr(e) RPAREN.
{ out.ptr.ptr = e.ptr.ptr; }

compound_literal(out) ::= LPAREN type(t) RPAREN initializer.
{
   out.ptr.ptr = _Tcm1_Fexpr_initializer_final_4(
      t.ptr.ptr, (size_t)t.ptr.ptr2, 0, PATH(t));
}

expr_single(out) ::= compound_literal(e).
{ out.ptr.ptr = e.ptr.ptr; }

postfix_expr(out) ::= expr_single(e).
{ out.ptr.ptr = e.ptr.ptr; }

postfix_expr(out) ::= postfix_expr(a) LBRACKET expr(i) RBRACKET.
{ out.ptr.ptr = _Tcm1_Fexpr_index_2(a.ptr.ptr, i.ptr.ptr); }

postfix_expr ::= expr_call.

postfix_expr ::= expr_field.

postfix_expr(out) ::= postfix_expr(e) INC.
{ out.ptr.ptr = _Tcm1_Fexpr_inc_dec_3(e.ptr.ptr, 1, 0); }

postfix_expr(out) ::= postfix_expr(e) DEC.
{ out.ptr.ptr = _Tcm1_Fexpr_inc_dec_3(e.ptr.ptr, 0, 0); }

unary_expr(out) ::= postfix_expr(e).
{ out.ptr.ptr = e.ptr.ptr; }

unary_expr(out) ::= INC unary_expr(e).
{ out.ptr.ptr = _Tcm1_Fexpr_inc_dec_3(e.ptr.ptr, 1, 1); }

unary_expr(out) ::= DEC unary_expr(e).
{ out.ptr.ptr = _Tcm1_Fexpr_inc_dec_3(e.ptr.ptr, 0, 1); }

unary_expr(out) ::= unary_op(o) cast_expr(e).
{ out.ptr.ptr = _Tcm1_Fexpr_unary_2(e.ptr.ptr, o.basic.id); }

unary_expr(out) ::= SIZEOF unary_expr(e).
{ out.ptr.ptr = _Tcm1_Fexpr_sizeof_expr_1(e.ptr.ptr); }

unary_expr(out) ::= SIZEOF LPAREN type(t) RPAREN.
{ out.ptr.ptr = _Tcm1_Fexpr_size_align_type_4(t.ptr.ptr, (size_t)t.ptr.ptr2, 0, PATH(t)); }

unary_expr(out) ::= ALIGNOF LPAREN type(t) RPAREN.
{ out.ptr.ptr = _Tcm1_Fexpr_size_align_type_4(t.ptr.ptr, (size_t)t.ptr.ptr2, 1, PATH(t)); }

unary_op(out) ::= AMP.
{ out.basic.id = 0; }

unary_op(out) ::= STAR.
{ out.basic.id = 1; }

unary_op(out) ::= PLUS.
{ out.basic.id = 2; }

unary_op(out) ::= MINUS.
{ out.basic.id = 3; }

unary_op(out) ::= TILDE.
{ out.basic.id = 4; }

unary_op(out) ::= BANG.
{ out.basic.id = 5; }

cast_expr ::= unary_expr.

cast_expr(out) ::= LPAREN type(t) RPAREN cast_expr(e).
{ out.ptr.ptr = _Tcm1_Fexpr_cast_4(e.ptr.ptr, t.ptr.ptr, (size_t)t.ptr.ptr2, PATH(t)); }

multiplicative_expr ::= cast_expr.

multiplicative_expr(out) ::= multiplicative_expr(e1) STAR cast_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 2, e2.ptr.ptr); }

multiplicative_expr(out) ::= multiplicative_expr(e1) SLASH cast_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 3, e2.ptr.ptr); }

multiplicative_expr(out) ::= multiplicative_expr(e1) PERCENT cast_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 4, e2.ptr.ptr); }

additive_expr ::= multiplicative_expr.

additive_expr(out) ::= additive_expr(e1) PLUS multiplicative_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 0, e2.ptr.ptr); }

additive_expr(out) ::= additive_expr(e1) MINUS multiplicative_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 1, e2.ptr.ptr); }

shift_expr ::= additive_expr.

shift_expr(out) ::= shift_expr(e1) SHL additive_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 5, e2.ptr.ptr); }

shift_expr(out) ::= shift_expr(e1) SHR additive_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 6, e2.ptr.ptr); }

relational_expr ::= shift_expr.

relational_expr(out) ::= relational_expr(e1) LT shift_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 10, e2.ptr.ptr); }

relational_expr(out) ::= relational_expr(e1) LE shift_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 11, e2.ptr.ptr); }

relational_expr(out) ::= relational_expr(e1) GT shift_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 12, e2.ptr.ptr); }

relational_expr(out) ::= relational_expr(e1) GE shift_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 13, e2.ptr.ptr); }

equality_expr ::= relational_expr.

equality_expr(out) ::= equality_expr(e1) EQ relational_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 14, e2.ptr.ptr); }

equality_expr(out) ::= equality_expr(e1) NE relational_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 15, e2.ptr.ptr); }

bitwise_and_expr ::= equality_expr.

bitwise_and_expr(out) ::= bitwise_and_expr(e1) AMP equality_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 7, e2.ptr.ptr); }

bitwise_xor_expr ::= bitwise_and_expr.

bitwise_xor_expr(out) ::= bitwise_xor_expr(e1) CARET bitwise_and_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 8, e2.ptr.ptr); }

bitwise_or_expr ::= bitwise_xor_expr.

bitwise_or_expr(out) ::= bitwise_or_expr(e1) PIPE bitwise_xor_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_math_3(e1.ptr.ptr, 9, e2.ptr.ptr); }

logical_and_expr ::= bitwise_or_expr.

logical_and_expr(out) ::= logical_and_expr(e1) LAND bitwise_or_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_logical_3(e1.ptr.ptr, 1, e2.ptr.ptr); }

logical_or_expr ::= logical_and_expr.

logical_or_expr(out) ::= logical_or_expr(e1) LOR logical_and_expr(e2).
{ out.ptr.ptr = _Tcm1_Fexpr_logical_3(e1.ptr.ptr, 0, e2.ptr.ptr); }

conditional_expr ::= logical_or_expr.

conditional_expr(out) ::= logical_or_expr(c) QUESTION expr(t) COLON conditional_expr(f).
{ out.ptr.ptr = _Tcm1_Fexpr_conditional_3(c.ptr.ptr, t.ptr.ptr, f.ptr.ptr); }

expr_assign ::= conditional_expr.

expr_assign(out) ::= unary_expr(d) assign_op(o) expr_assign(s).
{ out.ptr.ptr = _Tcm1_Fexpr_assign_3(d.ptr.ptr, o.basic.id, s.ptr.ptr); }

expr ::= expr_assign.

// Used in C but not supported in Cm1:
// expr ::= expr COMMA expr_assign.

assign_op(out) ::= ASSIGN.
{ out.basic.id = 0; }

assign_op(out) ::= ADD_ASSIGN.
{ out.basic.id = 1; }

assign_op(out) ::= SUB_ASSIGN.
{ out.basic.id = 2; }

assign_op(out) ::= MUL_ASSIGN.
{ out.basic.id = 3; }

assign_op(out) ::= DIV_ASSIGN.
{ out.basic.id = 4; }

assign_op(out) ::= MOD_ASSIGN.
{ out.basic.id = 5; }

assign_op(out) ::= SHL_ASSIGN.
{ out.basic.id = 6; }

assign_op(out) ::= SHR_ASSIGN.
{ out.basic.id = 7; }

assign_op(out) ::= AND_ASSIGN.
{ out.basic.id = 8; }

assign_op(out) ::= XOR_ASSIGN.
{ out.basic.id = 9; }

// Make sure OR_ASSIGN is at the bottom of this file because lex-re2c relies on it.
assign_op(out) ::= OR_ASSIGN.
{ out.basic.id = 10; }
