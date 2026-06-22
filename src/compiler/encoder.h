#ifndef ENCODER_H
#define ENCODER_H

#include <stdio.h>

#include "ir.h"

void encode_value(FILE *stream, Value *value);
void encode_procs(FILE *stream, Procs *procs);

#endif // ENCODER_H
