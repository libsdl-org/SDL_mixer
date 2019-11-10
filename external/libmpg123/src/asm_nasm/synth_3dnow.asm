; 1 "synth_3dnow.S"
; 1 "<built-in>"
; 1 "<command line>"
; 1 "synth_3dnow.S"
; 34 "synth_3dnow.S"
; 1 "mangle.h" 1
; 13 "mangle.h"
; 1 "config.h" 1
; 14 "mangle.h" 2
; 1 "intsym.h" 1
; 15 "mangle.h" 2
; 35 "synth_3dnow.S" 2
; 53 "synth_3dnow.S"

%include "asm_nasm.inc"

_sym_mangle dct64_3dnow
_sym_mangle synth_1to1_3dnow_asm

EXTERN dct64_3dnow

%ifdef ACCURATE_ROUNDING
SECTION .data
 ALIGN 8
max_s16:
 dd    1191181824 ;  32767.0
 dd    1191181824
min_s16:
 dd    -956301312 ; -32768.0
 dd    -956301312
ftoi_magic:
 dd    1262485504 ;  2^23 + 2^22
 dd    1262485504
%endif

SECTION .text
 ALIGN 16

GLOBAL synth_1to1_3dnow_asm
synth_1to1_3dnow_asm:
 sub  esp,24
 push  ebp
 push  edi
 xor  ebp,ebp
 push  esi
 push  ebx
; 86 "synth_3dnow.S"
 mov  esi, [esp+52]
 mov  [esp+16],esi
 mov  ebx, [esp+48]
 mov  esi, [esp+60]
 mov  edx, [esi]

 femms
 test  ebx,ebx
 jne L26

 dec  edx
 and  edx,15
 mov  [esi],edx
 mov  ecx, [esp+56]
 jmp L27
L26:
 add  dword [esp+16],2
 mov  ecx, [esp+56]
 add  ecx,2176
L27:

 test  dl,1
 je L28
 mov  [esp+36],edx
 mov  ebx,ecx
 mov  esi, [esp+44]
 mov  edi,edx
 push  esi
 sal  edi,2
 mov  eax,ebx
 mov  [esp+24],edi
 add  eax,edi
 push  eax
 mov  eax,edx
 inc  eax
 and  eax,15
 lea  eax, [eax*4+1088]
 add  eax,ebx
 push  eax
 call dct64_3dnow
 add  esp,12
 jmp L29
L28:
 lea  esi, [edx+1]
 mov  edi, [esp+44]
 mov  [esp+36],esi
 lea  eax, [ecx+edx*4+1092]
 push  edi
 lea  ebx, [ecx+1088]
 push  eax
 sal  esi,2
 lea  eax, [ecx+edx*4]
 push  eax
 call dct64_3dnow
 add  esp,12
 mov  [esp+20],esi
L29:
 mov  edx, [esp+64]
 add  edx,64
 mov  ecx,16
 sub  edx, [esp+20]
 mov  edi, [esp+16]

 pcmpeqb mm7,mm7
 pslld mm7,31
 movq mm0,[edx]
 movq mm1,[ebx]
 ALIGN 32
L33:
 movq mm3,[edx+8]
 pfmul mm0,mm1
 movq mm4,[ebx+8]
 movq mm5,[edx+16]
 pfmul mm3,mm4
 movq mm6,[ebx+16]
 pfadd mm0,mm3
 movq mm1,[edx+24]
 pfmul mm5,mm6
 movq mm2,[ebx+24]
 pfadd mm0,mm5
 movq mm3,[edx+32]
 pfmul mm1,mm2
 movq mm4,[ebx+32]
 pfadd mm0,mm1
 movq mm5,[edx+40]
 pfmul mm3,mm4
 movq mm6,[ebx+40]
 pfadd mm0,mm3
 movq mm1,[edx+48]
 pfmul mm5,mm6
 movq mm2,[ebx+48]
 pfadd mm5,mm0
 movq mm3,[edx+56]
 pfmul mm2,mm1
 movq mm4,[ebx+56]
 pfadd mm2,mm5
 add  ebx,64
 sub  edx,-128
 movq mm0,[edx]
 pfmul mm3,mm4
 movq mm1,[ebx]
 pfadd mm2,mm3
 movq mm3,mm2
 psrlq mm3,32
 pfsub mm2,mm3
 inc  ebp
%ifdef ACCURATE_ROUNDING
 pfmin mm2,[max_s16]
 pfmax mm2,[min_s16]
 pfadd mm2,[ftoi_magic]
%else
 pf2id mm2,mm2
 packssdw mm2,mm2
%endif
 movd eax,mm2
 mov  [edi+0],ax
 add  edi,4
 dec  ecx
 jnz L33

 movd mm0,[ebx]
 movd mm1,[edx]
 punpckldq mm0,[ebx+8]
 punpckldq mm1,[edx+8]
 movd mm3,[ebx+16]
 movd mm4,[edx+16]
 pfmul mm0,mm1
 punpckldq mm3,[ebx+24]
 punpckldq mm4,[edx+24]
 movd mm5,[ebx+32]
 movd mm6,[edx+32]
 pfmul mm3,mm4
 punpckldq mm5,[ebx+40]
 punpckldq mm6,[edx+40]
 pfadd mm0,mm3
 movd mm1,[ebx+48]
 movd mm2,[edx+48]
 pfmul mm5,mm6
 punpckldq mm1,[ebx+56]
 punpckldq mm2,[edx+56]
 pfadd mm0,mm5
 pfmul mm1,mm2
 pfadd mm0,mm1
 pfacc mm0,mm1
%ifdef ACCURATE_ROUNDING
 pfmin mm0,[max_s16]
 pfmax mm0,[min_s16]
 pfadd mm0,[ftoi_magic]
%else
 pf2id mm0,mm0
 packssdw mm0,mm0
%endif
 movd eax,mm0
 mov  [edi+0],ax
 inc  ebp
 mov  esi, [esp+36]
 add  ebx,-64
 mov  ebp,15
 add  edi,4
 lea  edx, [edx+esi*8-128]

 mov  ecx,15
 movd mm0,[ebx]
 movd mm1,[edx-4]
 punpckldq mm0,[ebx+4]
 punpckldq mm1,[edx-8]
 ALIGN 32
L46:
 movd mm3,[ebx+8]
 movd mm4,[edx-12]
 pfmul mm0,mm1
 punpckldq mm3,[ebx+12]
 punpckldq mm4,[edx-16]
 movd mm5,[ebx+16]
 movd mm6,[edx-20]
 pfmul mm3,mm4
 punpckldq mm5,[ebx+20]
 punpckldq mm6,[edx-24]
 pfadd mm0,mm3
 movd mm1,[ebx+24]
 movd mm2,[edx-28]
 pfmul mm5,mm6
 punpckldq mm1,[ebx+28]
 punpckldq mm2,[edx-32]
 pfadd mm0,mm5
 movd mm3,[ebx+32]
 movd mm4,[edx-36]
 pfmul mm1,mm2
 punpckldq mm3,[ebx+36]
 punpckldq mm4,[edx-40]
 pfadd mm0,mm1
 movd mm5,[ebx+40]
 movd mm6,[edx-44]
 pfmul mm3,mm4
 punpckldq mm5,[ebx+44]
 punpckldq mm6,[edx-48]
 pfadd mm0,mm3
 movd mm1,[ebx+48]
 movd mm2,[edx-52]
 pfmul mm5,mm6
 punpckldq mm1,[ebx+52]
 punpckldq mm2,[edx-56]
 pfadd mm5,mm0
 movd mm3,[ebx+56]
 movd mm4,[edx-60]
 pfmul mm1,mm2
 punpckldq mm3,[ebx+60]
 punpckldq mm4,[edx]
 pfadd mm5,mm1
 add  edx,-128
 add  ebx,-64
 movd mm0,[ebx]
 movd mm1,[edx-4]
 pfmul mm3,mm4
 punpckldq mm0,[ebx+4]
 punpckldq mm1,[edx-8]
 pfadd mm3,mm5
 pfacc mm3,mm3
 inc  ebp
 pxor mm3,mm7
%ifdef ACCURATE_ROUNDING
 pfmin mm3,[max_s16]
 pfmax mm3,[min_s16]
 pfadd mm3,[ftoi_magic]
%else
 pf2id mm3,mm3
 packssdw mm3,mm3
%endif
 movd eax,mm3
 mov  [edi],ax
 add  edi,4
 dec  ecx
 jnz L46

 femms
 mov  eax,ebp
 pop  ebx
 pop  esi
 pop  edi
 pop  ebp
 add  esp,24
 ret
; 345 "synth_3dnow.S"

