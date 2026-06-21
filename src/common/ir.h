#ifndef IR_H
#define IR_H

#include "shl/shl-defs.h"
#include "shl/shl-str.h"

typedef enum {
  ValueKindInt = 0,
  ValueKindFloat,
  ValueKindBool,
  ValueKindStr,
} ValueKind;

typedef union {
  i32  _int;
  f32  _float;
  bool _bool;
  Str  str;
} ValueAs;

typedef struct {
  ValueKind kind;
  ValueAs   as;
} Value;

typedef Da(Value *) Values;

typedef enum {
  InstrKindPush = 0,
  InstrKindCall,
} InstrKind;

typedef struct {
  Value value;
} InstrPush;

typedef struct {
  Str name;
  u32 args_len;
} InstrCall;

typedef union {
  InstrPush push;
  InstrCall call;
} InstrAs;

typedef struct {
  InstrKind kind;
  InstrAs   as;
} Instr;

typedef Da(Instr) Instrs;

typedef struct {
  Str    name;
  Instrs instrs;
} Proc;

typedef Da(Proc) Procs;

#endif // IR_H
