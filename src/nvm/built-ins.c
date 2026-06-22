#include "built-ins.h"
#include "shl/shl-log.h"

#define CHECK_KIND(value, expected, name)                     \
  do {                                                        \
    if ((value)->kind != expected) {                          \
      char str[] = "Wrong argument type for "name"built-in!"; \
      eprint_str(str, sizeof(str) - 1);                       \
      return NULL;                                            \
    }                                                         \
  } while (0)

void built_ins_add(BuiltIns *built_ins, Str name, u32 args_len, BuiltInFunc func) {
  u32 hash = str_hash(name) + args_len;
  u32 index = hash % BUILT_INS_CAP;

  BuiltIn **next = built_ins->items + index;
  while (*next)
    next = &(*next)->next;

  *next = malloc(sizeof(BuiltIn));
  **next = (BuiltIn) { name, args_len, func, NULL };
}

BuiltInFunc built_ins_get(BuiltIns *built_ins, Str name, u32 args_len) {
  u32 hash = str_hash(name) + args_len;
  u32 index = hash % BUILT_INS_CAP;

  BuiltIn *built_in = built_ins->items[index];
  while (built_in) {
    if (built_in->args_len == args_len && str_eq(built_in->name, name))
      return built_in->func;

    built_in = built_in->next;
  }

  return NULL;
}

static Value *print_built_in(Vm *vm, Value **args) {
  (void) vm;

  Value *arg = args[0];

  switch (arg->kind) {
  case ValueKindInt: {
    printf("%d\n", arg->as._int);
  } break;

  case ValueKindFloat: {
    printf("%f\n", arg->as._float);
  } break;

  case ValueKindBool: {
    printf("%s\n", arg->as._bool ? "true" : "false");
  } break;

  case ValueKindStr: {
    printf(STR_FMT"\n", STR_ARG(arg->as.str));
  } break;
  }

  return NULL;
}

static Value *wprint_built_in(Vm *vm, Value **args) {
  (void) vm;

  Value *arg = args[0];

  switch (arg->kind) {
  case ValueKindInt: {
    WARN("%d\n", arg->as._int);
  } break;

  case ValueKindFloat: {
    WARN("%f\n", arg->as._float);
  } break;

  case ValueKindBool: {
    WARN("%s\n", arg->as._bool ? "true" : "false");
  } break;

  case ValueKindStr: {
    WARN(STR_FMT"\n", STR_ARG(arg->as.str));
  } break;
  }

  return NULL;
}

static Value *eprint_built_in(Vm *vm, Value **args) {
  (void) vm;

  Value *arg = args[0];

  switch (arg->kind) {
  case ValueKindInt: {
    ERROR("%d\n", arg->as._int);
  } break;

  case ValueKindFloat: {
    ERROR("%f\n", arg->as._float);
  } break;

  case ValueKindBool: {
    ERROR("%s\n", arg->as._bool ? "true" : "false");
  } break;

  case ValueKindStr: {
    ERROR(STR_FMT"\n", STR_ARG(arg->as.str));
  } break;
  }

  return NULL;
}

void add_default_built_ins(BuiltIns *built_ins) {
  built_ins_add(built_ins, STR_LIT("print"), 1, print_built_in);
  built_ins_add(built_ins, STR_LIT("wprint"), 1, wprint_built_in);
  built_ins_add(built_ins, STR_LIT("eprint"), 1, eprint_built_in);
}
