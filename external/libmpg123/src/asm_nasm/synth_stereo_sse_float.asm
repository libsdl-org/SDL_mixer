;  1 "synth_stereo_sse_float.S"
;  1 "<built-in>"
;  1 "<command line>"
;  1 "synth_stereo_sse_float.S"
;  9 "synth_stereo_sse_float.S"
;  1 "mangle.h" 1
;  13 "mangle.h"
;  1 "config.h" 1
;  14 "mangle.h" 2
;  1 "intsym.h" 1
;  15 "mangle.h" 2
;  10 "synth_stereo_sse_float.S" 2
;  30 "synth_stereo_sse_float.S"

%include "asm_nasm.inc"

_sym_mangle synth_1to1_real_s_sse_asm

SECTION .data

align 32
scale_sse:
 dd 939524096
 dd 939524096
 dd 939524096
 dd 939524096

SECTION .text
align 16
GLOBAL synth_1to1_real_s_sse_asm
synth_1to1_real_s_sse_asm:
	push	dword ebp
	mov	dword ebp, esp
	and	dword esp, -16
	sub	dword esp, 128
	push	dword ebx
	push	dword esi
	push	dword edi

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

	mulps	xmm5, [scale_sse]
	mulps	xmm0, [scale_sse]
	movaps	xmm1, xmm5
	unpcklps	xmm5, xmm0
	unpckhps	xmm1, xmm0
	movups	[edi], xmm5
	movups	[edi+16], xmm1

	lea	dword edi, [edi+32]
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

	mulps	xmm5, [scale_sse]
	mulps	xmm0, [scale_sse]
	movaps	xmm1, xmm5
	unpcklps	xmm5, xmm0
	unpckhps	xmm1, xmm0
	movups	[edi], xmm5
	movups	[edi+16], xmm1

	lea	dword edi, [edi+32]
	dec	dword ecx
	jnz	Loop_start_2

	xor	dword eax, eax

	pop	dword edi
	pop	dword esi
	pop	dword ebx
	mov	dword esp, ebp
	pop	dword ebp

        ret


