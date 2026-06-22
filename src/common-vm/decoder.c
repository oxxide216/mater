#include "decoder.h"
#ifdef __wasm__
#include "web-api.h"
#endif

#ifdef __wasm__
#define ALLOC walloc
#else
#define ALLOC malloc
#endif

#define decode_buffer(decoder, buffer, len)        \
  do {                                             \
    if (!decode_buffer_impl(decoder, buffer, len)) \
      return false;                                \
  } while (0)

#define decode_value(decoder, value)        \
  do {                                      \
    if (!decode_value_impl(decoder, value)) \
      return false;                         \
  } while (0)

typedef struct {
  Procs *procs;
  u8    *bytecode;
  u32    len, decoded;
} Decoder;

static bool decode_buffer_impl(Decoder *decoder, void *buffer, u32 len) {
  if (decoder->decoded + len > decoder->len)
    return false;

  for (u32 i = 0; i < len; ++i)
    ((u8 *) buffer)[i] = decoder->bytecode[decoder->decoded + i];

  decoder->decoded += len;

  return true;
}

static bool decode_value_impl(Decoder *decoder, Value *value) {
  u8 kind;

  decode_buffer(decoder, &kind, 1);
  value->kind = kind;

  switch (value->kind) {
  case ValueKindInt: {
    decode_buffer(decoder, &value->as._int, sizeof(value->as._int));
  } break;

  case ValueKindFloat: {
    decode_buffer(decoder, &value->as._float, sizeof(value->as._float));
  } break;

  case ValueKindBool: {
    u8 _bool;
    decode_buffer(decoder, &_bool, 1);
    value->as._bool = _bool;
  } break;

  case ValueKindStr: {
    decode_buffer(decoder, &value->as.str.len, sizeof(value->as.str.len));
    value->as.str.ptr = ALLOC(value->as.str.len);
    decode_buffer(decoder, value->as.str.ptr, value->as.str.len);
  } break;
  }

  return true;
}

bool decode_procs(Procs *procs, u8 *bytecode, u32 len) {
  Decoder decoder = { procs, bytecode, len, 0 };

  decode_buffer(&decoder, &decoder.procs->len, sizeof(decoder.procs->len));
  decoder.procs->cap = decoder.procs->len;
  decoder.procs->items = ALLOC(sizeof(Proc) * decoder.procs->cap);

  for (u32 i = 0; i < decoder.procs->len; ++i) {
    Proc *proc = decoder.procs->items + i;

    decode_buffer(&decoder, &proc->name.len, sizeof(proc->name.len));
    proc->name.ptr = ALLOC(proc->name.len);
    decode_buffer(&decoder, proc->name.ptr, proc->name.len);

    decode_buffer(&decoder, &proc->instrs.len, sizeof(proc->instrs.len));
    proc->instrs.cap = proc->instrs.len;
    proc->instrs.items = ALLOC(sizeof(Instr) * proc->instrs.cap);

    for (u32 j = 0; j < proc->instrs.len; ++j) {
      Instr *instr = proc->instrs.items + j;
      u8 kind;

      decode_buffer(&decoder, &kind, 1);
      instr->kind = kind;

      switch (instr->kind) {
      case InstrKindPush: {
        decode_value(&decoder, &instr->as.push.value);
      } break;

      case InstrKindCall: {
        decode_buffer(&decoder, &instr->as.call.name.len, sizeof(instr->as.call.name.len));
        instr->as.call.name.ptr = ALLOC(instr->as.call.name.len);
        decode_buffer(&decoder, instr->as.call.name.ptr, instr->as.call.name.len);
        decode_buffer(&decoder, &instr->as.call.args_len, sizeof(instr->as.call.args_len));
      } break;

      case InstrKindOp: {
        decode_buffer(&decoder, &kind, 1);
        instr->as.op.kind = kind;
      } break;
      }
    }
  }

  return true;
}
