#include "parser.h"
#include "shl/shl-log.h"
#include "grammar.h"
#include "codegen.h"

#define MASK(id) (1lu << (id))

#define parser_expect_token(parser, mask, expected)   \
  do {                                                \
    parser_expect_token_impl(parser, mask, expected); \
    if ((parser)->has_error)                          \
      return;                                         \
  } while (0)

typedef struct {
  Procs       *procs;
  Lexer        lexer;
  TokenStatus  status;
  Token        token;
  Str          file_path;
  bool         has_error;
} Parser;

TokenStatus parser_peek_token(Parser *parser, Token *token) {
  if (token)
    *token = parser->token;
  return parser->status;
}

TokenStatus parser_next_token(Parser *parser, Token *token) {
  TokenStatus status = parser->status;
  if (token)
    *token = parser->token;
  while ((parser->status = lexer_lex(&parser->lexer,
                                     &parser->token,
                                     parser->file_path)) == TokenStatusEmpty);
  return status;
}

void parser_expect_token_impl(Parser *parser, u64 mask, char *expected) {
  Token token;
  TokenStatus status = parser_next_token(parser, &token);

  if (status == TokenStatusEOF) {
    PERROR(STR_FMT": ", "Unexpected EOF, expected %s\n",
           STR_ARG(parser->file_path), expected);
    parser->has_error = true;
    return;
  }

  if (!(MASK(token.id) & mask)) {
    PERROR(STR_FMT":%u:%u: ", "Unexpected `"STR_FMT"`, expected %s\n",
           STR_ARG(parser->file_path), token.row + 1,
           token.col + 1, STR_ARG(parser->token.lexeme), expected);
    parser->has_error = true;
  }
}

static void parser_parse_primary_expr(Parser *parser) {
  Token token;
  parser_peek_token(parser, &token);
  parser_expect_token(parser,
                      MASK(TT_INT) | MASK(TT_FLOAT) |
                      MASK(TT_BOOL) | MASK(TT_STR),
                      "int, float, bool or string");

  switch (token.id) {
  case TT_INT: {
    emit_instr(
      parser->procs,
      InstrKindPush,
      .push = {
        {
          ValueKindInt,
          { ._int = str_to_i32(token.lexeme) },
        },
      },
    );
  } break;

  case TT_FLOAT: {
    emit_instr(
      parser->procs,
      InstrKindPush,
      .push = {
        {
          ValueKindFloat,
          { ._float = str_to_f32(token.lexeme) },
        },
      },
    );
  } break;

  case TT_BOOL: {
    emit_instr(
      parser->procs,
      InstrKindPush,
      .push = {
        {
          ValueKindBool,
          { ._bool = str_eq(token.lexeme, STR_LIT("true")) },
        },
      },
    );
  } break;

  case TT_STR: {
    Str str = {
      token.lexeme.ptr + 1,
      token.lexeme.len - 2,
    };
    emit_instr(
      parser->procs,
      InstrKindPush,
      .push = {
        {
          ValueKindStr,
          { .str = str },
        },
      },
    );
  } break;
  }
}

static void parser_parse_mul(Parser *parser) {
  parser_parse_primary_expr(parser);

  Token token;
  parser_peek_token(parser, &token);

  if (token.id != TT_STAR && token.id != TT_SLASH && token.id != TT_PERC)
    return;

  parser_next_token(parser, NULL);

  parser_parse_primary_expr(parser);

  emit_instr(
    parser->procs,
    InstrKindOp,
    .op = {
      token.id == TT_STAR ? OpKindMul :
        token.id == TT_SLASH ? OpKindDiv : OpKindRem,
    },
  );
}

static void parser_parse_add(Parser *parser) {
  parser_parse_mul(parser);

  Token token;
  parser_peek_token(parser, &token);

  if (token.id != TT_PLUS && token.id != TT_MINUS)
    return;

  parser_next_token(parser, NULL);

  parser_parse_mul(parser);

  emit_instr(
    parser->procs,
    InstrKindOp,
    .op = {
      token.id == TT_PLUS ? OpKindAdd : OpKindSub,
    },
  );
}

static void parser_parse_expr(Parser *parser) {
  parser_parse_add(parser);
}

static void parser_parse_stmt(Parser *parser) {
  Token token;
  parser_peek_token(parser, &token);
  parser_expect_token(parser, MASK(TT_IDENT), "identifier");
  Str name = token.lexeme;
  parser_expect_token(parser, MASK(TT_OPAREN), "`(`");

  u32 args_len = 0;

  while (parser_peek_token(parser, &token) != TokenStatusEOF &&
         token.id != TT_CPAREN) {
    ++args_len;
    parser_parse_expr(parser);
    if (parser_peek_token(parser, &token) == TokenStatusEOF ||  token.id != TT_CPAREN)
      parser_expect_token(parser, MASK(TT_COMMA) | MASK(TT_CPAREN), "`,` or `)`");
  }

  emit_instr(
    parser->procs,
    InstrKindCall,
    .call = { name, args_len },
  );

  parser_expect_token(parser, MASK(TT_CPAREN), "`)`");
}

static void parser_parse_proc(Parser *parser) {
  Token token;
  parser_peek_token(parser, &token);
  parser_expect_token(parser, MASK(TT_IDENT), "identifier");
  Str name = token.lexeme;
  parser_expect_token(parser, MASK(TT_OPAREN), "`(`");
  parser_expect_token(parser, MASK(TT_CPAREN), "`)`");

  emit_proc(parser->procs, name);

  while (parser_peek_token(parser, &token) != TokenStatusEOF &&
         token.id != TT_END) {
    parser_parse_stmt(parser);
  }

  parser_expect_token(parser, MASK(TT_END), "`end`");
}

static void parser_parse_program(Parser *parser) {
  Token token;
  while (parser_peek_token(parser, &token) != TokenStatusEOF) {
    parser_expect_token(parser, MASK(TT_PROC), "`proc`");

    switch (token.id) {
    case TT_PROC: {
      parser_parse_proc(parser);
    } break;
    }
  }
}

bool parse(Procs *procs, Str code, Str file_path) {
  Parser parser = {
    procs,
    lexer_create(code),
    TokenStatusOk,
    {},
    file_path,
    false,
  };

  parser_next_token(&parser, NULL);
  parser_parse_program(&parser);

  lexer_destroy(&parser.lexer);

  return !parser.has_error;
}
