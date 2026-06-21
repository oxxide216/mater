#ifndef PARSER_H
#define PARSER_H

#include "ir.h"
#include "lexer.h"

bool parse(Procs *procs, Str code, Str file_path);

#endif // PARSER_H
