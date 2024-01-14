	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0
	.globl	_provaArray                     ; -- Begin function provaArray
	.p2align	2
_provaArray:                            ; @provaArray
	.cfi_startproc
; %bb.0:                                ; %entry
	sub	sp, sp, #192
	stp	d9, d8, [sp, #144]              ; 16-byte Folded Spill
	stp	x20, x19, [sp, #160]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #176]            ; 16-byte Folded Spill
	.cfi_def_cfa_offset 192
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset b8, -40
	.cfi_offset b9, -48
	mov	x8, #4607182418800017408        ; =0x3ff0000000000000
	mov	x9, #4611686018427387904        ; =0x4000000000000000
	mov	x10, #4613937818241073152       ; =0x4008000000000000
	mov	x11, #4616189618054758400       ; =0x4010000000000000
Lloh0:
	adrp	x19, _i@GOTPAGE
	str	d0, [sp, #8]
	stp	x8, x9, [sp, #16]
	mov	x8, #4617315517961601024        ; =0x4014000000000000
	mov	x9, #4618441417868443648        ; =0x4018000000000000
	stp	x10, x11, [sp, #32]
	mov	x10, #4619567317775286272       ; =0x401c000000000000
	mov	x11, #4620693217682128896       ; =0x4020000000000000
	fmov	d8, #14.00000000
	add	x20, sp, #16
	stp	x8, x9, [sp, #48]
	mov	x8, #4621256167635550208        ; =0x4022000000000000
	mov	x9, #4621819117588971520        ; =0x4024000000000000
	stp	x10, x11, [sp, #64]
	mov	x10, #4622382067542392832       ; =0x4026000000000000
	mov	x11, #4622945017495814144       ; =0x4028000000000000
	fmov	d9, #1.00000000
	stp	x8, x9, [sp, #80]
	mov	x8, #4623507967449235456        ; =0x402a000000000000
	stp	x10, x11, [sp, #96]
	mov	x10, #4624070917402656768       ; =0x402c000000000000
Lloh1:
	ldr	x19, [x19, _i@GOTPAGEOFF]
	str	xzr, [sp, #128]
	stp	x8, x10, [sp, #112]
	str	x9, [x19]
LBB0_1:                                 ; %condstmt
                                        ; =>This Inner Loop Header: Depth=1
	ldr	d0, [sp, #128]
	fcmp	d0, d8
	b.ge	LBB0_3
; %bb.2:                                ; %loopstmt
                                        ;   in Loop: Header=BB0_1 Depth=1
	ldr	d0, [sp, #128]
	fcvt	s0, d0
	fcvtzs	w8, s0
	ldr	d0, [x20, w8, sxtw #3]
	str	d0, [sp, #136]
	bl	_printval
	ldr	d0, [sp, #128]
	fadd	d0, d0, d9
	str	d0, [sp, #128]
	b	LBB0_1
LBB0_3:                                 ; %mergestmt
	ldr	d0, [x19]
	ldp	x29, x30, [sp, #176]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #160]            ; 16-byte Folded Reload
	ldp	d9, d8, [sp, #144]              ; 16-byte Folded Reload
	add	sp, sp, #192
	ret
	.loh AdrpLdrGot	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.comm	_i,8,3                          ; @i
.subsections_via_symbols
