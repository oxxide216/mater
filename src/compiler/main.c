#include "parser.h"
#ifndef NDEBUG
#include "ir-print.h"
#endif
#include "encoder.h"
#include "io.h"
#include "shl/shl-defs.h"
#define SHL_STR_IMPLEMENTATION
#include "shl/shl-str.h"
#include "shl/shl-log.h"

void print_usage(char *program_name) {
  fprintf(stderr, "Usage: %s <input file>\n", program_name);
}

char *make_output_path(char *input_path) {
  u32 len = strlen(input_path);
  u32 begin = len;
  while (begin > 0 && input_path[begin - 1] != '/')
    --begin;

  input_path += begin;
  len -= begin;

  if (len > 4 && strcmp(input_path + len - 4, ".mtr") == 0) {
    char *result = malloc(len);
    memcpy(result, input_path, len - 4);
    strcpy(result + len - 4, ".mbc");
    return result;
  } else {
    char *result = malloc(len + 5);
    strcpy(result, input_path);
    strcpy(result + len, ".mbc");
    return result;
  }
}

i32 main(i32 argc, char **argv) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  char *input_path = argv[1];

  Str code = read_file(input_path);
  if (code.len == (u32) -1) {
    ERROR("Could not read %s\n", input_path);
    return 1;
  }

  Str input_path_str = str_new(input_path);
  Procs procs = {0};
  if (!parse(&procs, code, input_path_str))
    return 1;

  char *output_path = make_output_path(input_path);
  FILE *output_file = fopen(output_path, "wb");
  if (!output_file) {
    ERROR("Could not write %s\n", output_path);
    free(output_path);
    return 1;
  }

#ifndef NDEBUG
  print_procs(&procs);
#endif

  encode_procs(output_file, &procs);

  fclose(output_file);
  free(output_path);

  return 0;
}
