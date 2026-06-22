#include "built-ins.h"
#include "web-api.h"

#define CHECK_KIND(value, expected, name)                     \
  do {                                                        \
    if ((value)->kind != expected) {                          \
      char str[] = "Wrong argument type for "name"built-in!"; \
      eprint_str(str, sizeof(str) - 1);                       \
      return NULL;                                            \
    }                                                         \
  } while (0)

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

static Value *wprint_built_in(Vm *vm, Value **args) {
  (void) vm;

  Value *arg = args[0];

  switch (arg->kind) {
  case ValueKindInt: {
    wprint_int(arg->as._int);
  } break;

  case ValueKindFloat: {
    wprint_float(arg->as._float);
  } break;

  case ValueKindBool: {
    wprint_bool(arg->as._bool);
  } break;

  case ValueKindStr: {
    wprint_str(arg->as.str.ptr, arg->as.str.len);
  } break;
  }

  return NULL;
}

static Value *eprint_built_in(Vm *vm, Value **args) {
  (void) vm;

  Value *arg = args[0];

  switch (arg->kind) {
  case ValueKindInt: {
    eprint_int(arg->as._int);
  } break;

  case ValueKindFloat: {
    eprint_float(arg->as._float);
  } break;

  case ValueKindBool: {
    eprint_bool(arg->as._bool);
  } break;

  case ValueKindStr: {
    eprint_str(arg->as.str.ptr, arg->as.str.len);
  } break;
  }

  return NULL;
}

static Value *alert_built_in(Vm *vm, Value **args) {
  (void) vm;

  Value *str = args[0];

  CHECK_KIND(str, ValueKindStr, "alert");

  alert(str->as.str.ptr, str->as.str.len);

  return NULL;
}

void add_default_built_ins(BuiltIns *built_ins) {
  built_ins_add(built_ins, STR_LIT("print"), 1, print_built_in);
  built_ins_add(built_ins, STR_LIT("wprint"), 1, wprint_built_in);
  built_ins_add(built_ins, STR_LIT("eprint"), 1, eprint_built_in);
  built_ins_add(built_ins, STR_LIT("alert"), 1, alert_built_in);
}
