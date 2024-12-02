#include "script_local.h"

#include "write.h"

struct FormatAST {
  int indent;
  struct Writer * w;
};

static void _format_ast(struct FormatAST * f, const struct Script_AST * ast) {
  switch(ast->kind) {
  case Script_AST_list:
    break;
  case Script_AST_declaration:
    break;
  case Script_AST_binding:
    break;
  case Script_AST_if:
    break;
  case Script_AST_elif:
    break;
  case Script_AST_while:
    break;
  case Script_AST_assign:
    break;
  case Script_AST_return:
    break;
  case Script_AST_continue:
    break;
  case Script_AST_break:
    break;
  case Script_AST_unary:
    break;
  case Script_AST_binary:
    break;
  case Script_AST_array_access:
    break;
  case Script_AST_call:
    break;
  case Script_AST_member_access:
    break;
  case Script_AST_name:
    break;
  case Script_AST_type:
    break;
  case Script_AST_char_type:
    break;
  case Script_AST_char:
    break;
  case Script_AST_void_type:
    break;
  case Script_AST_bool_type:
    break;
  case Script_AST_true_value:
    break;
  case Script_AST_false_value:
    break;
  case Script_AST_uint_type:
    break;
  case Script_AST_uint_value:
    break;
  case Script_AST_u8_type:
    break;
  case Script_AST_u8_value:
    break;
  case Script_AST_u16_type:
    break;
  case Script_AST_u16_value:
    break;
  case Script_AST_u32_type:
    break;
  case Script_AST_u32_value:
    break;
  case Script_AST_u64_type:
    break;
  case Script_AST_u64_value:
    break;
  case Script_AST_int_type:
    break;
  case Script_AST_int_value:
    break;
  case Script_AST_i8_type:
    break;
  case Script_AST_i8_value:
    break;
  case Script_AST_i16_type:
    break;
  case Script_AST_i16_value:
    break;
  case Script_AST_i32_type:
    break;
  case Script_AST_i32_value:
    break;
  case Script_AST_i64_type:
    break;
  case Script_AST_i64_value:
    break;
  case Script_AST_real_type:
    break;
  case Script_AST_real_value:
    break;
  case Script_AST_f32_type:
    break;
  case Script_AST_f32_value:
    break;
  case Script_AST_f64_type:
    break;
  case Script_AST_f64_value:
    break;
  case Script_AST_annotation:
    break;
  case Script_AST_structure_type:
    break;
  case Script_AST_structure_value:
    break;
  case Script_AST_enum_type:
    break;
  case Script_AST_enum_value:
    break;
  case Script_AST_function_type:
    break;
  case Script_AST_function_value:
    break;
  case Script_AST_tuple_type:
    break;
  case Script_AST_tuple_value:
    break;
  case Script_AST_string:
    break;
  }
}

void Script_AST_format(const struct Script_AST * ast, struct Writer * w) {
  struct FormatAST f;
  f.indent = 0;
  f.w = w;
  _format_ast(&f, ast);
}

