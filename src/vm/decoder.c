#include "decoder.h"
#include "web-api.h"

bool decode_procs(Procs *procs, u8 *bytecode, u32 len) {
  (void) bytecode;
  (void) len;

  procs->len = 1;
  procs->cap = 1;
  procs->items = walloc(sizeof(Proc));

  procs->items[0] = (Proc) {
    STR_LIT("init"),
    {
      walloc(sizeof(Instr) * 4),
      4,
      4,
    },
  };

  procs->items[0].instrs.items[0] = (Instr) {
    InstrKindPush,
    {
      .push = {
        ValueKindInt,
        {
          ._int = 420,
        },
      },
    },
  };
  procs->items[0].instrs.items[1] = (Instr) {
    InstrKindPush,
    {
      .push = {
        ValueKindInt,
        {
          ._int = 246,
        },
      },
    },
  };
  procs->items[0].instrs.items[2] = (Instr) {
    InstrKindOp,
    {
      .op = {
        OpKindAdd,
      },
    },
  };
  procs->items[0].instrs.items[3] = (Instr) {
    InstrKindCall,
    {
      .call = {
        STR_LIT("print"),
        1,
      },
    },
  };

  return true;
}
