GCC := /opt/cross/bin/i686-elf
CFLAGS := -O0 -nostdlib -nostartfiles -ffreestanding -I usr/include

LIBC_DIR := libc
LIBC_A := lib/libc.a

CRT0 := $(LIBC_DIR)/crt0.c

LDSCRIPT := $(LIBC_DIR)/linker.ld

LIBC_SRCS := $(LIBC_DIR)/string.c $(LIBC_DIR)/stdio.c $(LIBC_DIR)/stdlib.c $(LIBC_DIR)/process.c
LIBC_OBJS := $(LIBC_SRCS:.c=.o)

INIT_SRCS := $(shell find ./src/init -type f -name "*.c")
INIT_OBJS := $(patsubst ./src/init/%.c,./bin/init/%.o,$(INIT_SRCS))
INIT_BIN := bin/init/init

TEST_SRCS := $(shell find ./src/test -type f -name "*.c")
TEST_OBJS := $(patsubst ./src/test/%.c,./bin/test/%.o,$(TEST_SRCS))
TEST_BIN := bin/test/test

OBJS := $(INIT_OBJS) $(TEST_OBJS)
BINS := $(INIT_BIN) $(TEST_BIN)

.PHONY: all libc userspace clean

all: libc userspace

libc: $(LIBC_A)

$(LIBC_A): $(LIBC_OBJS)
	mkdir -p lib
	$(GCC)-ar rcs $@ $^

$(LIBC_DIR)/%.o: $(LIBC_DIR)/%.c
	$(GCC)-gcc $(CFLAGS) -c -o $@ $<

userspace: $(BINS)

$(INIT_BIN): $(INIT_OBJS) $(LIBC_A)
	mkdir -p $(dir $@)
	$(GCC)-gcc $(CFLAGS) -T $(LDSCRIPT) -o $@ $(CRT0) $(INIT_OBJS) $(LIBC_A)

$(TEST_BIN): $(TEST_OBJS) $(LIBC_A)
	mkdir -p $(dir $@)
	$(GCC)-gcc $(CFLAGS) -T $(LDSCRIPT) -o $@ $(CRT0) $(TEST_OBJS) $(LIBC_A)

bin/init/%.o: src/init/%.c
	mkdir -p $(dir $@)
	$(GCC)-gcc $(CFLAGS) -c -o $@ $<

bin/test/%.o: src/test/%.c
	mkdir -p $(dir $@)
	$(GCC)-gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(LIBC_OBJS) $(LIBC_A)
	rm -f $(BINS) $(OBJS)