.RECIPEPREFIX = >

CC = clang
LD = wasm-ld
override CFLAGS += -Wall -Wextra -Ilibs -Isrc/common -Isrc/common-vm -Isrc/common-native
override LDFLAGS +=
override LINUX_CFLAGS += $(CFLAGS) -Ilibs/lexgen/include
override LINUX_LDFLAGS += $(LDFLAGS) -lm
override WASM_CFLAGS += $(CFLAGS) -Isrc/vm --target=wasm32 -Os -fno-builtin \
                                  -DSHL_DEFS_NO_STD -DSHL_STR_NO_STD
override WASM_LDFLAGS += $(LDFLAGS) -mwasm32 --strip-all -O3 \
                                    --allow-undefined --no-entry
EXPORTS = --export=vm_create --export=vm_destroy --export=vm_run_proc_named
BUILD_DIR = build

COMPILER_SRC = $(wildcard src/compiler/*.c)
VM_SRC = $(wildcard src/vm/*.c)
NVM_SRC = $(wildcard src/nvm/*.c)
COMMON_SRC = $(wildcard src/common/*.c)
COMMON_VM_SRC = $(wildcard src/common-vm/*.c)
COMMON_NATIVE_SRC = $(wildcard src/common-native/*.c)
LEXGEN_SRC = libs/lexgen/src/common/wstr.c \
             libs/lexgen/src/runtime/runtime.c

COMPILER_OBJ = $(patsubst src/compiler/%.c,$(BUILD_DIR)/linux/compiler/%.o,$(COMPILER_SRC))
VM_OBJ = $(patsubst src/vm/%.c,$(BUILD_DIR)/wasm/vm/%.o,$(VM_SRC))
NVM_OBJ = $(patsubst src/nvm/%.c,$(BUILD_DIR)/linux/nvm/%.o,$(NVM_SRC))
COMMON_LINUX_OBJ = $(patsubst src/common/%.c,$(BUILD_DIR)/linux/common/%.o,$(COMMON_SRC))
COMMON_WASM_OBJ = $(patsubst src/common/%.c,$(BUILD_DIR)/wasm/common/%.o,$(COMMON_SRC))
COMMON_VM_LINUX_OBJ = $(patsubst src/common-vm/%.c,$(BUILD_DIR)/linux/common-vm/%.o,$(COMMON_VM_SRC))
COMMON_VM_WASM_OBJ = $(patsubst src/common-vm/%.c,$(BUILD_DIR)/wasm/common-vm/%.o,$(COMMON_VM_SRC))
COMMON_NATIVE_OBJ = $(patsubst src/common-native/%.c,$(BUILD_DIR)/linux/common-native/%.o,$(COMMON_NATIVE_SRC))
LEXGEN_OBJ = $(patsubst %.c,$(BUILD_DIR)/linux/%.o,$(LEXGEN_SRC))

MATER_TEST_SRC = tests/basic.mtr
MATER_TEST_BC = $(patsubst tests/%.mtr,%.mbc,$(MATER_TEST_SRC))

all: materc mater mater.wasm

materc: $(COMPILER_OBJ) $(COMMON_LINUX_OBJ) $(COMMON_NATIVE_OBJ) $(LEXGEN_OBJ)
> $(CC) -o materc $(COMPILER_OBJ) $(COMMON_LINUX_OBJ) $(COMMON_NATIVE_OBJ) $(LEXGEN_OBJ) $(LINUX_LDFLAGS)

mater: $(NVM_OBJ) $(COMMON_LINUX_OBJ) $(COMMON_VM_LINUX_OBJ) $(COMMON_NATIVE_OBJ)
> $(CC) -o mater $(NVM_OBJ) $(COMMON_LINUX_OBJ) $(COMMON_VM_LINUX_OBJ) $(COMMON_NATIVE_OBJ) $(LINUX_LDFLAGS)

mater.wasm: $(VM_OBJ) $(COMMON_WASM_OBJ) $(COMMON_VM_WASM_OBJ)
> $(LD) -o mater.wasm $(VM_OBJ) $(COMMON_WASM_OBJ) $(COMMON_VM_WASM_OBJ) $(WASM_LDFLAGS) $(EXPORTS)

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

run: materc mater
> ./materc $(MATER_TEST_SRC)
> ./mater $(MATER_TEST_BC)

run-web: materc mater.wasm
> ./materc $(MATER_TEST_SRC)
> ./mater-deploy $(MATER_TEST_BC) dest
> cd dest && python -m http.server 8080

clean:
> rm -rf $(BUILD_DIR) materc mater mater.wasm *.mbc dest
