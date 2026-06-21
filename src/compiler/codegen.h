#ifndef CODEGEN_H
#define CODEGEN_H

#include "shl/shl-defs.h"
#include "ir.h"

#define emit_proc(procs, name) \
  do {                         \
    Proc proc = { name, {} };  \
    DA_APPEND(*(procs), proc); \
  } while (0)

#define emit_instr(procs, kind, ...)                           \
  do {                                                         \
    Instr instr = { kind, { __VA_ARGS__ } };                   \
    DA_APPEND((procs)->items[(procs)->len - 1].instrs, instr); \
  } while (0)

#endif // CODEGEN_H
