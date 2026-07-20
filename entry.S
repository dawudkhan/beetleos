.section .text.entry
.global _start

.equ CLINT_MTIMECMP0, 0x02004000
.equ CLINT_MTIME, 0x0200bff8
.equ TIMER_INTERVAL, 1000000

_start:
    # set up kernel stack
    la sp, stack_top

    # set mpp to supervisor mode
    li t0, 0x1800
    csrrc zero, mstatus, t0

    li t0, 0x0800
    csrs mstatus, t0

    # set return address for mret
    la t0, kernel_main
    csrw mepc, t0

    # allow s mode to access physical memory
    li t0, -1
    csrw pmpaddr0, t0

    li t0, 0x0f
    csrw pmpcfg0, t0

    # delegate exceptions and interrupts to s-mode
    li t0, 0xffff
    csrw medeleg, t0
    csrw mideleg, t0

    # point machine mode at the timer trap handler
    la t0, timervec
    csrw mtvec, t0

    # arm the first timer interrupt
    li t0, CLINT_MTIME
    ld t1, 0(t0)
    li t2, TIMER_INTERVAL
    add t1, t1, t2
    li t0, CLINT_MTIMECMP0
    sd t1, 0(t0)

    # enable machine timer interrupts
    li t0, 0x80
    csrs mie, t0

    # enter supervisor mode
    mret

1:
    wfi
    j 1b

# runs in machine mode when the hardware timer fires.
.align 4
timervec:
    addi sp, sp, -24
    sd t0, 0(sp)
    sd t1, 8(sp)
    sd t2, 16(sp)

    # push the next interrupt out by one interval
    li t0, CLINT_MTIME
    ld t1, 0(t0)
    li t2, TIMER_INTERVAL
    add t1, t1, t2
    li t0, CLINT_MTIMECMP0
    sd t1, 0(t0)

    # ask supervisor mode to take a software interrupt instead
    li t0, 2
    csrs mip, t0

    ld t2, 16(sp)
    ld t1, 8(sp)
    ld t0, 0(sp)
    addi sp, sp, 24

    mret