#include "vm.h"
#include "web-api.h"
#include "decoder.h"
#define SHL_STR_IMPLEMENTATION
#include "shl/shl-str.h"

Vm *vm_create(u8 *bytecode, u32 len) {
  Procs procs = {0};

  if (!decode_procs(&procs, bytecode, len))
    return NULL;

  Vm *vm = walloc(sizeof(Vm));
  vm->procs = procs;
  return vm;
}

u32 vm_get_proc_index(Vm *vm, char *name, u32 name_len) {
  Str name_str = { name, name_len };

  for (u32 i = 0; i < vm->procs.len; ++i)
    if (str_eq(vm->procs.items[i].name, name_str))
      return i;

  return (u32) -1;
}

static bool stack_size_is_enough(Values *stack, u32 min) {
  return stack->len >= min;
}

void vm_run_proc(Vm *vm, u32 index) {
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
      if (!stack_size_is_enough(&vm->stack, instr->as.call.args_len))
        return;

      vm_run_proc_named(vm, instr->as.call.name.ptr, instr->as.call.name.len);
    } break;

    default: {
      return;
    }
    }
  }
}

void vm_run_proc_named(Vm *vm, char *name, u32 name_len) {
  vm_run_proc(vm, vm_get_proc_index(vm, name, name_len));
}
