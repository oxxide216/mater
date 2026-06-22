.RECIPEPREFIX = >

CC = clang
LD = wasm-ld
override CFLAGS += -Wall -Wextra -Ilibs -Isrc/common
override LDFLAGS +=
override LINUX_CFLAGS += $(CFLAGS) -Ilibs/lexgen/include
override LINUX_LDFLAGS += $(LDFLAGS)
override WASM_CFLAGS += $(CFLAGS) --target=wasm32 -Os \
                                  -DSHL_DEFS_NO_STD -DSHL_STR_NO_STD
override WASM_LDFLAGS += $(LDFLAGS) -mwasm32 --strip-all -O3 \
                                    --allow-undefined --no-entry
EXPORTS = --export=vm_create --export=vm_run_proc_named
BUILD_DIR = build

MATER_TEST_SRC = tests/basic.mtr

COMPILER_SRC = $(wildcard src/compiler/*.c)
VM_SRC = $(wildcard src/vm/*.c)
COMMON_SRC = $(wildcard src/common/*.c)
LEXGEN_SRC = libs/lexgen/src/common/wstr.c \
             libs/lexgen/src/runtime/runtime.c

COMPILER_OBJ = $(patsubst src/compiler/%.c,$(BUILD_DIR)/linux/compiler/%.o,$(COMPILER_SRC))
VM_OBJ = $(patsubst src/vm/%.c,$(BUILD_DIR)/wasm/vm/%.o,$(VM_SRC))
COMMON_LINUX_OBJ = $(patsubst src/common/%.c,$(BUILD_DIR)/linux/common/%.o,$(COMMON_SRC))
COMMON_WASM_OBJ = $(patsubst src/common/%.c,$(BUILD_DIR)/wasm/common/%.o,$(COMMON_SRC))
LEXGEN_OBJ = $(patsubst %.c,$(BUILD_DIR)/linux/%.o,$(LEXGEN_SRC))

all: mater mater.wasm

mater: $(COMPILER_OBJ) $(COMMON_LINUX_OBJ) $(LEXGEN_OBJ)
> $(CC) -o mater $(COMPILER_OBJ) $(COMMON_LINUX_OBJ) $(LEXGEN_OBJ) $(LINUX_LDFLAGS)

mater.wasm: $(VM_OBJ) $(COMMON_WASM_OBJ)
> $(LD) -o mater.wasm $(VM_OBJ) $(COMMON_WASM_OBJ) $(WASM_LDFLAGS) $(EXPORTS)

$(BUILD_DIR)/linux/%.o: src/%.c src/compiler/grammar.h
> mkdir -p $(dir $@)
> $(CC) $(LINUX_CFLAGS) -c -o $@ $<

$(BUILD_DIR)/wasm/%.o: src/%.c
> mkdir -p $(dir $@)
> $(CC) $(WASM_CFLAGS) -c -o $@ $<

$(BUILD_DIR)/linux/libs/%.o: libs/%.c
> mkdir -p $(dir $@)
> $(CC) $(LINUX_CFLAGS) -c -o $@ $<

src/compiler/grammar.h: libs/lexgen/lexgen
> libs/lexgen/lexgen src/compiler/grammar.h grammar.lg

libs/lexgen/lexgen:
> cd libs/lexgen && ./build.sh

run: all
> ./mater $(MATER_TEST_SRC)
> ./mater-deploy $(patsubst tests/%.mtr,%.mbc,$(MATER_TEST_SRC)) dest
> cd dest && python -m http.server 8080

clean:
> rm -rf $(BUILD_DIR) mater mater.wasm
