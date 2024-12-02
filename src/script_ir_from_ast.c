#include "script_local.h"

struct FromAST {
  int indent;
};

static void _from_ast(struct ToC * to_c, struct Script_IR * ir) {
  switch(ast->kind) {
  case Script_IR_list:
    break;
  case Script_IR_declaration:
    break;
  case Script_IR_binding:
    break;
  case Script_IR_if:
    break;
  case Script_IR_elif:
    break;
  case Script_IR_while:
    break;
  case Script_IR_assign:
    break;
  case Script_IR_return:
    break;
  case Script_IR_continue:
    break;
  case Script_IR_break:
    break;
  case Script_IR_unary:
    break;
  case Script_IR_binary:
    break;
  case Script_IR_array_access:
    break;
  case Script_IR_call:
    break;
  case Script_IR_member_access:
    break;
  case Script_IR_name:
    break;
  case Script_IR_type:
    break;
  case Script_IR_char_type:
    break;
  case Script_IR_char:
    break;
  case Script_IR_void_type:
    break;
  case Script_IR_bool_type:
    break;
  case Script_IR_true_value:
    break;
  case Script_IR_false_value:
    break;
  case Script_IR_uint_type:
    break;
  case Script_IR_uint_value:
    break;
  case Script_IR_u8_type:
    break;
  case Script_IR_u8_value:
    break;
  case Script_IR_u16_type:
    break;
  case Script_IR_u16_value:
    break;
  case Script_IR_u32_type:
    break;
  case Script_IR_u32_value:
    break;
  case Script_IR_u64_type:
    break;
  case Script_IR_u64_value:
    break;
  case Script_IR_int_type:
    break;
  case Script_IR_int_value:
    break;
  case Script_IR_i8_type:
    break;
  case Script_IR_i8_value:
    break;
  case Script_IR_i16_type:
    break;
  case Script_IR_i16_value:
    break;
  case Script_IR_i32_type:
    break;
  case Script_IR_i32_value:
    break;
  case Script_IR_i64_type:
    break;
  case Script_IR_i64_value:
    break;
  case Script_IR_real_type:
    break;
  case Script_IR_real_value:
    break;
  case Script_IR_f32_type:
    break;
  case Script_IR_f32_value:
    break;
  case Script_IR_f64_type:
    break;
  case Script_IR_f64_value:
    break;
  case Script_IR_annotation:
    break;
  case Script_IR_structure_type:
    break;
  case Script_IR_structure_value:
    break;
  case Script_IR_enum_type:
    break;
  case Script_IR_enum_value:
    break;
  case Script_IR_function_type:
    break;
  case Script_IR_function_value:
    break;
  case Script_IR_tuple_type:
    break;
  case Script_IR_tuple_value:
    break;
  case Script_IR_string:
    break;
  }
}

struct Script_IR * Script_IR_from_ast(const struct Script_AST * ast) {
  _from_ast(NULL, ast);
}

