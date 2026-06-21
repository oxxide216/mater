#ifndef VM_H
#define VM_H

#include "shl/shl-str.h"
#include "ir.h"

#define BUILT_INS_CAP 40

typedef struct Vm Vm;

typedef Value *(*BuiltInFunc)(Vm *vm, Value **args);

typedef struct BuiltIn BuiltIn;

struct BuiltIn {
  Str          name;
  u32          args_len;
  BuiltInFunc  func;
  BuiltIn     *next;
};

typedef struct {
  BuiltIn *items[BUILT_INS_CAP];
} BuiltIns;

struct Vm {
  Values   stack;
  Procs    procs;
  BuiltIns built_ins;
};

void        built_ins_add(BuiltIns *built_ins, Str name, u32 args_len, BuiltInFunc func);
BuiltInFunc built_ins_get(BuiltIns *built_ins, Str name, u32 args_len);

Value *value_int(i32 _int);
Value *value_float(f32 _float);
Value *value_bool(bool _bool);
Value *value_str(Str str);

Vm *vm_create(u8 *bytecode, u32 len);

u32 vm_get_proc_index(Vm *vm, Str name);

void vm_run_proc(Vm *vm, u32 index);
void vm_run_proc_named(Vm *vm, char *name, u32 name_len, u32 args_len);

#endif // VM_H
