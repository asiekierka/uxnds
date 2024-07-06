@
@ Core variables
@

.section .itcm, "ax", %progbits

.global wst_ptr
wst_ptr: .word wst
.global rst_ptr
rst_ptr: .word rst

@ UXN evaluation function.
@
@   r0: PC pointer (argument for this function is the offset from UXN RAM).
@   r1: Stack pointer (wst).
@   r2: Stack pointer (rst).
@   r3-r6: Scratch registers.
@   r7: Ram ptr.
@
.global uxn_eval_asm
uxn_eval_asm:
    @ Ensure PC is not null.
    cmp     r0, #0
    bxeq    lr

    @ Initialization.
    push    {r4-r7}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    ldr     r7, =uxn_ram
    add     r0, r0, r7

uxn_decode:
    ldrb    r3, [r0], #1 @ current OP value / table index

    @ Decode OP based on table lookup.
    adr     r4, op_table         @ decoding table
    ldr     r4, [r4, r3, lsl #2] @ op_table[idx * 4]
    bx      r4                   @ op_table[idx * 4]()

uxn_ret:
    @ Update stack pointers and return.
    adr     r0, wst_ptr
    str     r1, [r0]
    adr     r0, rst_ptr
    str     r2, [r0]
    pop     {r4-r7}
    bx      lr

@
@ Macros.
@

.macro next a
    ldrb    \a, [r0], #1
.endm

.macro wpop8 a
    ldrb    \a, [r1, #-1]!
.endm

.macro wpop8s a
    ldrsb   \a, [r1, #-1]!
.endm

.macro wpop16 a, b
    ldrb    \a, [r1, #-1]!
    ldrb    \b, [r1, #-1]!
    orr     \a, \a, \b, lsl #8
.endm

.macro wpush8 a
    strb    \a, [r1], #1
.endm

.macro wpush16 a
    strb    \a, [r1, #1]
    lsr     \a, #8
    strb    \a, [r1], #2
.endm

.macro wpeek8 a, off
    ldrb    \a, [r1, \off]
.endm

.macro wpeek8s a, off
    ldrsb   \a, [r1, \off]
.endm

.macro wpeek16 a, b, offa, offb
    ldrb    \a, [r1, \offa]
    ldrb    \b, [r1, \offb]
    orr     \a, \a, \b, lsl #8
.endm

.macro rpop8 a
    ldrb    \a, [r2, #-1]!
.endm

.macro rpop8s a
    ldrsb   \a, [r2, #-1]!
.endm

.macro rpop16 a, b
    ldrb    \a, [r2, #-1]!
    ldrb    \b, [r2, #-1]!
    orr     \a, \a, \b, lsl #8
.endm

.macro rpush8 a
    strb    \a, [r2], #1
.endm

.macro rpush16 a
    strb    \a, [r2, #1]
    lsr     \a, #8
    strb    \a, [r2], #2
.endm

.macro rpeek8 a, off
    ldrb    \a, [r2, \off]
.endm

.macro rpeek8s a, off
    ldrsb   \a, [r2, \off]
.endm

.macro rpeek16 a, b, offa, offb
    ldrb    \a, [r2, \offa]
    ldrb    \b, [r2, \offb]
    orr     \a, \a, \b, lsl #8
.endm

.macro zsave8 a, off
    strb    \a, [r7, \off]
.endm

.macro zsave16 a, off
    add     \off, r7
    strb    \a, [\off, #1]
    lsr     \a, #8
    strb    \a, [\off]
.endm

.macro zload8 a, off
    ldrb    \a, [r7, \off]
.endm

.macro rload8 a, off
    ldrb    \a, [r0, \off]
.endm

.macro aload8 a, off
    ldrb    \a, [r7, \off]
.endm

.macro asave8 a, off
    strb    \a, [r7, \off]
.endm

.macro asave16 a, off
    add     \off, r7
    strb    \a, [\off, #1]
    lsr     \a, #8
    strb    \a, [\off]
.endm

.macro rsave8 a, off
    strb    \a, [r0, \off]
.endm

.macro rsave16 a, off
    adds    \off, r0
    strb    \a, [\off, #1]
    lsr     \a, #8
    strb    \a, [\off]
.endm

@
@ OP table
@

op_table:
    .word brk       @ 0x00
    .word inc       @ 0x01
    .word pop       @ 0x02
    .word nip       @ 0x03
    .word swp       @ 0x04
    .word rot       @ 0x05
    .word dup       @ 0x06
    .word ovr       @ 0x07
    .word equ       @ 0x08
    .word neq       @ 0x09
    .word gth       @ 0x0a
    .word lth       @ 0x0b
    .word jmp       @ 0x0c
    .word jcn       @ 0x0d
    .word jsr       @ 0x0e
    .word sth       @ 0x0f
    .word ldz       @ 0x10
    .word stz       @ 0x11
    .word ldr       @ 0x12
    .word str       @ 0x13
    .word lda       @ 0x14
    .word sta       @ 0x15
    .word dei       @ 0x16
    .word deo       @ 0x17
    .word add       @ 0x18
    .word sub       @ 0x19
    .word mul       @ 0x1a
    .word div       @ 0x1b
    .word and       @ 0x1c
    .word ora       @ 0x1d
    .word eor       @ 0x1e
    .word sft       @ 0x1f
    .word jci       @ 0x20
    .word inc2      @ 0x21
    .word pop2      @ 0x22
    .word nip2      @ 0x23
    .word swp2      @ 0x24
    .word rot2      @ 0x25
    .word dup2      @ 0x26
    .word ovr2      @ 0x27
    .word equ2      @ 0x28
    .word neq2      @ 0x29
    .word gth2      @ 0x2a
    .word lth2      @ 0x2b
    .word jmp2      @ 0x2c
    .word jcn2      @ 0x2d
    .word jsr2      @ 0x2e
    .word sth2      @ 0x2f
    .word ldz2      @ 0x30
    .word stz2      @ 0x31
    .word ldr2      @ 0x32
    .word str2      @ 0x33
    .word lda2      @ 0x34
    .word sta2      @ 0x35
    .word dei2      @ 0x36
    .word deo2      @ 0x37
    .word add2      @ 0x38
    .word sub2      @ 0x39
    .word mul2      @ 0x3a
    .word div2      @ 0x3b
    .word and2      @ 0x3c
    .word ora2      @ 0x3d
    .word eor2      @ 0x3e
    .word sft2      @ 0x3f
    .word jmi       @ 0x40
    .word incr      @ 0x41
    .word popr      @ 0x42
    .word nipr      @ 0x43
    .word swpr      @ 0x44
    .word rotr      @ 0x45
    .word dupr      @ 0x46
    .word ovrr      @ 0x47
    .word equr      @ 0x48
    .word neqr      @ 0x49
    .word gthr      @ 0x4a
    .word lthr      @ 0x4b
    .word jmpr      @ 0x4c
    .word jcnr      @ 0x4d
    .word jsrr      @ 0x4e
    .word sthr      @ 0x4f
    .word ldzr      @ 0x50
    .word stzr      @ 0x51
    .word ldrr      @ 0x52
    .word strr      @ 0x53
    .word ldar      @ 0x54
    .word star      @ 0x55
    .word deir      @ 0x56
    .word deor      @ 0x57
    .word addr      @ 0x58
    .word subr      @ 0x59
    .word mulr      @ 0x5a
    .word divr      @ 0x5b
    .word andr      @ 0x5c
    .word orar      @ 0x5d
    .word eorr      @ 0x5e
    .word sftr      @ 0x5f
    .word jsi       @ 0x60
    .word inc2r     @ 0x61
    .word pop2r     @ 0x62
    .word nip2r     @ 0x63
    .word swp2r     @ 0x64
    .word rot2r     @ 0x65
    .word dup2r     @ 0x66
    .word ovr2r     @ 0x67
    .word equ2r     @ 0x68
    .word neq2r     @ 0x69
    .word gth2r     @ 0x6a
    .word lth2r     @ 0x6b
    .word jmp2r     @ 0x6c
    .word jcn2r     @ 0x6d
    .word jsr2r     @ 0x6e
    .word sth2r     @ 0x6f
    .word ldz2r     @ 0x70
    .word stz2r     @ 0x71
    .word ldr2r     @ 0x72
    .word str2r     @ 0x73
    .word lda2r     @ 0x74
    .word sta2r     @ 0x75
    .word dei2r     @ 0x76
    .word deo2r     @ 0x77
    .word add2r     @ 0x78
    .word sub2r     @ 0x79
    .word mul2r     @ 0x7a
    .word div2r     @ 0x7b
    .word and2r     @ 0x7c
    .word ora2r     @ 0x7d
    .word eor2r     @ 0x7e
    .word sft2r     @ 0x7f
    .word lit       @ 0x80
    .word inck      @ 0x81
    .word popk      @ 0x82
    .word nipk      @ 0x83
    .word swpk      @ 0x84
    .word rotk      @ 0x85
    .word dupk      @ 0x86
    .word ovrk      @ 0x87
    .word equk      @ 0x88
    .word neqk      @ 0x89
    .word gthk      @ 0x8a
    .word lthk      @ 0x8b
    .word jmpk      @ 0x8c
    .word jcnk      @ 0x8d
    .word jsrk      @ 0x8e
    .word sthk      @ 0x8f
    .word ldzk      @ 0x90
    .word stzk      @ 0x91
    .word ldrk      @ 0x92
    .word strk      @ 0x93
    .word ldak      @ 0x94
    .word stak      @ 0x95
    .word deik      @ 0x96
    .word deok      @ 0x97
    .word addk      @ 0x98
    .word subk      @ 0x99
    .word mulk      @ 0x9a
    .word divk      @ 0x9b
    .word andk      @ 0x9c
    .word orak      @ 0x9d
    .word eork      @ 0x9e
    .word sftk      @ 0x9f
    .word lit2      @ 0xa0
    .word inc2k     @ 0xa1
    .word pop2k     @ 0xa2
    .word nip2k     @ 0xa3
    .word swp2k     @ 0xa4
    .word rot2k     @ 0xa5
    .word dup2k     @ 0xa6
    .word ovr2k     @ 0xa7
    .word equ2k     @ 0xa8
    .word neq2k     @ 0xa9
    .word gth2k     @ 0xaa
    .word lth2k     @ 0xab
    .word jmp2k     @ 0xac
    .word jcn2k     @ 0xad
    .word jsr2k     @ 0xae
    .word sth2k     @ 0xaf
    .word ldz2k     @ 0xb0
    .word stz2k     @ 0xb1
    .word ldr2k     @ 0xb2
    .word str2k     @ 0xb3
    .word lda2k     @ 0xb4
    .word sta2k     @ 0xb5
    .word dei2k     @ 0xb6
    .word deo2k     @ 0xb7
    .word add2k     @ 0xb8
    .word sub2k     @ 0xb9
    .word mul2k     @ 0xba
    .word div2k     @ 0xbb
    .word and2k     @ 0xbc
    .word ora2k     @ 0xbd
    .word eor2k     @ 0xbe
    .word sft2k     @ 0xbf
    .word litr      @ 0xc0
    .word inckr     @ 0xc1
    .word popkr     @ 0xc2
    .word nipkr     @ 0xc3
    .word swpkr     @ 0xc4
    .word rotkr     @ 0xc5
    .word dupkr     @ 0xc6
    .word ovrkr     @ 0xc7
    .word equkr     @ 0xc8
    .word neqkr     @ 0xc9
    .word gthkr     @ 0xca
    .word lthkr     @ 0xcb
    .word jmpkr     @ 0xcc
    .word jcnkr     @ 0xcd
    .word jsrkr     @ 0xce
    .word sthkr     @ 0xcf
    .word ldzkr     @ 0xd0
    .word stzkr     @ 0xd1
    .word ldrkr     @ 0xd2
    .word strkr     @ 0xd3
    .word ldakr     @ 0xd4
    .word stakr     @ 0xd5
    .word deikr     @ 0xd6
    .word deokr     @ 0xd7
    .word addkr     @ 0xd8
    .word subkr     @ 0xd9
    .word mulkr     @ 0xda
    .word divkr     @ 0xdb
    .word andkr     @ 0xdc
    .word orakr     @ 0xdd
    .word eorkr     @ 0xde
    .word sftkr     @ 0xdf
    .word lit2r     @ 0xe0
    .word inc2kr    @ 0xe1
    .word pop2kr    @ 0xe2
    .word nip2kr    @ 0xe3
    .word swp2kr    @ 0xe4
    .word rot2kr    @ 0xe5
    .word dup2kr    @ 0xe6
    .word ovr2kr    @ 0xe7
    .word equ2kr    @ 0xe8
    .word neq2kr    @ 0xe9
    .word gth2kr    @ 0xea
    .word lth2kr    @ 0xeb
    .word jmp2kr    @ 0xec
    .word jcn2kr    @ 0xed
    .word jsr2kr    @ 0xee
    .word sth2kr    @ 0xef
    .word ldz2kr    @ 0xf0
    .word stz2kr    @ 0xf1
    .word ldr2kr    @ 0xf2
    .word str2kr    @ 0xf3
    .word lda2kr    @ 0xf4
    .word sta2kr    @ 0xf5
    .word dei2kr    @ 0xf6
    .word deo2kr    @ 0xf7
    .word add2kr    @ 0xf8
    .word sub2kr    @ 0xf9
    .word mul2kr    @ 0xfa
    .word div2kr    @ 0xfb
    .word and2kr    @ 0xfc
    .word ora2kr    @ 0xfd
    .word eor2kr    @ 0xfe
    .word sft2kr    @ 0xff

@
@ OP implementations.
@

dei:
    wpop8   r3
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    ldr     r6, =dei_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r3
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    wpush8  r0
    ldmfd   sp!, {r0, r7, lr}
    b       uxn_decode

dei2:
    wpop8   r5
    mov     r4, r5, lsr #4 @ idx
    and     r5, #0x0f      @ port
    ldr     r6, =dei_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r5
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    add     r1, r5, #1
    mov     r5, r0
    ldr     r0, =device_data
    add     r0, r4
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    wpush8  r5
    wpush8  r0
    ldmfd   sp!, {r0, r7, lr}
    b       uxn_decode

deo:
    @ Get args (idx/port/value).
    wpop8   r3
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    wpop8   r5             @ value

    @ Find current devide.
    ldr     r6, =deo_map
    ldr     r6, [r6, r4, lsl #2]

    @ Save registers that can be affected.
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]

    @ Call the deo function.
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    strb    r5, [r0, r3]
    mov     r1, r3
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif

    @ Restore saved variables.
    ldmfd   sp!, {r0, r7, lr}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    b       uxn_decode

deo2:
    @ Get args (idx/port/value).
    wpop8   r3
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    wpop16  r5, r6         @ value

    @ Find current devide.
    ldr     r6, =deo_map
    ldr     r6, [r6, r4, lsl #2]

    @ Save registers that can be affected.
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]

    @ Call the deo function.
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r3
    add     r3, r0
    strb    r5, [r3, #1]
    lsr     r5, #8
    strb    r5, [r3]
    mov     r2, r6
    ldr     r6, =deo2_wrap
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif

    @ Restore saved variables.
    ldmfd   sp!, {r0, r7, lr}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    b       uxn_decode

deir:
    rpop8   r3
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    ldr     r6, =dei_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r3
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    rpush8  r0
    ldmfd   sp!, {r0, r7, lr}
    b       uxn_decode

dei2r:
    rpop8   r5
    mov     r4, r5, lsr #4 @ idx
    and     r5, #0x0f      @ port
    ldr     r6, =dei_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r5
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    add     r1, r5, #1
    mov     r5, r0
    ldr     r0, =device_data
    add     r0, r4
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    rpush8  r5
    rpush8  r0
    ldmfd   sp!, {r0, r7, lr}
    b       uxn_decode

deor:
    @ Get args (idx/port/value).
    rpop8   r3
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    rpop8   r5             @ value

    @ Find current devide.
    ldr     r6, =deo_map
    ldr     r6, [r6, r4, lsl #2]

    @ Save registers that can be affected.
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]

    @ Call the deo function.
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    strb    r5, [r0, r3]
    mov     r1, r3
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif

    @ Restore saved variables.
    ldmfd   sp!, {r0, r7, lr}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    b       uxn_decode

deo2r:
    @ Get args (idx/port/value).
    rpop8   r3
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    rpop16  r5, r6         @ value

    @ Find current devide.
    ldr     r6, =deo_map
    ldr     r6, [r6, r4, lsl #2]

    @ Save registers that can be affected.
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]

    @ Call the deo function.
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r3
    add     r3, r0
    strb    r5, [r3, #1]
    lsr     r5, #8
    strb    r5, [r3]
    mov     r2, r6
    ldr     r6, =deo2_wrap
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif

    @ Restore saved variables.
    ldmfd   sp!, {r0, r7, lr}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    b       uxn_decode

deik:
    wpeek8  r3, #-1
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    ldr     r6, =dei_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r5
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    wpush8  r0
    ldmfd   sp!, {r0, r7, lr}
    b       uxn_decode

dei2k:
    wpeek8  r5, #-1
    mov     r4, r5, lsr #4 @ idx
    and     r5, #0x0f      @ port
    ldr     r6, =dei_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r5
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    add     r1, r5, #1
    mov     r5, r0
    ldr     r0, =device_data
    add     r0, r4
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    wpush8  r5
    wpush8  r0
    ldmfd   sp!, {r0, r7, lr}
    b       uxn_decode

deok:
    wpeek8  r3, #-1
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    wpeek8  r5, #-2        @ value
    ldr     r6, =deo_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    strb    r5, [r0, r3]
    mov     r1, r3
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldmfd   sp!, {r0, r7, lr}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    b       uxn_decode

deo2k:
    wpeek8  r3, #-1
    mov     r4, r3, lsr #4   @ idx
    and     r3, #0x0f        @ port
    wpeek16 r5, r6, #-2, #-3 @ value
    ldr     r6, =deo_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r3
    add     r3, r0
    strb    r5, [r3, #1]
    lsr     r5, #8
    strb    r5, [r3]
    mov     r2, r6
    ldr     r6, =deo2_wrap
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldmfd   sp!, {r0, r7, lr}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    b       uxn_decode

deikr:
    rpeek8  r3, #-1
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    ldr     r6, =dei_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r5
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    rpush8  r0
    ldmfd   sp!, {r0, r7, lr}
    b       uxn_decode

dei2kr:
    rpeek8  r5, #-1
    mov     r4, r5, lsr #4 @ idx
    and     r5, #0x0f      @ port
    ldr     r6, =dei_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r5
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    add     r1, r5, #1
    mov     r5, r0
    ldr     r0, =device_data
    add     r0, r4
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    rpush8  r5
    rpush8  r0
    ldmfd   sp!, {r0, r7, lr}
    b       uxn_decode

deokr:
    rpeek8  r3, #-1
    mov     r4, r3, lsr #4 @ idx
    and     r3, #0x0f      @ port
    rpeek8  r5, #-2        @ value
    ldr     r6, =deo_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    strb    r5, [r0, r3]
    mov     r1, r3
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldmfd   sp!, {r0, r7, lr}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    b       uxn_decode

deo2kr:
    rpeek8  r3, #-1
    mov     r4, r3, lsr #4   @ idx
    and     r3, #0x0f        @ port
    rpeek16 r5, r6, #-2, #-3 @ value
    ldr     r6, =deo_map
    ldr     r6, [r6, r4, lsl #2]
    stmfd   sp!, {r0, r7, lr}
    ldr     r0, =wst_ptr
    str     r1, [r0]
    ldr     r0, =rst_ptr
    str     r2, [r0]
    ldr     r0, =device_data
    lsl     r4, #4
    add     r0, r4
    mov     r1, r3
    add     r3, r0
    strb    r5, [r3, #1]
    lsr     r5, #8
    strb    r5, [r3]
    mov     r2, r6
    ldr     r6, =deo2_wrap
#if __ARM_ARCH >= 5
    blx     r6
#else
    mov     lr, pc
    bx      r6
#endif
    ldmfd   sp!, {r0, r7, lr}
    ldr     r1, wst_ptr
    ldr     r2, rst_ptr
    b       uxn_decode

.ltorg
.align 2

.global dei_map
dei_map:
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub
    .word dei_stub

.global deo_map
deo_map:
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub
    .word deo_stub

brk:
    b       uxn_ret

jci:
    ldrb    r5, [r0], #1
    ldrb    r3, [r0], #1
    orr     r3, r3, r5, lsl #8
#if __ARM_ARCH >= 6
    sxth    r3, r3
#else
    lsl     r3, r3, #16
    asr     r3, r3, #16
#endif
    wpop8   r4
    cmp     r4, #0
    addne   r0, r3
    b       uxn_decode

jmi:
    ldrb    r5, [r0], #1
    ldrb    r3, [r0], #1
    orr     r3, r3, r5, lsl #8
#if __ARM_ARCH >= 6
    sxtah   r0, r0, r3
#else
    lsl     r3, r3, #16
    asr     r3, r3, #16
    add     r0, r3
#endif
    b       uxn_decode

jsi:
    ldrb    r5, [r0], #1
    ldrb    r3, [r0], #1
    orr     r3, r3, r5, lsl #8
    mov     r4, r0
    rpush16 r4
#if __ARM_ARCH >= 6
    sxtah   r0, r0, r3
#else
    lsl     r3, r3, #16
    asr     r3, r3, #16
    add     r0, r3
#endif
    b       uxn_decode

lit:
    next    r3
    wpush8  r3
    b       uxn_decode

lit2:
    next    r3
    next    r4
    wpush8  r3
    wpush8  r4
    b       uxn_decode

litr:
    next    r3
    rpush8  r3
    b       uxn_decode

lit2r:
    next    r3
    next    r4
    rpush8  r3
    rpush8  r4
    b       uxn_decode

.ltorg
.align 2

inc:
    wpop8   r3
    add     r3, #1
    wpush8  r3
    b       uxn_decode

inc2:
    wpop16  r3, r5
    add     r3, r3, #1
    wpush16 r3
    b       uxn_decode

pop:
    sub     r1, #1
    b       uxn_decode

pop2:
    sub     r1, #2
    b       uxn_decode

nip:
    wpop8   r3
    strb    r3, [r1, #-1]
    b       uxn_decode

nip2:
    wpop16  r3, r5
    strb    r3, [r1, #-1]
    lsr     r3, #8
    strb    r3, [r1, #-2]
    b       uxn_decode

swp:
    wpop8   r3
    wpop8   r4
    wpush8  r3
    wpush8  r4
    b       uxn_decode

swp2:
    wpop16  r3, r5
    wpop16  r4, r5
    wpush16 r3
    wpush16 r4
    b       uxn_decode

rot:
    wpop8   r5
    wpop8   r4
    wpop8   r3
    wpush8  r4
    wpush8  r5
    wpush8  r3
    b       uxn_decode

rot2:
    wpop16  r5, r6
    wpop16  r4, r6
    wpop16  r3, r6
    wpush16 r4
    wpush16 r5
    wpush16 r3
    b       uxn_decode

dup:
    wpeek8  r3, #-1
    wpush8  r3
    b       uxn_decode

dup2:
    wpeek8  r3, #-2
    wpeek8  r4, #-1
    wpush8  r3
    wpush8  r4
    b       uxn_decode

ovr:
    wpeek8  r3, #-2
    wpush8  r3
    b       uxn_decode

ovr2:
    wpeek8  r3, #-4
    wpeek8  r4, #-3
    wpush8  r3
    wpush8  r4
    b       uxn_decode

equ:
    wpop8   r3
    wpop8   r4
    sub     r3, r4, r3
    rsbs    r4, r3, #0
    adc     r4, r4, r3
    wpush8  r4
    b       uxn_decode

equ2:
    wpop16  r3, r5
    wpop16  r4, r5
    sub     r3, r4, r3
    rsbs    r4, r3, #0
    adc     r4, r4, r3
    wpush8  r4
    b       uxn_decode

neq:
    wpop8   r3
    wpop8   r4
    subs    r3, r4, r3
    movne   r3, #1
    wpush8  r3
    b       uxn_decode

neq2:
    wpop16  r3, r5
    wpop16  r4, r5
    subs    r3, r4, r3
    movne   r3, #1
    wpush8  r3
    b       uxn_decode

gth:
    wpop8   r3
    wpop8   r4
    cmp     r4, r3
    movls   r3, #0
    movhi   r3, #1
    wpush8  r3
    b       uxn_decode

gth2:
    wpop16  r3, r5
    wpop16  r4, r5
    cmp     r4, r3
    movls   r3, #0
    movhi   r3, #1
    wpush8  r3
    b       uxn_decode

lth:
    wpop8   r3
    wpop8   r4
    cmp     r4, r3
    movcs   r3, #0
    movcc   r3, #1
    wpush8  r3
    b       uxn_decode

lth2:
    wpop16  r3, r5
    wpop16  r4, r5
    cmp     r4, r3
    movcs   r3, #0
    movcc   r3, #1
    wpush8  r3
    b       uxn_decode

jmp:
    wpop8s  r3
    add     r0, r3
    b       uxn_decode

jmp2:
    wpop16  r3, r5
    mov     r0, r7
    add     r0, r0, r3
    b       uxn_decode

jcn:
    wpop8s  r3
    wpop8   r4
    cmp     r4, #0
    addne   r0, r3
    b       uxn_decode

jcn2:
    wpop16  r3, r5
    wpop8   r4
    cmp     r4, #0
    movne   r0, r7
    cmp     r4, #0
    addne   r0, r0, r3
    b       uxn_decode

jsr:
    mov     r3, r0
    sub     r3, r3, r7
    wpop8s  r4
    rpush16 r3
    add     r0, r4
    b       uxn_decode

jsr2:
    mov     r3, r0
    sub     r3, r3, r7
    wpop16  r4, r5
    rpush16 r3
    mov     r0, r7
    add     r0, r0, r4
    b       uxn_decode

sth:
    wpop8   r3
    rpush8  r3
    b       uxn_decode

sth2:
    wpop16  r3, r5
    rpush16 r3
    b       uxn_decode

ldz:
    wpop8   r3
    zload8  r4, r3
    wpush8  r4
    b       uxn_decode

ldz2:
    wpop8   r3
    zload8  r4, r3
    wpush8  r4
    add     r3, #1
    zload8  r4, r3
    wpush8  r4
    b       uxn_decode

stz:
    wpop8   r3
    wpop8   r4
    zsave8  r4, r3
    b       uxn_decode

stz2:
    wpop8   r3
    wpop16  r4, r5
    zsave16 r4, r3
    b       uxn_decode

ldr:
    wpop8s  r4
    rload8  r3, r4
    wpush8  r3
    b       uxn_decode

ldr2:
    wpop8s  r4
    rload8  r3, r4
    wpush8  r3
    add     r4, #1
    rload8  r3, r4
    wpush8  r3
    b       uxn_decode

str:
    wpop8s  r4
    wpop8   r3
    rsave8  r3, r4
    b       uxn_decode

str2:
    wpop8s  r4
    wpop16  r3, r5
    rsave16 r3, r4
    b       uxn_decode

lda:
    wpop16  r4, r5
    aload8  r3, r4
    wpush8  r3
    b       uxn_decode

lda2:
    wpop16  r4, r5
    aload8  r3, r4
    wpush8  r3
    add     r4, #1
    aload8  r3, r4
    wpush8  r3
    b       uxn_decode

sta:
    wpop16  r4, r5
    wpop8   r3
    asave8  r3, r4
    b       uxn_decode

sta2:
    wpop16  r4, r5
    wpop16  r3, r5
    asave16 r3, r4
    b       uxn_decode

add:
    wpop8   r3
    wpop8   r4
    add     r3, r3, r4
    wpush8  r3
    b       uxn_decode

add2:
    wpop16  r3, r5
    wpop16  r4, r5
    add     r3, r3, r4
    wpush16 r3
    b       uxn_decode

sub:
    wpop8   r3
    wpop8   r4
    sub     r4, r4, r3
    wpush8  r4
    b       uxn_decode

sub2:
    wpop16  r3, r5
    wpop16  r4, r5
    sub     r3, r4, r3
    wpush16 r3
    b       uxn_decode

mul:
    wpop8   r3
    wpop8   r4
    mul     r4, r3, r4
    wpush8  r4
    b       uxn_decode

mul2:
    wpop16  r3, r5
    wpop16  r4, r5
    mul     r3, r4, r3
    wpush16 r3
    b       uxn_decode

div:
    wpop8   r3
    wpop8   r4
    push    {r0, r1, r2, r7, lr}
    mov     r1, r3
    mov     r0, r4
    bl      uxn_uidiv
    mov     r3, r0
    pop     {r0, r1, r2, r7, lr}
    wpush8  r3
    b       uxn_decode

div2:
    wpop16  r3, r5
    wpop16  r4, r5
    push    {r0, r1, r2, r7, lr}
    mov     r1, r3
    mov     r0, r4
    bl      uxn_uidiv
    mov     r3, r0
    pop     {r0, r1, r2, r7, lr}
    wpush16 r3
    b       uxn_decode

and:
    wpop8   r3
    wpop8   r4
    and     r3, r3, r4
    wpush8  r3
    b       uxn_decode

and2:
    wpop16  r3, r5
    wpop16  r4, r5
    and     r3, r3, r4
    wpush16 r3
    b       uxn_decode

ora:
    wpop8   r3
    wpop8   r4
    orr     r3, r3, r4
    wpush8  r3
    b       uxn_decode

ora2:
    wpop16  r3, r5
    wpop16  r4, r5
    orr     r3, r3, r4
    wpush16 r3
    b       uxn_decode

eor:
    wpop8   r3
    wpop8   r4
    eor     r3, r3, r4
    wpush8  r3
    b       uxn_decode

eor2:
    wpop16  r3, r5
    wpop16  r4, r5
    eor     r3, r3, r4
    wpush16 r3
    b       uxn_decode

sft:
    wpop8   r4
    wpop8   r3
    lsr r5, r4, #4
    and r4, #0x0f
    lsr r3, r3, r4
    lsl r3, r3, r5
    wpush8  r3
    b       uxn_decode

sft2:
    wpop8   r4
    wpop16  r3, r5
    lsr r5, r4, #4
    and r4, #0x0f
    lsr r3, r3, r4
    lsl r3, r3, r5
    wpush16 r3
    b       uxn_decode

.ltorg
.align 2

incr:
    rpop8   r3
    add     r3, #1
    rpush8  r3
    b       uxn_decode

inc2r:
    rpop16  r3, r5
    add     r3, r3, #1
    rpush16 r3
    b       uxn_decode

popr:
    sub     r2, #1
    b       uxn_decode

pop2r:
    sub     r2, #2
    b       uxn_decode

nipr:
    rpop8   r3
    strb    r3, [r2, #-1]
    b       uxn_decode

nip2r:
    rpop16  r3, r5
    strb    r3, [r2, #-1]
    lsr     r3, #8
    strb    r3, [r2, #-2]
    b       uxn_decode

swpr:
    rpop8   r3
    rpop8   r4
    rpush8  r3
    rpush8  r4
    b       uxn_decode

swp2r:
    rpop16  r3, r5
    rpop16  r4, r5
    rpush16 r3
    rpush16 r4
    b       uxn_decode

rotr:
    rpop8   r5
    rpop8   r4
    rpop8   r3
    rpush8  r4
    rpush8  r5
    rpush8  r3
    b       uxn_decode

rot2r:
    rpop16  r5, r6
    rpop16  r4, r6
    rpop16  r3, r6
    rpush16 r4
    rpush16 r5
    rpush16 r3
    b       uxn_decode

dupr:
    rpeek8  r3, #-1
    rpush8  r3
    b       uxn_decode

dup2r:
    rpeek8  r3, #-2
    rpeek8  r4, #-1
    rpush8  r3
    rpush8  r4
    b       uxn_decode

ovrr:
    rpeek8  r3, #-2
    rpush8  r3
    b       uxn_decode

ovr2r:
    rpeek8  r3, #-4
    rpeek8  r4, #-3
    rpush8  r3
    rpush8  r4
    b       uxn_decode

equr:
    rpop8   r3
    rpop8   r4
    sub     r3, r4, r3
    rsbs    r4, r3, #0
    adc     r4, r4, r3
    rpush8  r4
    b       uxn_decode

equ2r:
    rpop16  r3, r5
    rpop16  r4, r5
    sub     r3, r4, r3
    rsbs    r4, r3, #0
    adc     r4, r4, r3
    rpush8  r4
    b       uxn_decode

neqr:
    rpop8   r3
    rpop8   r4
    subs    r3, r4, r3
    movne   r3, #1
    rpush8  r3
    b       uxn_decode

neq2r:
    rpop16  r3, r5
    rpop16  r4, r5
    subs    r3, r4, r3
    movne   r3, #1
    rpush8  r3
    b       uxn_decode

gthr:
    rpop8   r3
    rpop8   r4
    cmp     r4, r3
    movls   r3, #0
    movhi   r3, #1
    rpush8  r3
    b       uxn_decode

gth2r:
    rpop16  r3, r5
    rpop16  r4, r5
    cmp     r4, r3
    movls   r3, #0
    movhi   r3, #1
    rpush8  r3
    b       uxn_decode

lthr:
    rpop8   r3
    rpop8   r4
    cmp     r4, r3
    movcs   r3, #0
    movcc   r3, #1
    rpush8  r3
    b       uxn_decode

lth2r:
    rpop16  r3, r5
    rpop16  r4, r5
    cmp     r4, r3
    movcs   r3, #0
    movcc   r3, #1
    rpush8  r3
    b       uxn_decode

jmpr:
    rpop8s  r3
    add     r0, r3
    b       uxn_decode

jmp2r:
    rpop16  r3, r5
    mov     r0, r7
    add     r0, r0, r3
    b       uxn_decode

jcnr:
    rpop8s  r3
    rpop8   r4
    cmp     r4, #0
    addne   r0, r3
    b       uxn_decode

jcn2r:
    rpop16  r3, r5
    rpop8   r4
    cmp     r4, #0
    movne   r0, r7
    cmp     r4, #0
    addne   r0, r0, r3
    b       uxn_decode

jsrr:
    mov     r3, r0
    sub     r3, r3, r7
    rpop8s  r4
    rpush16 r3
    add     r0, r4
    b       uxn_decode

jsr2r:
    mov     r3, r0
    sub     r3, r3, r7
    rpop16  r4, r5
    rpush16 r3
    mov     r0, r7
    add     r0, r0, r4
    b       uxn_decode

sthr:
    rpop8   r3
    wpush8  r3
    b       uxn_decode

sth2r:
    rpop16  r3, r5
    wpush16 r3
    b       uxn_decode

ldzr:
    rpop8   r3
    zload8  r4, r3
    rpush8  r4
    b       uxn_decode

ldz2r:
    rpop8   r3
    zload8  r4, r3
    rpush8  r4
    add     r3, #1
    zload8  r4, r3
    rpush8  r4
    b       uxn_decode

stzr:
    rpop8   r3
    rpop8   r4
    zsave8  r4, r3
    b       uxn_decode

stz2r:
    rpop8   r3
    rpop16  r4, r5
    zsave16 r4, r3
    b       uxn_decode

ldrr:
    rpop8s  r4
    rload8  r3, r4
    rpush8  r3
    b       uxn_decode

ldr2r:
    rpop8s  r4
    rload8  r3, r4
    rpush8  r3
    add     r4, #1
    rload8  r3, r4
    rpush8  r3
    b       uxn_decode

strr:
    rpop8s  r4
    rpop8   r3
    rsave8  r3, r4
    b       uxn_decode

str2r:
    rpop8s  r4
    rpop16  r3, r5
    rsave16 r3, r4
    b       uxn_decode

ldar:
    rpop16  r4, r5
    aload8  r3, r4
    rpush8  r3
    b       uxn_decode

lda2r:
    rpop16  r4, r5
    aload8  r3, r4
    rpush8  r3
    add     r4, #1
    aload8  r3, r4
    rpush8  r3
    b       uxn_decode

star:
    rpop16  r4, r5
    rpop8   r3
    asave8  r3, r4
    b       uxn_decode

sta2r:
    rpop16  r4, r5
    rpop16  r3, r5
    asave16 r3, r4
    b       uxn_decode

addr:
    rpop8   r3
    rpop8   r4
    add     r3, r3, r4
    rpush8  r3
    b       uxn_decode

add2r:
    rpop16  r3, r5
    rpop16  r4, r5
    add     r3, r3, r4
    rpush16 r3
    b       uxn_decode

subr:
    rpop8   r3
    rpop8   r4
    sub     r4, r4, r3
    rpush8  r4
    b       uxn_decode

sub2r:
    rpop16  r3, r5
    rpop16  r4, r5
    sub     r3, r4, r3
    rpush16 r3
    b       uxn_decode

mulr:
    rpop8   r3
    rpop8   r4
    mul     r4, r3, r4
    rpush8  r4
    b       uxn_decode

mul2r:
    rpop16  r3, r5
    rpop16  r4, r5
    mul     r3, r4, r3
    rpush16 r3
    b       uxn_decode

divr:
    rpop8   r3
    rpop8   r4
    push    {r0, r1, r2, r7, lr}
    mov     r1, r3
    mov     r0, r4
    bl      uxn_uidiv
    mov     r3, r0
    pop     {r0, r1, r2, r7, lr}
    rpush8  r3
    b       uxn_decode

div2r:
    rpop16  r3, r5
    rpop16  r4, r5
    push    {r0, r1, r2, r7, lr}
    mov     r1, r3
    mov     r0, r4
    bl      uxn_uidiv
    mov     r3, r0
    pop     {r0, r1, r2, r7, lr}
    rpush16 r3
    b       uxn_decode

andr:
    rpop8   r3
    rpop8   r4
    and     r3, r3, r4
    rpush8  r3
    b       uxn_decode

and2r:
    rpop16  r3, r5
    rpop16  r4, r5
    and     r3, r3, r4
    rpush16 r3
    b       uxn_decode

orar:
    rpop8   r3
    rpop8   r4
    orr     r3, r3, r4
    rpush8  r3
    b       uxn_decode

ora2r:
    rpop16  r3, r5
    rpop16  r4, r5
    orr     r3, r3, r4
    rpush16 r3
    b       uxn_decode

eorr:
    rpop8   r3
    rpop8   r4
    eor     r3, r3, r4
    rpush8  r3
    b       uxn_decode

eor2r:
    rpop16  r3, r5
    rpop16  r4, r5
    eor     r3, r3, r4
    rpush16 r3
    b       uxn_decode

sftr:
    rpop8   r4
    rpop8   r3
    lsr r5, r4, #4
    and r4, #0x0f
    lsr r3, r3, r4
    lsl r3, r3, r5
    rpush8  r3
    b       uxn_decode

sft2r:
    rpop8   r4
    rpop16  r3, r5
    lsr r5, r4, #4
    and r4, #0x0f
    lsr r3, r3, r4
    lsl r3, r3, r5
    rpush16 r3
    b       uxn_decode

.ltorg
.align 2

inck:
    wpeek8  r3, #-1
    add     r3, #1
    wpush8  r3
    b       uxn_decode

inc2k:
    wpeek16 r3, r5, #-1, #-2
    add     r3, r3, #1
    wpush16 r3
    b       uxn_decode

popk:
    b       uxn_decode

pop2k:
    b       uxn_decode

nipk:
    wpeek8  r3, #-1
    wpush8  r3
    b       uxn_decode

nip2k:
    wpeek16 r3, r5, #-1, #-2
    wpush16 r3
    b       uxn_decode

swpk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    wpush8  r3
    wpush8  r4
    b       uxn_decode

swp2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    wpush16 r3
    wpush16 r4
    b       uxn_decode

rotk:
    wpeek8  r5, #-1
    wpeek8  r4, #-2
    wpeek8  r3, #-3
    wpush8  r4
    wpush8  r5
    wpush8  r3
    b       uxn_decode

rot2k:
    wpeek16 r5, r6, #-1, #-2
    wpeek16 r4, r6, #-3, #-4
    wpeek16 r3, r6, #-5, #-6
    wpush16 r4
    wpush16 r5
    wpush16 r3
    b       uxn_decode

dupk:
    wpeek8  r3, #-1
    wpush8  r3
    wpush8  r3
    b       uxn_decode

dup2k:
    wpeek8  r3, #-2
    wpeek8  r4, #-1
    wpush8  r3
    wpush8  r4
    wpush8  r3
    wpush8  r4
    b       uxn_decode

ovrk:
    wpeek8  r3, #-2
    wpeek8  r4, #-1
    wpush8  r3
    wpush8  r4
    wpush8  r3
    b       uxn_decode

ovr2k:
    wpeek8  r3, #-4
    wpeek8  r4, #-3
    wpeek8  r5, #-2
    wpeek8  r6, #-1
    wpush8  r3
    wpush8  r4
    wpush8  r5
    wpush8  r6
    wpush8  r3
    wpush8  r4
    b       uxn_decode

equk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    sub     r3, r4, r3
    rsbs    r4, r3, #0
    adc     r4, r4, r3
    wpush8  r4
    b       uxn_decode

equ2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    sub     r3, r4, r3
    rsbs    r4, r3, #0
    adc     r4, r4, r3
    wpush8  r4
    b       uxn_decode

neqk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    subs    r3, r4, r3
    movne   r3, #1
    wpush8  r3
    b       uxn_decode

neq2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    subs    r3, r4, r3
    movne   r3, #1
    wpush8  r3
    b       uxn_decode

gthk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    cmp     r4, r3
    movls   r3, #0
    movhi   r3, #1
    wpush8  r3
    b       uxn_decode

gth2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    cmp     r4, r3
    movls   r3, #0
    movhi   r3, #1
    wpush8  r3
    b       uxn_decode

lthk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    cmp     r4, r3
    movcs   r3, #0
    movcc   r3, #1
    wpush8  r3
    b       uxn_decode

lth2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    cmp     r4, r3
    movcs   r3, #0
    movcc   r3, #1
    wpush8  r3
    b       uxn_decode

jmpk:
    wpeek8s r3, #-1
    add     r0, r3
    b       uxn_decode

jmp2k:
    wpeek16 r3, r5, #-1, #-2
    mov     r0, r7
    add     r0, r0, r3
    b       uxn_decode

jcnk:
    wpeek8s r3, #-1
    wpeek8  r4, #-2
    cmp     r4, #0
    addne   r0, r3
    b       uxn_decode

jcn2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek8  r4, #-3
    cmp     r4, #0
    movne   r0, r7
    cmp     r4, #0
    addne   r0, r0, r3
    b       uxn_decode

jsrk:
    mov     r3, r0
    sub     r3, r3, r7
    wpeek8s r4, #-1
    rpush16 r3
    add     r0, r4
    b       uxn_decode

jsr2k:
    mov     r3, r0
    sub     r3, r3, r7
    wpeek16 r4, r5, #-1, #-2
    rpush16 r3
    mov     r0, r7
    add     r0, r0, r4
    b       uxn_decode

sthk:
    wpeek8  r3, #-1
    rpush8  r3
    b       uxn_decode

sth2k:
    wpeek16 r3, r5, #-1, #-2
    rpush16 r3
    b       uxn_decode

ldzk:
    wpeek8  r3, #-1
    zload8  r4, r3
    wpush8  r4
    b       uxn_decode

ldz2k:
    wpeek8  r3, #-1
    zload8  r4, r3
    wpush8  r4
    add     r3, #1
    zload8  r4, r3
    wpush8  r4
    b       uxn_decode

stzk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    zsave8  r4, r3
    b       uxn_decode

stz2k:
    wpeek8  r3, #-1
    wpeek16 r4, r5, #-2, #-3
    zsave16 r4, r3
    b       uxn_decode

ldrk:
    wpeek8s r4, #-1
    rload8  r3, r4
    wpush8  r3
    b       uxn_decode

ldr2k:
    wpeek8s r4, #-1
    rload8  r3, r4
    wpush8  r3
    add     r4, #1
    rload8  r3, r4
    wpush8  r3
    b       uxn_decode

strk:
    wpeek8s r4, #-1
    wpeek8  r3, #-2
    rsave8  r3, r4
    b       uxn_decode

str2k:
    wpeek8s r4, #-1
    wpeek16 r3, r5, #-2, #-3
    rsave16 r3, r4
    b       uxn_decode

ldak:
    wpeek16 r4, r5, #-1, #-2
    aload8  r3, r4
    wpush8  r3
    b       uxn_decode

lda2k:
    wpeek16 r4, r5, #-1, #-2
    aload8  r3, r4
    wpush8  r3
    add     r4, #1
    aload8  r3, r4
    wpush8  r3
    b       uxn_decode

stak:
    wpeek16 r4, r5, #-1, #-2
    wpeek8  r3, #-3
    asave8  r3, r4
    b       uxn_decode

sta2k:
    wpeek16 r4, r5, #-1, #-2
    wpeek16 r3, r5, #-3, #-4
    asave16 r3, r4
    b       uxn_decode

addk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    add     r3, r3, r4
    wpush8  r3
    b       uxn_decode

add2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    add     r3, r3, r4
    wpush16 r3
    b       uxn_decode

subk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    sub     r4, r4, r3
    wpush8  r4
    b       uxn_decode

sub2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    sub     r3, r4, r3
    wpush16 r3
    b       uxn_decode

mulk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    mul     r4, r3, r4
    wpush8  r4
    b       uxn_decode

mul2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    mul     r3, r4, r3
    wpush16 r3
    b       uxn_decode

divk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    push    {r0, r1, r2, r7, lr}
    mov     r1, r3
    mov     r0, r4
    bl      uxn_uidiv
    mov     r3, r0
    pop     {r0, r1, r2, r7, lr}
    wpush8  r3
    b       uxn_decode

div2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    push    {r0, r1, r2, r7, lr}
    mov     r1, r3
    mov     r0, r4
    bl      uxn_uidiv
    mov     r3, r0
    pop     {r0, r1, r2, r7, lr}
    wpush16 r3
    b       uxn_decode

andk:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    and     r3, r3, r4
    wpush8  r3
    b       uxn_decode

and2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    and     r3, r3, r4
    wpush16 r3
    b       uxn_decode

orak:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    orr     r3, r3, r4
    wpush8  r3
    b       uxn_decode

ora2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    orr     r3, r3, r4
    wpush16 r3
    b       uxn_decode

eork:
    wpeek8  r3, #-1
    wpeek8  r4, #-2
    eor     r3, r3, r4
    wpush8  r3
    b       uxn_decode

eor2k:
    wpeek16 r3, r5, #-1, #-2
    wpeek16 r4, r5, #-3, #-4
    eor     r3, r3, r4
    wpush16 r3
    b       uxn_decode

sftk:
    wpeek8  r4, #-1
    wpeek8  r3, #-2
    lsr r5, r4, #4
    and r4, #0x0f
    lsr r3, r3, r4
    lsl r3, r3, r5
    wpush8  r3
    b       uxn_decode

sft2k:
    wpeek8  r4, #-1
    wpeek16 r3, r5, #-2, #-3
    lsr r5, r4, #4
    and r4, #0x0f
    lsr r3, r3, r4
    lsl r3, r3, r5
    wpush16 r3
    b       uxn_decode

.ltorg
.align 2

inckr:
    rpeek8  r3, #-1
    add     r3, #1
    rpush8  r3
    b       uxn_decode

inc2kr:
    rpeek16 r3, r5, #-1, #-2
    add     r3, r3, #1
    rpush16 r3
    b       uxn_decode

popkr:
    b       uxn_decode

pop2kr:
    b       uxn_decode

nipkr:
    rpeek8  r3, #-1
    wpush8  r3
    b       uxn_decode

nip2kr:
    rpeek16 r3, r5, #-1, #-2
    rpush16 r3
    b       uxn_decode

swpkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    rpush8  r3
    rpush8  r4
    b       uxn_decode

swp2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    rpush16 r3
    rpush16 r4
    b       uxn_decode

rotkr:
    rpeek8  r5, #-1
    rpeek8  r4, #-2
    rpeek8  r3, #-3
    rpush8  r4
    rpush8  r5
    rpush8  r3
    b       uxn_decode

rot2kr:
    rpeek16 r5, r6, #-1, #-2
    rpeek16 r4, r6, #-3, #-4
    rpeek16 r3, r6, #-5, #-6
    rpush16 r4
    rpush16 r5
    rpush16 r3
    b       uxn_decode

dupkr:
    rpeek8  r3, #-1
    rpush8  r3
    rpush8  r3
    b       uxn_decode

dup2kr:
    rpeek8  r3, #-2
    rpeek8  r4, #-1
    rpush8  r3
    rpush8  r4
    rpush8  r3
    rpush8  r4
    b       uxn_decode

ovrkr:
    rpeek8  r3, #-2
    rpeek8  r4, #-1
    rpush8  r3
    rpush8  r4
    rpush8  r3
    b       uxn_decode

ovr2kr:
    rpeek8  r3, #-4
    rpeek8  r4, #-3
    rpeek8  r5, #-2
    rpeek8  r6, #-1
    rpush8  r3
    rpush8  r4
    rpush8  r5
    rpush8  r6
    rpush8  r3
    rpush8  r4
    b       uxn_decode

equkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    sub     r3, r4, r3
    rsbs    r4, r3, #0
    adc     r4, r4, r3
    rpush8  r4
    b       uxn_decode

equ2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    sub     r3, r4, r3
    rsbs    r4, r3, #0
    adc     r4, r4, r3
    rpush8  r4
    b       uxn_decode

neqkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    subs    r3, r4, r3
    movne   r3, #1
    rpush8  r3
    b       uxn_decode

neq2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    subs    r3, r4, r3
    movne   r3, #1
    rpush8  r3
    b       uxn_decode

gthkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    cmp     r4, r3
    movls   r3, #0
    movhi   r3, #1
    rpush8  r3
    b       uxn_decode

gth2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    cmp     r4, r3
    movls   r3, #0
    movhi   r3, #1
    rpush8  r3
    b       uxn_decode

lthkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    cmp     r4, r3
    movcs   r3, #0
    movcc   r3, #1
    rpush8  r3
    b       uxn_decode

lth2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    cmp     r4, r3
    movcs   r3, #0
    movcc   r3, #1
    rpush8  r3
    b       uxn_decode

jmpkr:
    rpeek8s r3, #-1
    add     r0, r3
    b       uxn_decode

jmp2kr:
    rpeek16 r3, r5, #-1, #-2
    mov     r0, r7
    add     r0, r0, r3
    b       uxn_decode

jcnkr:
    rpeek8s r3, #-1
    rpeek8  r4, #-2
    cmp     r4, #0
    addne   r0, r3
    b       uxn_decode

jcn2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek8  r4, #-3
    cmp     r4, #0
    movne   r0, r7
    cmp     r4, #0
    addne   r0, r0, r3
    b       uxn_decode

jsrkr:
    mov     r3, r0
    sub     r3, r3, r7
    rpeek8s r4, #-1
    rpush16 r3
    add     r0, r4
    b       uxn_decode

jsr2kr:
    mov     r3, r0
    sub     r3, r3, r7
    rpeek16 r4, r5, #-1, #-2
    rpush16 r3
    mov     r0, r7
    add     r0, r0, r4
    b       uxn_decode

sthkr:
    rpeek8  r3, #-1
    wpush8  r3
    b       uxn_decode

sth2kr:
    rpeek16 r3, r5, #-1, #-2
    wpush16 r3
    b       uxn_decode

ldzkr:
    rpeek8  r3, #-1
    zload8  r4, r3
    rpush8  r4
    b       uxn_decode

ldz2kr:
    rpeek8  r3, #-1
    zload8  r4, r3
    rpush8  r4
    add     r3, #1
    zload8  r4, r3
    rpush8  r4
    b       uxn_decode

stzkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    zsave8  r4, r3
    b       uxn_decode

stz2kr:
    rpeek8  r3, #-1
    rpeek16 r4, r5, #-2, #-3
    zsave16 r4, r3
    b       uxn_decode

ldrkr:
    rpeek8s r4, #-1
    rload8  r3, r4
    rpush8  r3
    b       uxn_decode

ldr2kr:
    rpeek8s r4, #-1
    rload8  r3, r4
    rpush8  r3
    add     r4, #1
    rload8  r3, r4
    rpush8  r3
    b       uxn_decode

strkr:
    rpeek8s r4, #-1
    rpeek8  r3, #-2
    rsave8  r3, r4
    b       uxn_decode

str2kr:
    rpeek8s r4, #-1
    rpeek16 r3, r5, #-1, #-2
    rsave16 r3, r4
    b       uxn_decode

ldakr:
    rpeek16 r4, r5, #-1, #-2
    aload8  r3, r4
    rpush8  r3
    b       uxn_decode

lda2kr:
    rpeek16 r4, r5, #-1, #-2
    aload8  r3, r4
    rpush8  r3
    add     r4, #1
    aload8  r3, r4
    rpush8  r3
    b       uxn_decode

stakr:
    rpeek16 r4, r5, #-1, #-2
    rpeek8  r3, #-3
    asave8  r3, r4
    b       uxn_decode

sta2kr:
    rpeek16 r4, r5, #-1, #-2
    rpeek16 r3, r5, #-3, #-4
    asave16 r3, r4
    b       uxn_decode

addkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    add     r3, r3, r4
    rpush8  r3
    b       uxn_decode

add2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    add     r3, r3, r4
    rpush16 r3
    b       uxn_decode

subkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    sub     r4, r4, r3
    rpush8  r4
    b       uxn_decode

sub2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    sub     r3, r4, r3
    rpush16 r3
    b       uxn_decode

mulkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    mul     r4, r3, r4
    rpush8  r4
    b       uxn_decode

mul2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    mul     r3, r4, r3
    rpush16 r3
    b       uxn_decode

divkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    push    {r0, r1, r2, r7, lr}
    mov     r1, r3
    mov     r0, r4
    bl      uxn_uidiv
    mov     r3, r0
    pop     {r0, r1, r2, r7, lr}
    rpush8  r3
    b       uxn_decode

div2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    push    {r0, r1, r2, r7, lr}
    mov     r1, r3
    mov     r0, r4
    bl      uxn_uidiv
    mov     r3, r0
    pop     {r0, r1, r2, r7, lr}
    rpush16 r3
    b       uxn_decode

andkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    and     r3, r3, r4
    rpush8  r3
    b       uxn_decode

and2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    and     r3, r3, r4
    rpush16 r3
    b       uxn_decode

orakr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    orr     r3, r3, r4
    rpush8  r3
    b       uxn_decode

ora2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    orr     r3, r3, r4
    rpush16 r3
    b       uxn_decode

eorkr:
    rpeek8  r3, #-1
    rpeek8  r4, #-2
    eor     r3, r3, r4
    rpush8  r3
    b       uxn_decode

eor2kr:
    rpeek16 r3, r5, #-1, #-2
    rpeek16 r4, r5, #-3, #-4
    eor     r3, r3, r4
    rpush16 r3
    b       uxn_decode

sftkr:
    rpeek8  r4, #-1
    rpeek8  r3, #-2
    lsr r5, r4, #4
    and r4, #0x0f
    lsr r3, r3, r4
    lsl r3, r3, r5
    rpush8  r3
    b       uxn_decode

sft2kr:
    rpeek8  r4, #-1
    rpeek16 r3, r5, #-2, #-3
    lsr r5, r4, #4
    and r4, #0x0f
    lsr r3, r3, r4
    lsl r3, r3, r5
    rpush16 r3
    b       uxn_decode
