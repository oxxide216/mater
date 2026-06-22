#ifndef BUILT_INS_H
#define BUILT_INS_H

#include "vm.h"

void        built_ins_add(BuiltIns *built_ins, Str name, u32 args_len, BuiltInFunc func);
BuiltInFunc built_ins_get(BuiltIns *built_ins, Str name, u32 args_len);

void add_default_built_ins(BuiltIns *built_ins);

#endif // BUILT_INS_H
