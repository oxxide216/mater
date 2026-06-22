#include "io.h"
#include "vm.h"
#include "shl/shl-defs.h"
#define SHL_STR_IMPLEMENTATION
#include "shl/shl-str.h"
#include "shl/shl-log.h"

void print_usage(char *program_name) {
  fprintf(stderr, "Usage: %s <input file>\n", program_name);
}

i32 main(i32 argc, char **argv) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  char *input_path = argv[1];
  Str bytecode = read_file(input_path);
  if (bytecode.len == (u32) -1) {
    ERROR("Could not read %s\n", input_path);
    return 1;
  }

  Vm *vm = vm_create((u8 *) bytecode.ptr, bytecode.len);

  vm_run_proc_named(vm, "main", 4, 0);

  vm_destroy(vm);

  return 0;
}
