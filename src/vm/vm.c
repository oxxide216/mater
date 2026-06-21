#include "vm.h"
#include "web-api.h"
#include "decoder.h"
#include "built-ins.h"
#define SHL_STR_IMPLEMENTATION
#include "shl/shl-str.h"

void built_ins_add(BuiltIns *built_ins, Str name, u32 args_len, BuiltInFunc func) {
  u32 hash = str_hash(name) + args_len;
  u32 index = hash % BUILT_INS_CAP;

  BuiltIn **next = built_ins->items + index;
  while (*next)
    next = &(*next)->next;

  *next = walloc(sizeof(BuiltIn));
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

Value *value_int(i32 _int) {
  Value *value = walloc(sizeof(Value));
  value->kind = ValueKindInt;
  value->as._int = _int;
  return value;
}

Value *value_float(f32 _float) {
  Value *value = walloc(sizeof(Value));
  value->kind = ValueKindFloat;
  value->as._float = _float;
  return value;
}

Value *value_bool(bool _bool) {
  Value *value = walloc(sizeof(Value));
  value->kind = ValueKindBool;
  value->as._bool = _bool;
  return value;
}

Value *value_str(Str str) {
  Value *value = walloc(sizeof(Value));
  value->kind = ValueKindStr;
  value->as.str = str;
  return value;
}

Vm *vm_create(u8 *bytecode, u32 len) {
  Procs procs = {0};

  if (!decode_procs(&procs, bytecode, len))
    return NULL;

  Vm *vm = walloc(sizeof(Vm));
  vm->procs = procs;
  add_default_built_ins(&vm->built_ins);
  return vm;
}

u32 vm_get_proc_index(Vm *vm, Str name) {
  for (u32 i = 0; i < vm->procs.len; ++i)
    if (str_eq(vm->procs.items[i].name, name))
      return i;

  return (u32) -1;
}

static bool stack_size_is_enough(Values *stack, u32 min) {
  // TODO: Error reporting
  return stack->len >= min;
}

void vm_run_proc(Vm *vm, u32 index) {
  // TODO: Error reporting
  if (index >= vm->procs.len)
    return;

  Proc *proc = vm->procs.items + index;
  u32 ip = 0;

  while (ip < proc->instrs.len) {
    Instr *instr = proc->instrs.items + ip++;

    switch (instr->kind) {
    case InstrKindPush: {
      // TODO: deeply clone value when I will introduce complex data types
      Value *value = walloc(sizeof(Value));
      *value = instr->as.push.value;

      da_reserve_space(&vm->stack);
      vm->stack.items[vm->stack.len++] = value;
    } break;

    case InstrKindCall: {
      // TODO: Error reporting
      if (!stack_size_is_enough(&vm->stack, instr->as.call.args_len))
        return;

      vm_run_proc_named(vm, instr->as.call.name.ptr,
                        instr->as.call.name.len,
                        instr->as.call.args_len);
    } break;

    case InstrKindOp: {
      // TODO: Error reporting
      if (!stack_size_is_enough(&vm->stack, 2))
        return;

      Value *b = vm->stack.items[--vm->stack.len];
      Value *a = vm->stack.items[--vm->stack.len];

      // TODO: Error reporting
      if ((a->kind != ValueKindInt && a->kind != ValueKindFloat) ||
          a->kind != b->kind)
        return;

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
          // TODO: Error reporting
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
          c = value_float(fmod(a->as._float, b->as._float));
        } break;

        default: {
          // TODO: Error reporting
          return;
        }
        }
      }

      vm->stack.items[vm->stack.len++] = c;
    } break;

    default: {
      // TODO: Error reporting
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

  vm_run_proc(vm, vm_get_proc_index(vm, name_str));
}
