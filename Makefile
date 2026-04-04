CROSS = riscv64-unknown-elf-

CC = $(CROSS)gcc
LD = $(CROSS)ld
OBJDUMP = $(CROSS)objdump

CFLAGS = \
	-march=rv64gc \
	-mabi=lp64 \
	-mcmodel=medany \
	-ffreestanding \
	-fno-builtin \
	-fno-stack-protector \
	-nostdlib \
	-nostartfiles \
	-nodefaultlibs \
	-Wall \
	-Wextra \
	-O2

LDFLAGS = -T linker.ld -nostdlib

OBJS = entry.o kernel.o

all: kernel.elf

entry.o: entry.S
	$(CC) $(CFLAGS) -c entry.S -o entry.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

kernel.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJS)

run: kernel.elf
	qemu-system-riscv64 \
		-machine virt \
		-bios none \
		-kernel kernel.elf \
		-nographic

clean:
	rm -f *.o kernel.elf
