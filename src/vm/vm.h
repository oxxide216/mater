#ifndef VM_H
#define VM_H

#include "shl/shl-str.h"
#include "ir.h"

typedef struct {
  Procs  procs;
  Values stack;
} Vm;

Vm *vm_create(u8 *bytecode, u32 len);

u32 vm_get_proc_index(Vm *vm, char *name, u32 name_len);

void vm_run_proc(Vm *vm, u32 index);
void vm_run_proc_named(Vm *vm, char *name, u32 name_len);

#endif // VM_H
