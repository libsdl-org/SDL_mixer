;  1 "dct64_sse_float.S"
;  1 "<built-in>"
;  1 "<command line>"
;  1 "dct64_sse_float.S"
;  9 "dct64_sse_float.S"
;  1 "mangle.h" 1
;  13 "mangle.h"
;  1 "config.h" 1
;  14 "mangle.h" 2
;  1 "intsym.h" 1
;  15 "mangle.h" 2
;  10 "dct64_sse_float.S" 2
;  22 "dct64_sse_float.S"

%include "asm_nasm.inc"

_sym_mangle costab_mmxsse
_sym_mangle dct64_real_sse

SECTION .data

EXTERN costab_mmxsse

align 4
pnpn:
 dd 0
 dd -2147483648
 dd 0
 dd -2147483648

align 4
mask:
 dd -1
 dd -1
 dd -1
 dd 0

SECTION .text
align 4
GLOBAL dct64_real_sse
dct64_real_sse:
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

	lea	eax, [costab_mmxsse]
	mulps	xmm7, [eax]
	mulps	xmm6, [eax+16]
	mulps	xmm5, [eax+32]
	mulps	xmm4, [eax+48]

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
	mulps	xmm0, [eax+64]
	mulps	xmm1, [eax+80]
	mulps	xmm6, [eax+80]
	mulps	xmm7, [eax+64]

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
	mulps	xmm2, [eax+96]
	mulps	xmm0, [eax+96]
	mulps	xmm5, [eax+96]
	mulps	xmm7, [eax+96]
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
	movaps	xmm0, [eax+112]
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

	movss	xmm5, [eax+120]
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
	lea	eax, [mask]
	andps	xmm0, [eax]
	andps	xmm2, [eax]
	andps	xmm4, [eax]
	andps	xmm6, [eax]
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
	andps	xmm0, [eax]
	andps	xmm4, [eax]
	addps	xmm2, xmm3
	addps	xmm3, xmm0
	addps	xmm6, xmm7
	addps	xmm7, xmm4

	movaps	xmm0, [esp+(4+0)]
	movaps	xmm4, [esp+(4+64)]

	mov	ecx, [ebp+(8+0*4)]
	mov	ebx, [ebp+(8+1*4)]

	movss	[ecx+1024], xmm0
	movss	[ecx+896], xmm2
	movss	[ecx+768], xmm1
	movss	[ecx+640], xmm3

	shufps	xmm0, xmm0, 0xe1
	shufps	xmm2, xmm2, 0xe1
	shufps	xmm1, xmm1, 0xe1
	shufps	xmm3, xmm3, 0xe1
	movss	[ecx], xmm0
	movss	[ebx], xmm0
	movss	[ebx+128], xmm2
	movss	[ebx+256], xmm1
	movss	[ebx+384], xmm3

	movhlps	xmm0, xmm0
	movhlps	xmm2, xmm2
	movhlps	xmm1, xmm1
	movhlps	xmm3, xmm3
	movss	[ecx+512], xmm0
	movss	[ecx+384], xmm2
	movss	[ecx+256], xmm1
	movss	[ecx+128], xmm3

	shufps	xmm0, xmm0, 0xe1
	shufps	xmm2, xmm2, 0xe1
	shufps	xmm1, xmm1, 0xe1
	shufps	xmm3, xmm3, 0xe1
	movss	[ebx+512], xmm0
	movss	[ebx+640], xmm2
	movss	[ebx+768], xmm1
	movss	[ebx+896], xmm3

	movaps	xmm0, xmm4
	shufps	xmm0, xmm0, 0x1e
	movaps	xmm1, xmm5
	andps	xmm0, [eax]

	addps	xmm4, xmm6
	addps	xmm5, xmm7
	addps	xmm6, xmm1
	addps	xmm7, xmm0

	movss	[ecx+960], xmm4
	movss	[ecx+832], xmm6
	movss	[ecx+704], xmm5
	movss	[ecx+576], xmm7
	movhlps	xmm0, xmm4
	movhlps	xmm1, xmm6
	movhlps	xmm2, xmm5
	movhlps	xmm3, xmm7
	movss	[ecx+448], xmm0
	movss	[ecx+320], xmm1
	movss	[ecx+192], xmm2
	movss	[ecx+64], xmm3

	shufps	xmm4, xmm4, 0xe1
	shufps	xmm6, xmm6, 0xe1
	shufps	xmm5, xmm5, 0xe1
	shufps	xmm7, xmm7, 0xe1
	movss	[ebx+64], xmm4
	movss	[ebx+192], xmm6
	movss	[ebx+320], xmm5
	movss	[ebx+448], xmm7

	shufps	xmm0, xmm0, 0xe1
	shufps	xmm1, xmm1, 0xe1
	shufps	xmm2, xmm2, 0xe1
	shufps	xmm3, xmm3, 0xe1
	movss	[ebx+576], xmm0
	movss	[ebx+704], xmm1
	movss	[ebx+832], xmm2
	movss	[ebx+960], xmm3

	pop	dword ebx
	mov	dword esp, ebp
	pop	dword ebp
        ret


