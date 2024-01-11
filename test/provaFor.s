	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0
	.globl	_provaFor                       ; -- Begin function provaFor
	.p2align	2
_provaFor:                              ; @provaFor
	.cfi_startproc
; %bb.0:                                ; %entry
	sub	sp, sp, #48
	stp	d9, d8, [sp, #16]               ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	.cfi_def_cfa_offset 48
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset b8, -24
	.cfi_offset b9, -32
	fmov	d8, #1.00000000
	str	d0, [sp]
	str	xzr, [sp, #8]
LBB0_1:                                 ; %loopstmt
                                        ; =>This Inner Loop Header: Depth=1
	ldr	d0, [sp, #8]
	bl	_printval
	ldp	d1, d0, [sp]
	fadd	d0, d0, d8
	fcmp	d0, d1
	str	d0, [sp, #8]
	b.lt	LBB0_1
; %bb.2:                                ; %condstmt6.preheader
	fmov	d8, #-1.00000000
LBB0_3:                                 ; %condstmt6
                                        ; =>This Inner Loop Header: Depth=1
	ldr	d0, [sp, #8]
	fcmp	d0, #0.0
	b.ls	LBB0_5
; %bb.4:                                ; %loopstmt8
                                        ;   in Loop: Header=BB0_3 Depth=1
	ldr	d0, [sp, #8]
	bl	_printval
	ldr	d0, [sp, #8]
	fadd	d0, d0, d8
	str	d0, [sp, #8]
	b	LBB0_3
LBB0_5:                                 ; %mergestmt12
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	movi	d0, #0000000000000000
	ldp	d9, d8, [sp, #16]               ; 16-byte Folded Reload
	add	sp, sp, #48
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
