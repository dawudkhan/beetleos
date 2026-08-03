.section .text
.global enter_user_mode

# enter_user_mode(satp_val, entry_pc);
enter_user_mode:
    csrw satp, a0
    sfence.vma

    csrw sepc, a1

    li t0, (1 << 8)
    csrc sstatus, t0

    sret
