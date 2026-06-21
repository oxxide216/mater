#include "ir-print.h"

void print_value(Value *value) {
  switch (value->kind) {
  case ValueKindInt: {
    printf("%d", value->as._int);
  } break;

  case ValueKindFloat: {
    printf("%f", value->as._float);
  } break;

  case ValueKindBool: {
    printf("%s", value->as._bool ? "true" : "false");
  } break;

  case ValueKindStr: {
    printf("\""STR_FMT"\"", STR_ARG(value->as.str));
  } break;
  }
}

void print_procs(Procs *procs) {
  for (u32 i = 0; i < procs->len; ++i) {
    Proc *proc = procs->items + i;

    printf("proc "STR_FMT"()\n", STR_ARG(proc->name));

    for (u32 j = 0; j < proc->instrs.len; ++j) {
      Instr *instr = proc->instrs.items + j;

      switch (instr->kind) {
      case InstrKindPush: {
        printf("  PUSH ");
        print_value(&instr->as.push.value);
        printf("\n");
      } break;

      case InstrKindCall: {
        printf("  CALL "STR_FMT" %u\n",
               STR_ARG(instr->as.call.name),
               instr->as.call.args_len);
      } break;

      case InstrKindOp: {
        switch (instr->as.op.kind) {
        case OpKindAdd: {
          printf("  ADD\n");
        } break;

        case OpKindSub: {
          printf("  SUB\n");
        } break;

        case OpKindMul: {
          printf("  MUL\n");
        } break;

        case OpKindDiv: {
          printf("  DIV\n");
        } break;

        case OpKindRem: {
          printf("  REM\n");
        } break;
        }
      } break;
      }
    }

    printf("end\n");
  }
}
