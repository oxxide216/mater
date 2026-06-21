#ifndef DECODER_H
#define DECODER_H

#include "ir.h"

bool decode_procs(Procs *procs, u8 *bytecode, u32 len);

#endif // DECODER_H
