#include "encoder.h"

void encode_value(FILE *stream, Value *value) {
  fwrite(&value->kind, 1, 1, stream);

  switch (value->kind) {
  case ValueKindInt: {
    fwrite(&value->as._int, sizeof(value->as._int), 1, stream);
  } break;

  case ValueKindFloat: {
    fwrite(&value->as._float, sizeof(value->as._float), 1, stream);
  } break;

  case ValueKindBool: {
    fwrite(&value->as._bool, 1, 1, stream);
  } break;

  case ValueKindStr: {
    fwrite(&value->as.str.len, sizeof(value->as.str.len), 1, stream);
    fwrite(value->as.str.ptr, 1, value->as.str.len, stream);
  } break;
  }
}

void encode_procs(FILE *stream, Procs *procs) {
  fwrite(&procs->len, sizeof(procs->len), 1, stream);

  for (u32 i = 0; i < procs->len; ++i) {
    Proc *proc = procs->items + i;

    fwrite(&proc->name.len, sizeof(proc->name.len), 1, stream);
    fwrite(proc->name.ptr, 1, proc->name.len, stream);
    fwrite(&proc->instrs.len, sizeof(proc->instrs.len), 1, stream);

    for (u32 j = 0; j < proc->instrs.len; ++j) {
      Instr *instr = proc->instrs.items + j;

      fwrite(&instr->kind, 1, 1, stream);

      switch (instr->kind) {
      case InstrKindPush: {
        encode_value(stream, &instr->as.push.value);
      } break;

      case InstrKindCall: {
        fwrite(&instr->as.call.name.len, sizeof(instr->as.call.name.len), 1, stream);
        fwrite(instr->as.call.name.ptr, 1, instr->as.call.name.len, stream);
        fwrite(&instr->as.call.args_len, sizeof(instr->as.call.args_len), 1, stream);
      } break;

      case InstrKindOp: {
        fwrite(&instr->as.op.kind, 1, 1, stream);
      } break;
      }
    }
  }
}
