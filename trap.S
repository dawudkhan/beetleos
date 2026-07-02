.section .text
.align 4
.global trap_vector
trap_vector:
    j trap_handler
