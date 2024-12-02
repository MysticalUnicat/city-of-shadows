#ifndef script_local_h_INCLUDED
#define script_local_h_INCLUDED

#include "string.h"
#include "memory.h"

enum {
  Script_AST_OP_none,
  Script_AST_OP_binary_or,
  Script_AST_OP_binary_and,
  Script_AST_OP_binary_xor,
  Script_AST_OP_binary_neg,
  Script_AST_OP_logical_or,
  Script_AST_OP_logical_and,
  Script_AST_OP_number_add,
  Script_AST_OP_number_sub,
  Script_AST_OP_number_mul,
  Script_AST_OP_number_div,
  Script_AST_OP_number_mod,
  Script_AST_OP_number_neg,
  Script_AST_OP_pointer_deref,
  Script_AST_OP_pointer_ref,
};

struct Script_AST {
  enum {
    Script_AST_list,
    Script_AST_declaration,
    Script_AST_binding,
    Script_AST_if,
    Script_AST_elif,
    Script_AST_while,
    Script_AST_assign,
    Script_AST_return,
    Script_AST_continue,
    Script_AST_break,
    Script_AST_unary,
    Script_AST_binary,
    Script_AST_array_access,
    Script_AST_call,
    Script_AST_member_access,
    Script_AST_name,
    Script_AST_type,
    Script_AST_char_type,
    Script_AST_char,
    Script_AST_void_type,
    Script_AST_bool_type,
    Script_AST_true_value,
    Script_AST_false_value,
    Script_AST_uint_type,
    Script_AST_uint_value,
    Script_AST_u8_type,
    Script_AST_u8_value,
    Script_AST_u16_type,
    Script_AST_u16_value,
    Script_AST_u32_type,
    Script_AST_u32_value,
    Script_AST_u64_type,
    Script_AST_u64_value,
    Script_AST_int_type,
    Script_AST_int_value,
    Script_AST_i8_type,
    Script_AST_i8_value,
    Script_AST_i16_type,
    Script_AST_i16_value,
    Script_AST_i32_type,
    Script_AST_i32_value,
    Script_AST_i64_type,
    Script_AST_i64_value,
    Script_AST_real_type,
    Script_AST_real_value,
    Script_AST_f32_type,
    Script_AST_f32_value,
    Script_AST_f64_type,
    Script_AST_f64_value,
    Script_AST_annotation,
    Script_AST_structure_type,
    Script_AST_structure_value,
    Script_AST_enum_type,
    Script_AST_enum_value,
    Script_AST_function_type,
    Script_AST_function_value,
    Script_AST_tuple_type,
    Script_AST_tuple_value,
    Script_AST_string,
  } kind;
  union {
    struct {
      uint32_t count;
      struct Script_AST ** items;
    } list;
    struct {
      MStr name;
      struct Script_AST * type;
      struct Script_AST * value;
    } declaration;
    struct {
      MStr name;
      struct Script_AST * type;
    } binding;
    struct {
      MStr let_name;
      struct Script_AST * let_value;
      struct Script_AST * test;
      struct Script_AST * block;
      struct Script_AST * _else;
    } _if, _elif, _while;
    struct {
      struct Script_AST * target;
      int op;
      struct Script_AST * value;
    } assign;
    struct {
      struct Script_AST * value;
    } _return;
    struct {
      int op;
      struct Script_AST * right;
    } unary;
    struct {
      struct Script_AST * left;
      int op;
      struct Script_AST * right;
    } binary;
    struct {
      struct Script_AST * left;
      struct Script_AST * index;
    } array_access;
    struct {
      struct Script_AST * function;
      struct Script_AST * arguments;
    } call;
    struct {
      struct Script_AST * value;
      MStr name;
    } member_access;
    struct {
      struct Script_AST * value;
      struct Script_AST * type;
    } annotation;
    struct {
      struct Script_AST * members;
    } structure_type;
    struct {
      struct Script_AST * values;
      struct Script_AST * from;
    } structure_value;
    struct {
      struct Script_AST * members;
    } enum_type;
    struct {
      struct Script_AST * option;
      struct Script_AST * value;
    } enum_value;
    struct {
      struct Script_AST * arguments;
      struct Script_AST * result;
    } function_type;
    struct {
      struct Script_AST * arguments;
      struct Script_AST * block;
    } function_value;
    struct {
      struct Script_AST * types;
    } tuple_type;
    struct {
      struct Script_AST * values;
    } tuple_value;
    struct {
      MStr value;
    } name;
    struct {
      MStr value;
    } string;
    uint64_t uint, character;
    int64_t _int;
    double real;
  };
};


struct Script_AST_Allocator {
  struct Script_AST return_no_argument;
  struct Script_AST _break;
  struct Script_AST _continue;
  struct Script_AST type;
  struct Script_AST char_type;
  struct Script_AST void_type;
  struct Script_AST bool_type;
  struct Script_AST true_value;
  struct Script_AST false_value;
  struct Script_AST uint_type;
  struct Script_AST u8_type;
  struct Script_AST u16_type;
  struct Script_AST u32_type;
  struct Script_AST u64_type;
  struct Script_AST int_type;
  struct Script_AST i8_type;
  struct Script_AST i16_type;
  struct Script_AST i32_type;
  struct Script_AST i64_type;
  struct Script_AST real_type;
  struct Script_AST f32_type;
  struct Script_AST f64_type;
};

static inline void Script_AST_Allocator_init(struct Script_AST_Allocator * allocator) {
  memory_clear(allocator, sizeof(*allocator));
  allocator->return_no_argument.kind = Script_AST_return;
  allocator->_break.kind = Script_AST_break;
  allocator->_continue.kind = Script_AST_continue;
  allocator->type.kind = Script_AST_type;
  allocator->char_type.kind = Script_AST_char_type;
  allocator->void_type.kind = Script_AST_void_type;
  allocator->bool_type.kind = Script_AST_bool_type;
  allocator->true_value.kind = Script_AST_true_value;
  allocator->false_value.kind = Script_AST_false_value;
  allocator->uint_type.kind = Script_AST_uint_type;
  allocator->u8_type.kind = Script_AST_u8_type;
  allocator->u16_type.kind = Script_AST_u16_type;
  allocator->u32_type.kind = Script_AST_u32_type;
  allocator->u64_type.kind = Script_AST_u64_type;
  allocator->int_type.kind = Script_AST_int_type;
  allocator->i8_type.kind = Script_AST_i8_type;
  allocator->i16_type.kind = Script_AST_i16_type;
  allocator->i32_type.kind = Script_AST_i32_type;
  allocator->i64_type.kind = Script_AST_i64_type;
  allocator->real_type.kind = Script_AST_real_type;
  allocator->f32_type.kind = Script_AST_f32_type;
  allocator->f64_type.kind = Script_AST_f64_type;
}

static inline void Script_AST_Allocator_free(struct Script_AST_Allocator * allocator) {
}

static inline struct Script_AST * Script_AST_allocate(struct Script_AST_Allocator * allocator, int kind) {
  struct Script_AST * result = memory_alloc(sizeof(*result), alignof(*result));
  result->kind = kind;
  return result;
}

static inline struct Script_AST * Script_AST_as_list(struct Script_AST_Allocator * allocator, struct Script_AST * list) {
  if(list->kind == Script_AST_list) {
    return list;
  }
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_list);
  result->list.count = 1;
  result->list.items = memory_alloc(sizeof(*result->list.items), alignof(*result->list.items));
  result->list.items[0] = list;
  return result;
}

static inline struct Script_AST * Script_AST_append(struct Script_AST_Allocator * allocator, struct Script_AST * list, struct Script_AST * item) {
  list = Script_AST_as_list(allocator, list);
  list->list.items = memory_realloc(list->list.items, sizeof(*list->list.items) * list->list.count, sizeof(*list->list.items) * (list->list.count + 1), alignof(*list->list.items));
  list->list.items[list->list.count] = item;
  list->list.count += 1;
  return list;
}

static inline struct Script_AST * Script_AST_prepend(struct Script_AST_Allocator * allocator, struct Script_AST * list, struct Script_AST * item) {
  list = Script_AST_as_list(allocator, list);
  struct Script_AST ** new_items = memory_alloc(sizeof(*list->list.items) * (list->list.count + 1), alignof(*list->list.items)); 
  new_items[0] = item;
  memory_copy(new_items + 1, sizeof(*list->list.items) * list->list.count, list->list.items, sizeof(*list->list.items) * list->list.count);
  memory_free(list->list.items, sizeof(*list->list.items) * (list->list.count + 1), alignof(*list->list.items));
  list->list.count += 1;
  return list;
}

static inline struct Script_AST * Script_AST_mk_name(struct Script_AST_Allocator * allocator, const char * name) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_name);
  result->name.value = string_clone(name);
  return result;
}

static inline struct Script_AST * Script_AST_mk_declaration(struct Script_AST_Allocator * allocator, const char * name, struct Script_AST * type, struct Script_AST * value) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_declaration);
  result->declaration.name = string_clone(name);
  result->declaration.type = type;
  result->declaration.value = value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_binding(struct Script_AST_Allocator * allocator, const char * name, struct Script_AST * type) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_binding);
  result->binding.name = string_clone(name);
  result->binding.type = type;
  return result;
}

static inline struct Script_AST * Script_AST_mk_if(struct Script_AST_Allocator * allocator, const char * let_name, struct Script_AST * let_value, struct Script_AST * test, struct Script_AST * block, struct Script_AST * elif) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_if);
  if(let_name != NULL) {
    result->_if.let_name = string_clone(let_name);
    result->_if.let_value = let_value;
  } else {
    result->_if.let_name = NULL;
    result->_if.let_value = NULL;
  }
  result->_if.test = test;
  result->_if.block = block;
  result->_if._else = elif;
  return result;
}

static inline struct Script_AST * Script_AST_mk_elif(struct Script_AST_Allocator * allocator, const char * let_name, struct Script_AST * let_value, struct Script_AST * test, struct Script_AST * block, struct Script_AST * elif) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_elif);
  if(let_name != NULL) {
    result->_elif.let_name = string_clone(let_name);
    result->_elif.let_value = let_value;
  } else {
    result->_elif.let_name = NULL;
    result->_elif.let_value = NULL;
  }
  result->_elif.test = test;
  result->_elif.block = block;
  result->_elif._else = elif;
  return result;
}

static inline struct Script_AST * Script_AST_mk_while(struct Script_AST_Allocator * allocator, const char * let_name, struct Script_AST * let_value, struct Script_AST * test, struct Script_AST * block, struct Script_AST * else_block) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_while);
  if(let_name != NULL) {
    result->_while.let_name = string_clone(let_name);
    result->_while.let_value = let_value;
  } else {
    result->_while.let_name = NULL;
    result->_while.let_value = NULL;
  }
  result->_while.test = test;
  result->_while.block = block;
  result->_while._else = else_block;
  return result;
}

static inline struct Script_AST * Script_AST_mk_assign(struct Script_AST_Allocator * allocator, struct Script_AST * target, int op, struct Script_AST * value) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_assign);
  result->assign.target = target;
  result->assign.op = op;
  result->assign.value = value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_return(struct Script_AST_Allocator * allocator, struct Script_AST * expr) {
  if(expr == NULL) {
    return &allocator->return_no_argument;
  }
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_return);
  result->_return.value = expr;
  return result;
}

static inline struct Script_AST * Script_AST_mk_break(struct Script_AST_Allocator * allocator) {
  return &allocator->_break;
}

static inline struct Script_AST * Script_AST_mk_continue(struct Script_AST_Allocator * allocator) {
  return &allocator->_continue;
}

static inline struct Script_AST * Script_AST_mk_unary(struct Script_AST_Allocator * allocator, int op, struct Script_AST * right) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_unary);
  result->unary.op = op;
  result->unary.right = right;
  return result;
}

static inline struct Script_AST * Script_AST_mk_binary(struct Script_AST_Allocator * allocator, struct Script_AST * left, int op, struct Script_AST * right) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_binary);
  result->binary.left = left;
  result->binary.op = op;
  result->binary.right = right;
  return result;
}

static inline struct Script_AST * Script_AST_mk_array_access(struct Script_AST_Allocator * allocator, struct Script_AST * left, struct Script_AST * index) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_array_access);
  result->array_access.left = left;
  result->array_access.index = index;
  return result;
}

static inline struct Script_AST * Script_AST_mk_call(struct Script_AST_Allocator * allocator, struct Script_AST * function, struct Script_AST * arguments) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_call);
  result->call.function = function;
  result->call.arguments = arguments;
  return result;
}

static inline struct Script_AST * Script_AST_mk_member_access(struct Script_AST_Allocator * allocator, struct Script_AST * value, const char * name) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_member_access);
  result->member_access.value = value;
  result->member_access.name = string_clone(name);
  return result;
}

static inline struct Script_AST * Script_AST_mk_annotation(struct Script_AST_Allocator * allocator, struct Script_AST * value, struct Script_AST * type) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_annotation);
  result->annotation.value = value;
  result->annotation.type = type;
  return result;
}

static inline struct Script_AST * Script_AST_mk_structure_type(struct Script_AST_Allocator * allocator, struct Script_AST * members) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_structure_type);
  result->structure_type.members = members;
  return result;
}

static inline struct Script_AST * Script_AST_mk_structure_value(struct Script_AST_Allocator * allocator, struct Script_AST * values, struct Script_AST * from) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_structure_value);
  result->structure_value.values = values;
  result->structure_value.from = from;
  return result;
}

static inline struct Script_AST * Script_AST_mk_enum_type(struct Script_AST_Allocator * allocator, struct Script_AST * members) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_enum_type);
  result->enum_type.members = members;
  return result;
}

static inline struct Script_AST * Script_AST_mk_enum_value(struct Script_AST_Allocator * allocator, const char * option, struct Script_AST * value) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_enum_value);
  result->enum_value.option = Script_AST_mk_name(allocator, option);
  result->enum_value.value = value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_function_type(struct Script_AST_Allocator * allocator, struct Script_AST * arguments, struct Script_AST * result_type) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_function_type);
  result->function_type.arguments = arguments;
  result->function_type.result = result_type;
  return result;
}

static inline struct Script_AST * Script_AST_mk_function_value(struct Script_AST_Allocator * allocator, struct Script_AST * arguments, struct Script_AST * block) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_function_value);
  result->function_value.arguments = arguments;
  result->function_value.block = block;
  return result;
}


static inline struct Script_AST * Script_AST_mk_extract_names(struct Script_AST_Allocator * allocator, struct Script_AST * arguments) {
  struct Script_AST * result = NULL;
  #define ADD_BINDING(X) \
		do { \
			if(result == NULL) result = Script_AST_as_list(allocator, Script_AST_mk_name(allocator, X->binding.name)); \
			else Script_AST_append(allocator, result, Script_AST_mk_name(allocator, X->binding.name)); \
		} while(0)
  if(arguments->kind == Script_AST_list) {
    for(uint32_t i = 0; i < arguments->list.count; i += 1) {
      ADD_BINDING(arguments->list.items[i]);
    }
  } else {
    ADD_BINDING(arguments);
  }
  #undef ADD_BINDING
  return result;
}

static inline struct Script_AST * Script_AST_mk_tuple_type(struct Script_AST_Allocator * allocator, struct Script_AST * types) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_tuple_type);
  result->tuple_type.types = types;
  return result;
}

static inline struct Script_AST * Script_AST_mk_tuple_value(struct Script_AST_Allocator * allocator, struct Script_AST * values) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_tuple_value);
  result->tuple_value.values = values;
  return result;
}

static inline struct Script_AST * Script_AST_mk_string(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_allocate(allocator, Script_AST_string);
  result->string.value = string_clone(s);
  return result;
}

static inline struct Script_AST * Script_AST_mk_char_type(struct Script_AST_Allocator * allocator) {
  return &allocator->char_type;
}

static inline struct Script_AST * Script_AST_mk_type(struct Script_AST_Allocator * allocator) {
  return &allocator->type;
}

static inline struct Script_AST * Script_AST_mk_void(struct Script_AST_Allocator * allocator) {
  return &allocator->void_type;
}

static inline struct Script_AST * Script_AST_mk_bool(struct Script_AST_Allocator * allocator) {
  return &allocator->bool_type;
}

static inline struct Script_AST * Script_AST_mk_true(struct Script_AST_Allocator * allocator) {
  return &allocator->true_value;
}

static inline struct Script_AST * Script_AST_mk_false(struct Script_AST_Allocator * allocator) {
  return &allocator->false_value;
}

static inline struct Script_AST * Script_AST_mk_character(struct Script_AST_Allocator * allocator, const char * s) {
  // TODO
}

static inline struct Script_AST * Script_AST_mk_uint_type(struct Script_AST_Allocator * allocator) {
  return &allocator->uint_type;
}

static inline struct Script_AST * Script_AST_mk_uint_value(struct Script_AST_Allocator * allocator, const char * s) {
  // TODO
}

static inline struct Script_AST * Script_AST_mk_u8_type(struct Script_AST_Allocator * allocator) {
  return &allocator->u8_type;
}

static inline struct Script_AST * Script_AST_mk_u8_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_uint_value(allocator, s);
  result->kind = Script_AST_u8_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_u16_type(struct Script_AST_Allocator * allocator) {
  return &allocator->u16_type;
}

static inline struct Script_AST * Script_AST_mk_u16_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_uint_value(allocator, s);
  result->kind = Script_AST_u16_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_u32_type(struct Script_AST_Allocator * allocator) {
  return &allocator->u32_type;
}

static inline struct Script_AST * Script_AST_mk_u32_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_uint_value(allocator, s);
  result->kind = Script_AST_u32_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_u64_type(struct Script_AST_Allocator * allocator) {
  return &allocator->u64_type;
}

static inline struct Script_AST * Script_AST_mk_u64_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_uint_value(allocator, s);
  result->kind = Script_AST_u64_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_int_type(struct Script_AST_Allocator * allocator) {
  return &allocator->int_type;
}

static inline struct Script_AST * Script_AST_mk_int_value(struct Script_AST_Allocator * allocator, const char * s) {
  // TODO
}

static inline struct Script_AST * Script_AST_mk_i8_type(struct Script_AST_Allocator * allocator) {
  return &allocator->i8_type;
}

static inline struct Script_AST * Script_AST_mk_i8_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_int_value(allocator, s);
  result->kind = Script_AST_i8_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_i16_type(struct Script_AST_Allocator * allocator) {
  return &allocator->i16_type;
}

static inline struct Script_AST * Script_AST_mk_i16_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_int_value(allocator, s);
  result->kind = Script_AST_i16_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_i32_type(struct Script_AST_Allocator * allocator) {
  return &allocator->i32_type;
}

static inline struct Script_AST * Script_AST_mk_i32_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_int_value(allocator, s);
  result->kind = Script_AST_i32_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_i64_type(struct Script_AST_Allocator * allocator) {
  return &allocator->i64_type;
}

static inline struct Script_AST * Script_AST_mk_i64_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_int_value(allocator, s);
  result->kind = Script_AST_i64_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_real_type(struct Script_AST_Allocator * allocator) {
  return &allocator->real_type;
}

static inline struct Script_AST * Script_AST_mk_real_value(struct Script_AST_Allocator * allocator, const char * s) {
  // TODO
}

static inline struct Script_AST * Script_AST_mk_f32_type(struct Script_AST_Allocator * allocator) {
  return &allocator->f32_type;
}

static inline struct Script_AST * Script_AST_mk_f32_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_real_value(allocator, s);
  result->kind = Script_AST_f32_value;
  return result;
}

static inline struct Script_AST * Script_AST_mk_f64_type(struct Script_AST_Allocator * allocator) {
  return &allocator->f64_type;
}

static inline struct Script_AST * Script_AST_mk_f64_value(struct Script_AST_Allocator * allocator, const char * s) {
  struct Script_AST * result = Script_AST_mk_real_value(allocator, s);
  result->kind = Script_AST_f64_value;
  return result;
}

#endif // script_local_h_INCLUDED
