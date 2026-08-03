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

OBJS = entry.o kernel.o kalloc.o panic.o vm.o trap.o trap_entry.o proc.o switch.o

all: kernel.elf

entry.o: entry.S
	$(CC) $(CFLAGS) -c entry.S -o entry.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

kalloc.o: kalloc.c
	$(CC) $(CFLAGS) -c kalloc.c -o kalloc.o

panic.o: panic.c
	$(CC) $(CFLAGS) -c panic.c -o panic.o

vm.o: vm.c
	$(CC) $(CFLAGS) -c vm.c -o vm.o

trap.o: trap.c
	$(CC) $(CFLAGS) -c trap.c -o trap.o

trap_entry.o: trap.S
	$(CC) $(CFLAGS) -c trap.S -o trap_entry.o

proc.o: proc.c
	$(CC) $(CFLAGS) -c proc.c -o proc.o

switch.o: switch.S
	$(CC) $(CFLAGS) -c switch.S -o switch.o

kernel.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJS)

run: kernel.elf
	qemu-system-riscv64 \
		-machine virt \
		-m 128M \
		-bios none \
		-kernel kernel.elf \
		-nographic

clean:
	rm -f *.o kernel.elf