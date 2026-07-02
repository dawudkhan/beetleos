.section .text.entry
.global _start

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

    # enter supervisor mode
    mret

1:
    wfi
    j 1b