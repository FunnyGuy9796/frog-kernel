GCC := /opt/cross/bin/i686-elf
C_SRCS := $(shell find ./src -type f -name "*.c")
S_SRCS := $(shell find ./src -type f -name "*.s")
C_OBJS := $(patsubst ./src/%.c,./build/%.o,$(C_SRCS))
S_OBJS := $(patsubst ./src/%.s,./build/%.o,$(S_SRCS))

DEBUG ?= 1

NAME := frog

ifeq ($(DEBUG), 1)
	CFLAGS := -std=gnu99 -ffreestanding -O0 -g -Wall -Wextra -fno-omit-frame-pointer
	LDFLAGS := -ffreestanding -O0 -nostdlib -g
else
	CFLAGS := -std=gnu99 -ffreestanding -O2 -Wall -Wextra -fno-omit-frame-pointer
	LDFLAGS := -ffreestanding -O2 -nostdlib
endif

.PHONY: all clean run iso

all: iso

./build/%.o: ./src/%.c
	mkdir -p $(dir $@)
	$(GCC)-gcc -c $< -o $@ $(CFLAGS)

./build/%.o: ./src/%.s
	mkdir -p $(dir $@)
	$(GCC)-as $< -o $@

$(NAME): $(S_OBJS) $(C_OBJS)
	$(GCC)-gcc -o $@.tmp $(LDFLAGS) -Wl,-T,linker.ld -Wl,-Map,$(NAME).map $^ -lgcc
	mkdir -p build
	python3 tools/gen_symtable.py $@.tmp > build/symtable.c
	$(GCC)-gcc -c build/symtable.c -o build/symtable_real.o $(CFLAGS)
	$(GCC)-gcc -o $@ $(LDFLAGS) -Wl,-T,linker.ld \
		$(filter-out build/ksym/symtable_stub.o, $^) \
		build/symtable_real.o -lgcc
	rm $@.tmp

iso: $(NAME)
	@if grub-file --is-x86-multiboot2 $(NAME); then \
		echo "multiboot confirmed"; \
	else \
		echo "the file is not multiboot"; \
		exit 1; \
	fi
	$(MAKE) -C initrd all
	tar -cvf initrd.tar -C initrd \
		bin \
		libc \
		usr \
		dev \
		hello.txt
	mkdir -p isodir/boot/grub
	cp $(NAME) isodir/boot/$(NAME)
	cp grub.cfg isodir/boot/grub/grub.cfg
	cp initrd.tar isodir/boot/initrd.tar
	grub-mkrescue -o $(NAME).iso isodir

run: iso
	qemu-system-i386 -cdrom $(NAME).iso -serial stdio -m 4G \
		-device VGA,vgamem_mb=16 -display sdl,gl=on -accel kvm \
		-rtc base=localtime,clock=host,driftfix=slew

clean:
	rm -rf ./build $(NAME) isodir $(NAME).iso $(NAME).map $(NAME).tmp initrd.tar
	$(MAKE) -C initrd clean