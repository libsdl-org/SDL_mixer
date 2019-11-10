;  1 "synth_sse.S"
;  1 "<built-in>"
;  1 "<command line>"
;  1 "synth_sse.S"
;  1 "mangle.h" 1
;  13 "mangle.h"
;  1 "config.h" 1
;  14 "mangle.h" 2
;  1 "intsym.h" 1
;  15 "mangle.h" 2
;  2 "synth_sse.S" 2
;  1 "synth_sse3d.h" 1
;  33 "synth_sse3d.h"

%include "asm_nasm.inc"

_sym_mangle dct64_sse

; NOTE: intsym.h doesn't prefix synth_1to1_sse_asm with INT123_
%ifdef UNDERSCORE
%define synth_1to1_sse_asm _synth_1to1_sse_asm
%endif

EXTERN dct64_sse

SECTION .data
align 8
one_null:
 dd -65536
 dd -65536

align 8
null_one:
 dd 65535
 dd 65535

SECTION .text
align 16

GLOBAL synth_1to1_sse_asm
synth_1to1_sse_asm:
	push	dword ebp

	mov	dword ebp, esp

	sub	dword esp, 4
	push	dword edi
	push	dword esi
	push	dword ebx


	mov	dword ecx, [ebp+12]
	mov	dword edi, [ebp+16]
	mov	dword ebx, 15
	mov	dword edx, [ebp+24]
	lea	dword edi, [edi+ecx*2]
	dec	dword ecx
	mov	dword esi, [ebp+20]
	mov	dword eax, [edx]
	jecxz	_L01
	dec	dword eax
	and	dword eax, ebx
	lea	dword esi, [esi+1088]
	mov	dword [edx], eax
_L01:
	lea	dword edx, [esi+eax*2]
	mov	dword [esp+12], eax
	inc	dword eax
	and	dword eax, ebx
	lea	dword ecx, [esi+eax*2+544]
	inc	dword ebx
	test	dword eax, 1
	jnz	_L02
	xchg	dword ecx, edx
	inc	dword [esp+12]
	lea	dword esi, [esi+544]
_L02:
	push	dword [ebp+8]
	push	dword edx
	push	dword ecx
	call	dword dct64_sse
	add	dword esp, 12
	lea	dword ecx, [ebx+1]
	sub	dword ebx, [esp+12]
	push	dword ecx

	mov	dword ecx, [ebp+28]
	lea	dword edx, [ecx+ebx*2]
	mov	dword ecx, [esp]
	shr	dword ecx, 1
align 16
_L03:
	movq	mm0, [edx]
	movq	mm4, [edx+64]
	pmaddwd	mm0, [esi]
	pmaddwd	mm4, [esi+32]
	movq	mm1, [edx+8]
	movq	mm5, [edx+72]
	pmaddwd	mm1, [esi+8]
	pmaddwd	mm5, [esi+40]
	movq	mm2, [edx+16]
	movq	mm6, [edx+80]
	pmaddwd	mm2, [esi+16]
	pmaddwd	mm6, [esi+48]
	movq	mm3, [edx+24]
	movq	mm7, [edx+88]
	pmaddwd	mm3, [esi+24]
	pmaddwd	mm7, [esi+56]
	paddd	mm0, mm1
	paddd	mm4, mm5
	paddd	mm0, mm2
	paddd	mm4, mm6
	paddd	mm0, mm3
	paddd	mm4, mm7
	movq	mm1, mm0
	movq	mm5, mm4
	psrlq	mm1, 32
	psrlq	mm5, 32
	paddd	mm0, mm1
	paddd	mm4, mm5
	psrad	mm0, 13
	psrad	mm4, 13
	packssdw	mm0, mm0
	packssdw	mm4, mm4
	movq	mm1, [edi]
	punpckldq	mm0, mm4
	pand	mm1, [one_null]
	pand	mm0, [null_one]
	por	mm1, mm0
	movq	[edi], mm1
	lea	dword esi, [esi+64]
	lea	dword edx, [edx+128]
	lea	dword edi, [edi+8]
	dec	dword ecx
	jnz	_L03
	pop	dword ecx
	and	dword ecx, 1
	jecxz	_next_loop
	movq	mm0, [edx]
	pmaddwd	mm0, [esi]
	movq	mm1, [edx+8]
	pmaddwd	mm1, [esi+8]
	movq	mm2, [edx+16]
	pmaddwd	mm2, [esi+16]
	movq	mm3, [edx+24]
	pmaddwd	mm3, [esi+24]
	paddd	mm0, mm1
	paddd	mm0, mm2
	paddd	mm0, mm3
	movq	mm1, mm0
	psrlq	mm1, 32
	paddd	mm0, mm1
	psrad	mm0, 13
	packssdw	mm0, mm0
	movd	eax, mm0
	mov	word [edi], ax
	lea	dword esi, [esi+32]
	lea	dword edx, [edx+64]
	lea	dword edi, [edi+4]
_next_loop:
	sub	dword esi, 64
	mov	dword ecx, 7
align 16
_L04:
	movq	mm0, [edx]
	movq	mm4, [edx+64]
	pmaddwd	mm0, [esi]
	pmaddwd	mm4, [esi+-32]
	movq	mm1, [edx+8]
	movq	mm5, [edx+72]
	pmaddwd	mm1, [esi+8]
	pmaddwd	mm5, [esi+-24]
	movq	mm2, [edx+16]
	movq	mm6, [edx+80]
	pmaddwd	mm2, [esi+16]
	pmaddwd	mm6, [esi+-16]
	movq	mm3, [edx+24]
	movq	mm7, [edx+88]
	pmaddwd	mm3, [esi+24]
	pmaddwd	mm7, [esi+-8]
	paddd	mm0, mm1
	paddd	mm4, mm5
	paddd	mm0, mm2
	paddd	mm4, mm6
	paddd	mm0, mm3
	paddd	mm4, mm7
	movq	mm1, mm0
	movq	mm5, mm4
	psrlq	mm1, 32
	psrlq	mm5, 32
	paddd	mm1, mm0
	paddd	mm5, mm4
	psrad	mm1, 13
	psrad	mm5, 13
	packssdw	mm1, mm1
	packssdw	mm5, mm5
	psubd	mm0, mm0
	psubd	mm4, mm4
	psubsw	mm0, mm1
	psubsw	mm4, mm5
	movq	mm1, [edi]
	punpckldq	mm0, mm4
	pand	mm1, [one_null]
	pand	mm0, [null_one]
	por	mm1, mm0
	movq	[edi], mm1
	sub	dword esi, 64
	add	dword edx, 128
	lea	dword edi, [edi+8]
	dec	dword ecx
	jnz	_L04
	movq	mm0, [edx]
	pmaddwd	mm0, [esi]
	movq	mm1, [edx+8]
	pmaddwd	mm1, [esi+8]
	movq	mm2, [edx+16]
	pmaddwd	mm2, [esi+16]
	movq	mm3, [edx+24]
	pmaddwd	mm3, [esi+24]
	paddd	mm0, mm1
	paddd	mm0, mm2
	paddd	mm0, mm3
	movq	mm1, mm0
	psrlq	mm1, 32
	paddd	mm1, mm0
	psrad	mm1, 13
	packssdw	mm1, mm1
	psubd	mm0, mm0
	psubsw	mm0, mm1
	movd	eax, mm0
	mov	word [edi], ax
        emms


	pop	dword ebx
	pop	dword esi
	pop	dword edi
	mov	dword esp, ebp
	pop	dword ebp
        ret
;  5 "synth_sse.S" 2


