.RECIPEPREFIX = >

CC = clang
LD = wasm-ld
override CFLAGS += -Wall -Wextra -Ilibs -Isrc/common
override LDFLAGS +=
override COMPILER_CFLAGS += $(CFLAGS) -Ilibs/lexgen/include
override COMPILER_LDFLAGS += $(LDFLAGS)
override VM_CFLAGS += $(CFLAGS) --target=wasm32 -Os \
                                -DSHL_DEFS_NO_STD -DSHL_STR_NO_STD
override VM_LDFLAGS += $(LDFLAGS) -mwasm32 --strip-all -O3 \
                                  --allow-undefined --no-entry
EXPORTS = --export=vm_create --export=vm_run_proc_named
BUILD_DIR = build

COMPILER_SRC = $(wildcard src/compiler/*.c)
VM_SRC = $(wildcard src/vm/*.c)
LEXGEN_SRC = libs/lexgen/src/common/wstr.c \
             libs/lexgen/src/runtime/runtime.c

COMPILER_OBJ = $(patsubst src/compiler/%.c,$(BUILD_DIR)/compiler/%.o,$(COMPILER_SRC))
VM_OBJ = $(patsubst src/vm/%.c,$(BUILD_DIR)/vm/%.o,$(VM_SRC))
LEXGEN_OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(LEXGEN_SRC))

all: mater mater.wasm

mater: $(COMPILER_OBJ) $(LEXGEN_OBJ)
> $(CC) -o mater $(COMPILER_OBJ) $(LEXGEN_OBJ) $(COMPILER_LDFLAGS)

mater.wasm: $(VM_OBJ)
> $(LD) -o mater.wasm $(VM_OBJ) $(VM_LDFLAGS) $(EXPORTS)

$(BUILD_DIR)/compiler/%.o: src/compiler/%.c src/compiler/grammar.h
> mkdir -p $(dir $@)
> $(CC) $(COMPILER_CFLAGS) -c -o $@ $<

$(BUILD_DIR)/vm/%.o: src/vm/%.c
> mkdir -p $(dir $@)
> $(CC) $(VM_CFLAGS) -c -o $@ $<

$(BUILD_DIR)/libs/%.o: libs/%.c
> mkdir -p $(dir $@)
> $(CC) $(COMPILER_CFLAGS) -c -o $@ $<

src/compiler/grammar.h: libs/lexgen/lexgen
> libs/lexgen/lexgen src/compiler/grammar.h grammar.lg

libs/lexgen/lexgen:
> cd libs/lexgen && ./build.sh

clean:
> rm -rf $(BUILD_DIR) mater mater.wasm
