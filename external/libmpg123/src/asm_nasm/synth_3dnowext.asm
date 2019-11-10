; 1 "synth_3dnowext.S"
; 1 "<built-in>"
; 1 "<command line>"
; 1 "synth_3dnowext.S"
; 1 "mangle.h" 1
; 13 "mangle.h"
; 1 "config.h" 1
; 14 "mangle.h" 2
; 1 "intsym.h" 1
; 15 "mangle.h" 2
; 2 "synth_3dnowext.S" 2
; 1 "synth_sse3d.h" 1
; 34 "synth_sse3d.h"

%include "asm_nasm.inc"

; NOTE: intsym.h doesn't prefix synth_1to1_3dnowext_asm with INT123_
%ifdef UNDERSCORE
%define synth_1to1_3dnowext_asm _synth_1to1_3dnowext_asm
%endif

_sym_mangle dct64_3dnowext

EXTERN dct64_3dnowext

SECTION .data
 ALIGN 8
one_null:
 dd    -65536
 dd    -65536

 ALIGN 8
null_one:
 dd    65535
 dd    65535

 SECTION .text
 ALIGN 16

GLOBAL synth_1to1_3dnowext_asm
synth_1to1_3dnowext_asm:
 push  ebp

 mov  ebp,esp





 sub  esp,4

 push  edi
 push  esi
 push  ebx
; 73 "synth_sse3d.h"
 mov  ecx, [ebp+12]
 mov  edi, [ebp+16]
 mov  ebx,15
 mov  edx, [ebp+24]
 lea  edi, [edi+ecx*2]
 dec  ecx
 mov  esi, [ebp+20]
 mov  eax, [edx]
 jecxz .l1
 dec  eax
 and  eax,ebx
 lea  esi, [esi+1088]
 mov  [edx],eax
.l1:
 lea  edx, [esi+eax*2]
 mov  [esp+12],eax
 inc  eax
 and  eax,ebx
 lea  ecx, [esi+eax*2+544]
 inc  ebx
 test  eax,1
 jnz short .l2
 xchg  ecx,edx
 inc  dword [esp+12]
 lea  esi, [esi+544]
.l2:
 push  dword [ebp+8]
 push  edx
 push  ecx
 call dct64_3dnowext
 add  esp,12
 lea  ecx, [ebx+1]
 sub  ebx, [esp+12]
 push  ecx

 mov  ecx, [ebp+28]
 lea  edx, [ecx+ebx*2]
 mov  ecx, [esp]
 shr  ecx,1



 ALIGN 16
.l3:
 movq mm0,[edx]
 movq mm4,[edx+64]
 pmaddwd mm0,[esi]
 pmaddwd mm4,[esi+32]
 movq mm1,[edx+8]
 movq mm5,[edx+72]
 pmaddwd mm1,[esi+8]
 pmaddwd mm5,[esi+40]
 movq mm2,[edx+16]
 movq mm6,[edx+80]
 pmaddwd mm2,[esi+16]
 pmaddwd mm6,[esi+48]
 movq mm3,[edx+24]
 movq mm7,[edx+88]
 pmaddwd mm3,[esi+24]
 pmaddwd mm7,[esi+56]
 paddd mm0,mm1
 paddd mm4,mm5
 paddd mm0,mm2
 paddd mm4,mm6
 paddd mm0,mm3
 paddd mm4,mm7
 movq mm1,mm0
 movq mm5,mm4
 psrlq mm1,32
 psrlq mm5,32
 paddd mm0,mm1
 paddd mm4,mm5
 psrad mm0,13
 psrad mm4,13
 packssdw mm0,mm0
 packssdw mm4,mm4
 movq mm1,[edi]
 punpckldq mm0,mm4
 pand mm1,[one_null]
 pand mm0,[null_one]
 por mm1,mm0
 movq [edi],mm1
 lea  esi, [esi+64]
 lea  edx, [edx+128]
 lea  edi, [edi+8]
 dec  ecx
 jnz .l3
 pop  ecx
 and  ecx,1
 jecxz .l4
 movq mm0,[edx]
 pmaddwd mm0,[esi]
 movq mm1,[edx+8]
 pmaddwd mm1,[esi+8]
 movq mm2,[edx+16]
 pmaddwd mm2,[esi+16]
 movq mm3,[edx+24]
 pmaddwd mm3,[esi+24]
 paddd mm0,mm1
 paddd mm0,mm2
 paddd mm0,mm3
 movq mm1,mm0
 psrlq mm1,32
 paddd mm0,mm1
 psrad mm0,13
 packssdw mm0,mm0
 movd eax,mm0
 mov  [edi],ax
 lea  esi, [esi+32]
 lea  edx, [edx+64]
 lea  edi, [edi+4]
.l4:
 sub  esi,64
 mov  ecx,7




 ALIGN 16
.l5:
 movq mm0,[edx]
 movq mm4,[edx+64]
 pmaddwd mm0,[esi]
 pmaddwd mm4,[esi-32]
 movq mm1,[edx+8]
 movq mm5,[edx+72]
 pmaddwd mm1,[esi+8]
 pmaddwd mm5,[esi-24]
 movq mm2,[edx+16]
 movq mm6,[edx+80]
 pmaddwd mm2,[esi+16]
 pmaddwd mm6,[esi-16]
 movq mm3,[edx+24]
 movq mm7,[edx+88]
 pmaddwd mm3,[esi+24]
 pmaddwd mm7,[esi-8]
 paddd mm0,mm1
 paddd mm4,mm5
 paddd mm0,mm2
 paddd mm4,mm6
 paddd mm0,mm3
 paddd mm4,mm7
 movq mm1,mm0
 movq mm5,mm4
 psrlq mm1,32
 psrlq mm5,32
 paddd mm1,mm0
 paddd mm5,mm4
 psrad mm1,13
 psrad mm5,13
 packssdw mm1,mm1
 packssdw mm5,mm5
 psubd mm0,mm0
 psubd mm4,mm4
 psubsw mm0,mm1
 psubsw mm4,mm5
 movq mm1,[edi]
 punpckldq mm0,mm4
 pand mm1,[one_null]
 pand mm0,[null_one]
 por mm1,mm0
 movq [edi],mm1
 sub  esi,64
 add  edx,128
 lea  edi, [edi+8]
 dec  ecx
 jnz .l5
 movq mm0,[edx]
 pmaddwd mm0,[esi]
 movq mm1,[edx+8]
 pmaddwd mm1,[esi+8]
 movq mm2,[edx+16]
 pmaddwd mm2,[esi+16]
 movq mm3,[edx+24]
 pmaddwd mm3,[esi+24]
 paddd mm0,mm1
 paddd mm0,mm2
 paddd mm0,mm3
 movq mm1,mm0
 psrlq mm1,32
 paddd mm1,mm0
 psrad mm1,13
 packssdw mm1,mm1
 psubd mm0,mm0
 psubsw mm0,mm1
 movd eax,mm0
 mov  [edi],ax
 emms


 pop  ebx
 pop  esi
 pop  edi
 mov  esp,ebp
 pop  ebp
 ret
; 5 "synth_3dnowext.S" 2
; 13 "synth_3dnowext.S"


