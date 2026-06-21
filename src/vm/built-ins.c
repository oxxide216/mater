#include "built-ins.h"
#include "web-api.h"

static Value *print_built_in(Vm *vm, Value **args) {
  (void) vm;

  Value *arg = args[0];

  switch (arg->kind) {
  case ValueKindInt: {
    print_int(arg->as._int);
  } break;

  case ValueKindFloat: {
    print_float(arg->as._float);
  } break;

  case ValueKindBool: {
    print_bool(arg->as._bool);
  } break;

  case ValueKindStr: {
    print_str(arg->as.str.ptr, arg->as.str.len);
  } break;
  }

  return NULL;
}

void add_default_built_ins(BuiltIns *built_ins) {
  built_ins_add(built_ins, STR_LIT("print"), 1, print_built_in);
}
