;  1 "dct64_sse.S"
;  1 "<built-in>"
;  1 "<command line>"
;  1 "dct64_sse.S"
;  9 "dct64_sse.S"
;  1 "mangle.h" 1
;  13 "mangle.h"
;  1 "config.h" 1
;  14 "mangle.h" 2
;  1 "intsym.h" 1
;  15 "mangle.h" 2
;  10 "dct64_sse.S" 2
;  22 "dct64_sse.S"

%include "asm_nasm.inc"

_sym_mangle costab_mmxsse
_sym_mangle dct64_sse

EXTERN costab_mmxsse

SECTION .data

align 16
pnpn:
 dd 0
 dd -2147483648
 dd 0
 dd -2147483648

align 16
mask:
 dd -1
 dd -1
 dd -1
 dd 0

SECTION .text
align 16
GLOBAL dct64_sse
dct64_sse:
	push	dword ebp
	mov	dword ebp, esp

	and	dword esp, -16
	sub	dword esp, 128
	push	dword ebx

	mov	dword eax, [ebp+(8+2*4)]

	MOVUAPS	xmm7, [eax]
	MOVUAPS	xmm6, [eax+16]
	MOVUAPS	xmm0, [eax+112]
	MOVUAPS	xmm1, [eax+96]
	shufps	xmm0, xmm0, 0x1b
	shufps	xmm1, xmm1, 0x1b
	movaps	xmm4, xmm7
	movaps	xmm5, xmm6
	addps	xmm4, xmm0
	addps	xmm5, xmm1
	subps	xmm7, xmm0
	subps	xmm6, xmm1
	movaps	[esp+(4+0*16)], xmm4
	movaps	[esp+(4+1*16)], xmm5

	MOVUAPS	xmm2, [eax+32]
	MOVUAPS	xmm3, [eax+48]
	MOVUAPS	xmm0, [eax+80]
	MOVUAPS	xmm1, [eax+64]
	shufps	xmm0, xmm0, 0x1b
	shufps	xmm1, xmm1, 0x1b
	movaps	xmm5, xmm2
	movaps	xmm4, xmm3
	addps	xmm2, xmm0
	addps	xmm3, xmm1
	subps	xmm5, xmm0
	subps	xmm4, xmm1

	lea	ecx, [costab_mmxsse]
	mulps	xmm7, [ecx]
	mulps	xmm6, [ecx+16]
	mulps	xmm5, [ecx+32]
	mulps	xmm4, [ecx+48]

	shufps	xmm2, xmm2, 0x1b
	shufps	xmm3, xmm3, 0x1b
	shufps	xmm4, xmm4, 0x1b
	shufps	xmm5, xmm5, 0x1b
	movaps	xmm0, [esp+(4+0*16)]
	movaps	xmm1, [esp+(4+1*16)]
	subps	xmm0, xmm3
	subps	xmm1, xmm2
	addps	xmm3, [esp+(4+0*16)]
	addps	xmm2, [esp+(4+1*16)]
	movaps	[esp+(4+0*16)], xmm3
	movaps	[esp+(4+1*16)], xmm2
	movaps	xmm2, xmm6
	movaps	xmm3, xmm7
	subps	xmm6, xmm5
	subps	xmm7, xmm4
	addps	xmm4, xmm3
	addps	xmm5, xmm2
	mulps	xmm0, [ecx+64]
	mulps	xmm1, [ecx+80]
	mulps	xmm6, [ecx+80]
	mulps	xmm7, [ecx+64]

	movaps	xmm2, [esp+(4+0*16)]
	movaps	xmm3, [esp+(4+1*16)]
	shufps	xmm3, xmm3, 0x1b
	shufps	xmm5, xmm5, 0x1b
	shufps	xmm1, xmm1, 0x1b
	shufps	xmm6, xmm6, 0x1b
	movaps	[esp+(4+1*16)], xmm0
	subps	xmm2, xmm3
	subps	xmm0, xmm1
	addps	xmm3, [esp+(4+0*16)]
	addps	xmm1, [esp+(4+1*16)]
	movaps	[esp+(4+0*16)], xmm3
	movaps	[esp+(4+2*16)], xmm1
	movaps	xmm1, xmm5
	movaps	xmm5, xmm4
	movaps	xmm3, xmm7
	subps	xmm5, xmm1
	subps	xmm7, xmm6
	addps	xmm4, xmm1
	addps	xmm6, xmm3
	mulps	xmm2, [ecx+96]
	mulps	xmm0, [ecx+96]
	mulps	xmm5, [ecx+96]
	mulps	xmm7, [ecx+96]
	movaps	[esp+(4+1*16)], xmm2
	movaps	[esp+(4+3*16)], xmm0

	movaps	xmm2, xmm4
	movaps	xmm3, xmm5
	shufps	xmm2, xmm6, 0x44
	shufps	xmm5, xmm7, 0xbb
	shufps	xmm4, xmm6, 0xbb
	shufps	xmm3, xmm7, 0x44
	movaps	xmm6, xmm2
	movaps	xmm7, xmm3
	subps	xmm2, xmm4
	subps	xmm3, xmm5
	addps	xmm4, xmm6
	addps	xmm5, xmm7
	movaps	xmm0, [ecx+112]
	movlhps	xmm0, xmm0
	mulps	xmm2, xmm0
	mulps	xmm3, xmm0
	movaps	[esp+(4+4*16)], xmm0
	movaps	xmm6, xmm4
	movaps	xmm7, xmm5
	shufps	xmm4, xmm2, 0x14
	shufps	xmm6, xmm2, 0xbe
	shufps	xmm5, xmm3, 0x14
	shufps	xmm7, xmm3, 0xbe
	movaps	[esp+(4+5*16)], xmm5
	movaps	[esp+(4+7*16)], xmm7

	movaps	xmm0, [esp+(4+0*16)]
	movaps	xmm1, [esp+(4+1*16)]
	movaps	xmm2, xmm0
	movaps	xmm3, xmm1
	shufps	xmm2, [esp+(4+2*16)], 0x44
	shufps	xmm1, [esp+(4+3*16)], 0xbb
	shufps	xmm0, [esp+(4+2*16)], 0xbb
	shufps	xmm3, [esp+(4+3*16)], 0x44
	movaps	xmm5, xmm2
	movaps	xmm7, xmm3
	subps	xmm2, xmm0
	subps	xmm3, xmm1
	addps	xmm0, xmm5
	addps	xmm1, xmm7
	mulps	xmm2, [esp+(4+4*16)]
	mulps	xmm3, [esp+(4+4*16)]
	movaps	xmm5, xmm0
	movaps	xmm7, xmm1
	shufps	xmm0, xmm2, 0x14
	shufps	xmm5, xmm2, 0xbe
	shufps	xmm1, xmm3, 0x14
	shufps	xmm7, xmm3, 0xbe

	movaps	[esp+(4+0*16)], xmm0
	movaps	[esp+(4+1*16)], xmm1
	movaps	[esp+(4+2*16)], xmm5
	movaps	[esp+(4+3*16)], xmm7

	movss	xmm5, [ecx+120]
	shufps	xmm5, xmm5, 0x00
	xorps	xmm5, [pnpn]

	movaps	xmm0, xmm4
	movaps	xmm1, xmm6
	unpcklps	xmm4, [esp+(4+5*16)]
	unpckhps	xmm0, [esp+(4+5*16)]
	unpcklps	xmm6, [esp+(4+7*16)]
	unpckhps	xmm1, [esp+(4+7*16)]
	movaps	xmm2, xmm4
	movaps	xmm3, xmm6
	unpcklps	xmm4, xmm0
	unpckhps	xmm2, xmm0
	unpcklps	xmm6, xmm1
	unpckhps	xmm3, xmm1
	movaps	xmm0, xmm4
	movaps	xmm1, xmm6
	subps	xmm0, xmm2
	subps	xmm1, xmm3
	addps	xmm4, xmm2
	addps	xmm6, xmm3
	mulps	xmm0, xmm5
	mulps	xmm1, xmm5
	movaps	[esp+(4+5*16)], xmm5
	movaps	xmm5, xmm4
	movaps	xmm7, xmm6
	unpcklps	xmm4, xmm0
	unpckhps	xmm5, xmm0
	unpcklps	xmm6, xmm1
	unpckhps	xmm7, xmm1

	movaps	xmm0, [esp+(4+0*16)]
	movaps	xmm2, [esp+(4+2*16)]
	movaps	[esp+(4+4*16)], xmm4
	movaps	[esp+(4+6*16)], xmm6

	movaps	xmm4, xmm0
	movaps	xmm6, xmm2
	unpcklps	xmm0, [esp+(4+1*16)]
	unpckhps	xmm4, [esp+(4+1*16)]
	unpcklps	xmm2, [esp+(4+3*16)]
	unpckhps	xmm6, [esp+(4+3*16)]
	movaps	xmm1, xmm0
	movaps	xmm3, xmm2
	unpcklps	xmm0, xmm4
	unpckhps	xmm1, xmm4
	unpcklps	xmm2, xmm6
	unpckhps	xmm3, xmm6
	movaps	xmm4, xmm0
	movaps	xmm6, xmm2
	subps	xmm4, xmm1
	subps	xmm6, xmm3
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	mulps	xmm4, [esp+(4+5*16)]
	mulps	xmm6, [esp+(4+5*16)]
	movaps	xmm1, xmm0
	movaps	xmm3, xmm2
	unpcklps	xmm0, xmm4
	unpckhps	xmm1, xmm4
	unpcklps	xmm2, xmm6
	unpckhps	xmm3, xmm6

	movaps	[esp+(4+0*16)], xmm0
	movaps	[esp+(4+1*16)], xmm1
	movaps	[esp+(4+2*16)], xmm2
	movaps	[esp+(4+3*16)], xmm3
	movaps	[esp+(4+5*16)], xmm5
	movaps	[esp+(4+7*16)], xmm7

	movss	xmm0, [esp+(4+12)]
	movss	xmm1, [esp+(4+28)]
	movss	xmm2, [esp+(4+44)]
	movss	xmm3, [esp+(4+60)]
	addss	xmm0, [esp+(4+8)]
	addss	xmm1, [esp+(4+24)]
	addss	xmm2, [esp+(4+40)]
	addss	xmm3, [esp+(4+56)]
	movss	[esp+(4+8)], xmm0
	movss	[esp+(4+24)], xmm1
	movss	[esp+(4+40)], xmm2
	movss	[esp+(4+56)], xmm3
	movss	xmm0, [esp+(4+76)]
	movss	xmm1, [esp+(4+92)]
	movss	xmm2, [esp+(4+108)]
	movss	xmm3, [esp+(4+124)]
	addss	xmm0, [esp+(4+72)]
	addss	xmm1, [esp+(4+88)]
	addss	xmm2, [esp+(4+104)]
	addss	xmm3, [esp+(4+120)]
	movss	[esp+(4+72)], xmm0
	movss	[esp+(4+88)], xmm1
	movss	[esp+(4+104)], xmm2
	movss	[esp+(4+120)], xmm3

	movaps	xmm1, [esp+(4+16)]
	movaps	xmm3, [esp+(4+48)]
	movaps	xmm5, [esp+(4+80)]
	movaps	xmm7, [esp+(4+112)]
	movaps	xmm0, xmm1
	movaps	xmm2, xmm3
	movaps	xmm4, xmm5
	movaps	xmm6, xmm7
	shufps	xmm0, xmm0, 0x1e
	shufps	xmm2, xmm2, 0x1e
	shufps	xmm4, xmm4, 0x1e
	shufps	xmm6, xmm6, 0x1e
	lea	ecx, [mask]
	andps	xmm0, [ecx]
	andps	xmm2, [ecx]
	andps	xmm4, [ecx]
	andps	xmm6, [ecx]
	addps	xmm1, xmm0
	addps	xmm3, xmm2
	addps	xmm5, xmm4
	addps	xmm7, xmm6

	movaps	xmm2, [esp+(4+32)]
	movaps	xmm6, [esp+(4+96)]
	movaps	xmm0, xmm2
	movaps	xmm4, xmm6
	shufps	xmm0, xmm0, 0x1e
	shufps	xmm4, xmm4, 0x1e
	andps	xmm0, [ecx]
	andps	xmm4, [ecx]
	addps	xmm2, xmm3
	addps	xmm3, xmm0
	addps	xmm6, xmm7
	addps	xmm7, xmm4

	movaps	xmm0, [esp+(4+0)]
	movaps	xmm4, [esp+(4+64)]

	cvtps2pi	mm0, xmm0
	cvtps2pi	mm1, xmm1
	movhlps	xmm0, xmm0
	movhlps	xmm1, xmm1
	cvtps2pi	mm2, xmm0
	cvtps2pi	mm3, xmm1
	packssdw	mm0, mm2
	packssdw	mm1, mm3

	cvtps2pi	mm2, xmm2
	cvtps2pi	mm3, xmm3
	movhlps	xmm2, xmm2
	movhlps	xmm3, xmm3
	cvtps2pi	mm4, xmm2
	cvtps2pi	mm5, xmm3
	packssdw	mm2, mm4
	packssdw	mm3, mm5

	mov	[esp+4+0],ecx
	mov	ecx, [ebp+(8+0*4)]
	mov	ebx, [ebp+(8+1*4)]

	movd	eax, mm0
	movd	edx, mm1
	mov	word [ecx+512], ax
	mov	word [ecx+384], dx
	shr	dword eax, 16
	shr	dword edx, 16
	mov	word [ecx], ax
	mov	word [ebx], ax
	mov	word [ebx+128], dx

	movd	eax, mm2
	movd	edx, mm3
	mov	word [ecx+448], ax
	mov	word [ecx+320], dx
	shr	dword eax, 16
	shr	dword edx, 16
	mov	word [ebx+64], ax
	mov	word [ebx+192], dx

	psrlq	mm0, 32
	psrlq	mm1, 32
	movd	eax, mm0
	movd	edx, mm1
	mov	word [ecx+256], ax
	mov	word [ecx+128], dx
	shr	dword eax, 16
	shr	dword edx, 16
	mov	word [ebx+256], ax
	mov	word [ebx+384], dx

	psrlq	mm2, 32
	psrlq	mm3, 32
	movd	eax, mm2
	movd	edx, mm3
	mov	word [ecx+192], ax
	mov	word [ecx+64], dx
	shr	dword eax, 16
	shr	dword edx, 16
	mov	word [ebx+320], ax
	mov	word [ebx+448], dx

	mov	eax, [esp+4+0]
	movaps	xmm0, xmm4
	shufps	xmm0, xmm0, 0x1e
	movaps	xmm1, xmm5
	andps	xmm0, [eax]

	addps	xmm4, xmm6
	addps	xmm5, xmm7
	addps	xmm6, xmm1
	addps	xmm7, xmm0

	cvtps2pi	mm0, xmm4
	cvtps2pi	mm1, xmm5
	movhlps	xmm4, xmm4
	movhlps	xmm5, xmm5
	cvtps2pi	mm2, xmm4
	cvtps2pi	mm3, xmm5
	packssdw	mm0, mm2
	packssdw	mm1, mm3

	cvtps2pi	mm2, xmm6
	cvtps2pi	mm3, xmm7
	movhlps	xmm6, xmm6
	movhlps	xmm7, xmm7
	cvtps2pi	mm4, xmm6
	cvtps2pi	mm5, xmm7
	packssdw	mm2, mm4
	packssdw	mm3, mm5

	movd	eax, mm0
	movd	edx, mm2
	mov	word [ecx+480], ax
	mov	word [ecx+416], dx
	shr	dword eax, 16
	shr	dword edx, 16
	mov	word [ebx+32], ax
	mov	word [ebx+96], dx

	psrlq	mm0, 32
	psrlq	mm2, 32
	movd	eax, mm0
	movd	edx, mm2
	mov	word [ecx+224], ax
	mov	word [ecx+160], dx
	shr	dword eax, 16
	shr	dword edx, 16
	mov	word [ebx+288], ax
	mov	word [ebx+352], dx

	movd	eax, mm1
	movd	edx, mm3
	mov	word [ecx+352], ax
	mov	word [ecx+288], dx
	shr	dword eax, 16
	shr	dword edx, 16
	mov	word [ebx+160], ax
	mov	word [ebx+224], dx

	psrlq	mm1, 32
	psrlq	mm3, 32
	movd	eax, mm1
	movd	edx, mm3
	mov	word [ecx+96], ax
	mov	word [ecx+32], dx
	shr	dword eax, 16
	shr	dword edx, 16
	mov	word [ebx+416], ax
	mov	word [ebx+480], dx

	pop	dword ebx
	mov	dword esp, ebp
	pop	dword ebp
        ret


