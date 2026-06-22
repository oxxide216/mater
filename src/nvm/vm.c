#include <math.h>

#include "vm.h"
#include "decoder.h"
#include "built-ins.h"
#include "shl/shl-log.h"

Value *value_int(i32 _int) {
  Value *value = malloc(sizeof(Value));
  value->kind = ValueKindInt;
  value->as._int = _int;
  return value;
}

Value *value_float(f32 _float) {
  Value *value = malloc(sizeof(Value));
  value->kind = ValueKindFloat;
  value->as._float = _float;
  return value;
}

Value *value_bool(bool _bool) {
  Value *value = malloc(sizeof(Value));
  value->kind = ValueKindBool;
  value->as._bool = _bool;
  return value;
}

Value *value_str(Str str) {
  Value *value = malloc(sizeof(Value));
  value->kind = ValueKindStr;
  value->as.str = str;
  return value;
}

char *get_value_kind_str(Value *value) {
  switch (value->kind) {
  case ValueKindInt:   return "int";
  case ValueKindFloat: return "float";
  case ValueKindBool:  return "bool";
  case ValueKindStr:   return "str";
  }

  return NULL;
}

Vm *vm_create(u8 *bytecode, u32 len) {
  Procs procs = {0};

  if (!decode_procs(&procs, bytecode, len))
    return NULL;

  Vm *vm = malloc(sizeof(Vm));
  *vm = (Vm) {0};
  vm->procs = procs;
  add_default_built_ins(&vm->built_ins);
  return vm;
}

void vm_destroy(Vm *vm) {
  // TODO: Deeply free complex types
  if (vm->stack.items)
    free(vm->stack.items);

  // TODO: Deeply free complex types in instructions
  for (u32 i = 0; i < vm->procs.len; ++i)
    if (vm->procs.items[i].instrs.items)
      free(vm->procs.items[i].instrs.items);
  if (vm->procs.items)
    free(vm->procs.items);

  free(vm);
}

u32 vm_get_proc_index(Vm *vm, Str name) {
  for (u32 i = 0; i < vm->procs.len; ++i)
    if (str_eq(vm->procs.items[i].name, name))
      return i;

  return (u32) -1;
}

static bool stack_size_is_enough(Values *stack, u32 min) {
  if (stack->len >= min)
    return true;

  ERROR("Stack size is not enough: %u/%u", stack->len, min);

  return false;
}

void vm_run_proc(Vm *vm, u32 index) {
  if (index >= vm->procs.len) {
    ERROR("Procedure index is out of bounds: %u/%u", index, vm->procs.len);
    return;
  }

  Proc *proc = vm->procs.items + index;
  u32 ip = 0;

  while (ip < proc->instrs.len) {
    Instr *instr = proc->instrs.items + ip++;

    switch (instr->kind) {
    case InstrKindPush: {
      // TODO: deeply clone value when I will introduce complex data types
      Value *value = malloc(sizeof(Value));
      *value = instr->as.push.value;

      DA_APPEND(vm->stack, value);
    } break;

    case InstrKindCall: {
      if (!stack_size_is_enough(&vm->stack, instr->as.call.args_len))
        return;

      vm_run_proc_named(vm, instr->as.call.name.ptr,
                        instr->as.call.name.len,
                        instr->as.call.args_len);
    } break;

    case InstrKindOp: {
      if (!stack_size_is_enough(&vm->stack, 2))
        return;

      Value *b = vm->stack.items[--vm->stack.len];
      Value *a = vm->stack.items[--vm->stack.len];

      if ((a->kind != ValueKindInt && a->kind != ValueKindFloat) ||
          a->kind != b->kind) {
        ERROR("Incorrect types for binary operation: %s and %s",
              get_value_kind_str(a), get_value_kind_str(b));
        return;
      }

      Value *c;

      if (a->kind == ValueKindInt) {
        switch (instr->as.op.kind) {
        case OpKindAdd: {
          c = value_int(a->as._int + b->as._int);
        } break;

        case OpKindSub: {
          c = value_int(a->as._int - b->as._int);
        } break;

        case OpKindMul: {
          c = value_int(a->as._int * b->as._int);
        } break;

        case OpKindDiv: {
          c = value_int(a->as._int / b->as._int);
        } break;

        case OpKindRem: {
          c = value_int(a->as._int % b->as._int);
        } break;

        default: {
          ERROR("Incorrect binary operation: %u", instr->as.op.kind);
          return;
        }
        }
      } else {
        switch (instr->as.op.kind) {
        case OpKindAdd: {
          c = value_float(a->as._float + b->as._float);
        } break;

        case OpKindSub: {
          c = value_float(a->as._float - b->as._float);
        } break;

        case OpKindMul: {
          c = value_float(a->as._float * b->as._float);
        } break;

        case OpKindDiv: {
          c = value_float(a->as._float / b->as._float);
        } break;

        case OpKindRem: {
          c = value_float(fmodf(a->as._float, b->as._float));
        } break;

        default: {
          ERROR("Incorrect binary operation: %u", instr->as.op.kind);
          return;
        }
        }
      }

      vm->stack.items[vm->stack.len++] = c;
    } break;

    default: {
      ERROR("Incorrect instruction: %u", instr->kind);
      return;
    }
    }
  }
}

void vm_run_proc_named(Vm *vm, char *name, u32 name_len, u32 args_len) {
  Str name_str = { name, name_len };
  BuiltInFunc built_in = built_ins_get(&vm->built_ins, name_str, args_len);

  if (built_in) {
    built_in(vm, vm->stack.items + vm->stack.len - args_len);
    return;
  }

  u32 index = vm_get_proc_index(vm, name_str);

  if (index == (u32) -1) {
    ERROR("Attemp to call an undefined procedure `%.*s`\n", name_len, name);
    return;
  }

  vm_run_proc(vm, index);
}
