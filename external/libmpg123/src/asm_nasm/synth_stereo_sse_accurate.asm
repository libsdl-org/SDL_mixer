;  1 "synth_stereo_sse_accurate.S"
;  1 "<built-in>"
;  1 "<command line>"
;  1 "synth_stereo_sse_accurate.S"
;  9 "synth_stereo_sse_accurate.S"
;  1 "mangle.h" 1
;  13 "mangle.h"
;  1 "config.h" 1
;  14 "mangle.h" 2
;  1 "intsym.h" 1
;  15 "mangle.h" 2
;  10 "synth_stereo_sse_accurate.S" 2
;  31 "synth_stereo_sse_accurate.S"

%include "asm_nasm.inc"

_sym_mangle synth_1to1_s_sse_accurate_asm

SECTION .data

align 32
maxmin_s16:
 dd 1191181824
 dd 1191181824
 dd 1191181824
 dd 1191181824
 dd -956301312
 dd -956301312
 dd -956301312
 dd -956301312

SECTION .text
align 16
GLOBAL synth_1to1_s_sse_accurate_asm
synth_1to1_s_sse_accurate_asm:
	push	dword ebp
	mov	dword ebp, esp
	and	dword esp, -16
	sub	dword esp, 128
	push	dword ebx
	push	dword esi
	push	dword edi

	pxor	mm7, mm7

	mov	dword ebx, [ebp+8]
	mov	dword edx, [ebp+12]
	mov	dword esi, [ebp+16]
	mov	dword edi, [ebp+20]
	mov	dword eax, [ebp+24]
	shl	dword eax, 2

	lea	dword ebx, [ebx+64]
	sub	dword ebx, eax

	mov	dword ecx, 4

align 16
Loop_start_1:
	movups	xmm0, [ebx]
	movups	xmm1, [ebx+16]
	movups	xmm2, [ebx+32]
	movups	xmm3, [ebx+48]
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	mulps	xmm0, [edx+0]
	mulps	xmm1, [edx+16]
	mulps	xmm2, [edx+32]
	mulps	xmm3, [edx+48]
	mulps	xmm4, [esi+0]
	mulps	xmm5, [esi+16]
	mulps	xmm6, [esi+32]
	mulps	xmm7, [esi+48]
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	addps	xmm6, xmm7
	addps	xmm0, xmm2
	addps	xmm4, xmm6
	movaps	[esp+(12+16*0)], xmm0
	movaps	[esp+(12+16*4)], xmm4

	lea	dword ebx, [ebx+128]
	lea	dword edx, [edx+64]
	lea	dword esi, [esi+64]

	movups	xmm0, [ebx]
	movups	xmm1, [ebx+16]
	movups	xmm2, [ebx+32]
	movups	xmm3, [ebx+48]
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	mulps	xmm0, [edx+0]
	mulps	xmm1, [edx+16]
	mulps	xmm2, [edx+32]
	mulps	xmm3, [edx+48]
	mulps	xmm4, [esi+0]
	mulps	xmm5, [esi+16]
	mulps	xmm6, [esi+32]
	mulps	xmm7, [esi+48]
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	addps	xmm6, xmm7
	addps	xmm0, xmm2
	addps	xmm4, xmm6
	movaps	[esp+(12+16*1)], xmm0
	movaps	[esp+(12+16*5)], xmm4

	lea	dword ebx, [ebx+128]
	lea	dword edx, [edx+64]
	lea	dword esi, [esi+64]

	movups	xmm0, [ebx]
	movups	xmm1, [ebx+16]
	movups	xmm2, [ebx+32]
	movups	xmm3, [ebx+48]
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	mulps	xmm0, [edx+0]
	mulps	xmm1, [edx+16]
	mulps	xmm2, [edx+32]
	mulps	xmm3, [edx+48]
	mulps	xmm4, [esi+0]
	mulps	xmm5, [esi+16]
	mulps	xmm6, [esi+32]
	mulps	xmm7, [esi+48]
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	addps	xmm6, xmm7
	addps	xmm0, xmm2
	addps	xmm4, xmm6
	movaps	[esp+(12+16*2)], xmm0
	movaps	[esp+(12+16*6)], xmm4

	lea	dword ebx, [ebx+128]
	lea	dword edx, [edx+64]
	lea	dword esi, [esi+64]

	movups	xmm0, [ebx]
	movups	xmm1, [ebx+16]
	movups	xmm2, [ebx+32]
	movups	xmm3, [ebx+48]
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	mulps	xmm0, [edx+0]
	mulps	xmm1, [edx+16]
	mulps	xmm2, [edx+32]
	mulps	xmm3, [edx+48]
	mulps	xmm4, [esi+0]
	mulps	xmm5, [esi+16]
	mulps	xmm6, [esi+32]
	mulps	xmm7, [esi+48]
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	addps	xmm6, xmm7
	addps	xmm0, xmm2
	addps	xmm4, xmm6
	movaps	xmm7, xmm0
	movaps	[esp+(12+16*7)], xmm4

	lea	dword ebx, [ebx+128]
	lea	dword edx, [edx+64]
	lea	dword esi, [esi+64]

	movaps	xmm4, [esp+(12+16*0)]
	movaps	xmm5, [esp+(12+16*1)]
	movaps	xmm6, [esp+(12+16*2)]
	movaps	xmm0, xmm4
	movaps	xmm1, xmm6
	unpcklps	xmm4, xmm5
	unpcklps	xmm6, xmm7
	unpckhps	xmm0, xmm5
	unpckhps	xmm1, xmm7
	movaps	xmm2, xmm4
	movaps	xmm3, xmm0
	movlhps	xmm4, xmm6
	movhlps	xmm6, xmm2
	movlhps	xmm0, xmm1
	movhlps	xmm1, xmm3
	subps	xmm4, xmm6
	subps	xmm0, xmm1
	addps	xmm0, xmm4
	movaps	xmm2, xmm0

	movaps	xmm4, [esp+(12+16*4)]
	movaps	xmm5, [esp+(12+16*5)]
	movaps	xmm6, [esp+(12+16*6)]
	movaps	xmm7, [esp+(12+16*7)]
	movaps	xmm0, xmm4
	movaps	xmm1, xmm6
	unpcklps	xmm4, xmm5
	unpcklps	xmm6, xmm7
	unpckhps	xmm0, xmm5
	unpckhps	xmm1, xmm7
	movaps	xmm5, xmm2
	movaps	xmm2, xmm4
	movaps	xmm3, xmm0
	movlhps	xmm4, xmm6
	movhlps	xmm6, xmm2
	movlhps	xmm0, xmm1
	movhlps	xmm1, xmm3
	subps	xmm4, xmm6
	subps	xmm0, xmm1
	addps	xmm0, xmm4

	movaps	xmm1, xmm5
	movaps	xmm2, xmm5
	movaps	xmm3, xmm0
	movaps	xmm4, xmm0
	cmpnleps	xmm1, [maxmin_s16]
	cmpltps	xmm2, [maxmin_s16+16]
	cmpnleps	xmm3, [maxmin_s16]
	cmpltps	xmm4, [maxmin_s16+16]
	cvtps2pi	mm0, xmm5
	cvtps2pi	mm1, xmm0
	movhlps	xmm5, xmm5
	movhlps	xmm0, xmm0
	cvtps2pi	mm2, xmm5
	cvtps2pi	mm3, xmm0
	packssdw	mm0, mm2
	packssdw	mm1, mm3
	movq	mm2, mm0
	punpcklwd	mm0, mm1
	punpckhwd	mm2, mm1
	movq	[edi], mm0
	movq	[edi+8], mm2

	cvtps2pi	mm2, xmm1
	cvtps2pi	mm3, xmm3
	movhlps	xmm1, xmm1
	movhlps	xmm3, xmm3
	cvtps2pi	mm4, xmm1
	cvtps2pi	mm5, xmm3
	packssdw	mm2, mm4
	packssdw	mm3, mm5
	psrlw	mm2, 15
	psrlw	mm3, 15
	cvtps2pi	mm0, xmm2
	cvtps2pi	mm1, xmm4
	movhlps	xmm2, xmm2
	movhlps	xmm4, xmm4
	cvtps2pi	mm4, xmm2
	cvtps2pi	mm5, xmm4
	packssdw	mm0, mm4
	packssdw	mm1, mm5
	psrlw	mm0, 15
	psrlw	mm1, 15
	paddw	mm2, mm3
	paddw	mm0, mm1
	paddw	mm0, mm2
	paddw	mm7, mm0

	lea	dword edi, [edi+16]
	dec	dword ecx
	jnz	Loop_start_1

	mov	dword ecx, 4

align 16
Loop_start_2:
	movups	xmm0, [ebx]
	movups	xmm1, [ebx+16]
	movups	xmm2, [ebx+32]
	movups	xmm3, [ebx+48]
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	mulps	xmm0, [edx+0]
	mulps	xmm1, [edx+16]
	mulps	xmm2, [edx+32]
	mulps	xmm3, [edx+48]
	mulps	xmm4, [esi+0]
	mulps	xmm5, [esi+16]
	mulps	xmm6, [esi+32]
	mulps	xmm7, [esi+48]
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	addps	xmm6, xmm7
	addps	xmm0, xmm2
	addps	xmm4, xmm6
	movaps	[esp+(12+16*0)], xmm0
	movaps	[esp+(12+16*4)], xmm4

	lea	dword ebx, [ebx+128]
	lea	dword edx, [edx+-64]
	lea	dword esi, [esi+-64]

	movups	xmm0, [ebx]
	movups	xmm1, [ebx+16]
	movups	xmm2, [ebx+32]
	movups	xmm3, [ebx+48]
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	mulps	xmm0, [edx+0]
	mulps	xmm1, [edx+16]
	mulps	xmm2, [edx+32]
	mulps	xmm3, [edx+48]
	mulps	xmm4, [esi+0]
	mulps	xmm5, [esi+16]
	mulps	xmm6, [esi+32]
	mulps	xmm7, [esi+48]
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	addps	xmm6, xmm7
	addps	xmm0, xmm2
	addps	xmm4, xmm6
	movaps	[esp+(12+16*1)], xmm0
	movaps	[esp+(12+16*5)], xmm4

	lea	dword ebx, [ebx+128]
	lea	dword edx, [edx+-64]
	lea	dword esi, [esi+-64]

	movups	xmm0, [ebx]
	movups	xmm1, [ebx+16]
	movups	xmm2, [ebx+32]
	movups	xmm3, [ebx+48]
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	mulps	xmm0, [edx+0]
	mulps	xmm1, [edx+16]
	mulps	xmm2, [edx+32]
	mulps	xmm3, [edx+48]
	mulps	xmm4, [esi+0]
	mulps	xmm5, [esi+16]
	mulps	xmm6, [esi+32]
	mulps	xmm7, [esi+48]
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	addps	xmm6, xmm7
	addps	xmm0, xmm2
	addps	xmm4, xmm6
	movaps	[esp+(12+16*2)], xmm0
	movaps	[esp+(12+16*6)], xmm4

	lea	dword ebx, [ebx+128]
	lea	dword edx, [edx+-64]
	lea	dword esi, [esi+-64]

	movups	xmm0, [ebx]
	movups	xmm1, [ebx+16]
	movups	xmm2, [ebx+32]
	movups	xmm3, [ebx+48]
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	mulps	xmm0, [edx+0]
	mulps	xmm1, [edx+16]
	mulps	xmm2, [edx+32]
	mulps	xmm3, [edx+48]
	mulps	xmm4, [esi+0]
	mulps	xmm5, [esi+16]
	mulps	xmm6, [esi+32]
	mulps	xmm7, [esi+48]
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	addps	xmm6, xmm7
	addps	xmm0, xmm2
	addps	xmm4, xmm6
	movaps	xmm7, xmm0
	movaps	[esp+(12+16*7)], xmm4

	lea	dword ebx, [ebx+128]
	lea	dword edx, [edx+-64]
	lea	dword esi, [esi+-64]

	movaps	xmm4, [esp+(12+16*0)]
	movaps	xmm5, [esp+(12+16*1)]
	movaps	xmm6, [esp+(12+16*2)]
	movaps	xmm0, xmm4
	movaps	xmm1, xmm6
	unpcklps	xmm4, xmm5
	unpcklps	xmm6, xmm7
	unpckhps	xmm0, xmm5
	unpckhps	xmm1, xmm7
	movaps	xmm2, xmm4
	movaps	xmm3, xmm0
	movlhps	xmm4, xmm6
	movhlps	xmm6, xmm2
	movlhps	xmm0, xmm1
	movhlps	xmm1, xmm3
	addps	xmm4, xmm6
	addps	xmm0, xmm1
	addps	xmm0, xmm4
	movaps	xmm2, xmm0

	movaps	xmm4, [esp+(12+16*4)]
	movaps	xmm5, [esp+(12+16*5)]
	movaps	xmm6, [esp+(12+16*6)]
	movaps	xmm7, [esp+(12+16*7)]
	movaps	xmm0, xmm4
	movaps	xmm1, xmm6
	unpcklps	xmm4, xmm5
	unpcklps	xmm6, xmm7
	unpckhps	xmm0, xmm5
	unpckhps	xmm1, xmm7
	movaps	xmm5, xmm2
	movaps	xmm2, xmm4
	movaps	xmm3, xmm0
	movlhps	xmm4, xmm6
	movhlps	xmm6, xmm2
	movlhps	xmm0, xmm1
	movhlps	xmm1, xmm3
	addps	xmm4, xmm6
	addps	xmm0, xmm1
	addps	xmm0, xmm4

	movaps	xmm1, xmm5
	movaps	xmm2, xmm5
	movaps	xmm3, xmm0
	movaps	xmm4, xmm0
	cmpnleps	xmm1, [maxmin_s16]
	cmpltps	xmm2, [maxmin_s16+16]
	cmpnleps	xmm3, [maxmin_s16]
	cmpltps	xmm4, [maxmin_s16+16]
	cvtps2pi	mm0, xmm5
	cvtps2pi	mm1, xmm0
	movhlps	xmm5, xmm5
	movhlps	xmm0, xmm0
	cvtps2pi	mm2, xmm5
	cvtps2pi	mm3, xmm0
	packssdw	mm0, mm2
	packssdw	mm1, mm3
	movq	mm2, mm0
	punpcklwd	mm0, mm1
	punpckhwd	mm2, mm1
	movq	[edi], mm0
	movq	[edi+8], mm2

	cvtps2pi	mm2, xmm1
	cvtps2pi	mm3, xmm3
	movhlps	xmm1, xmm1
	movhlps	xmm3, xmm3
	cvtps2pi	mm4, xmm1
	cvtps2pi	mm5, xmm3
	packssdw	mm2, mm4
	packssdw	mm3, mm5
	psrlw	mm2, 15
	psrlw	mm3, 15
	cvtps2pi	mm0, xmm2
	cvtps2pi	mm1, xmm4
	movhlps	xmm2, xmm2
	movhlps	xmm4, xmm4
	cvtps2pi	mm4, xmm2
	cvtps2pi	mm5, xmm4
	packssdw	mm0, mm4
	packssdw	mm1, mm5
	psrlw	mm0, 15
	psrlw	mm1, 15
	paddw	mm2, mm3
	paddw	mm0, mm1
	paddw	mm0, mm2
	paddw	mm7, mm0

	lea	dword edi, [edi+16]
	dec	dword ecx
	jnz	Loop_start_2

	pshufw	mm0, mm7, 0xee
	paddw	mm0, mm7
	pshufw	mm1, mm0, 0x55
	paddw	mm0, mm1
	movd	eax, mm0
	and	dword eax, 0xffff

	pop	dword edi
	pop	dword esi
	pop	dword ebx
	mov	dword esp, ebp
	pop	dword ebp

        emms

        ret


