; 1 "synth_mmx.S"
; 1 "<built-in>"
; 1 "<command line>"
; 1 "synth_mmx.S"
; 23 "synth_mmx.S"
; 1 "mangle.h" 1
; 13 "mangle.h"
; 1 "config.h" 1
; 14 "mangle.h" 2
; 1 "intsym.h" 1
; 15 "mangle.h" 2
; 24 "synth_mmx.S" 2

%include "asm_nasm.inc"

_sym_mangle dct64_MMX
_sym_mangle synth_1to1_MMX

EXTERN dct64_MMX

SECTION .text

GLOBAL synth_1to1_MMX
synth_1to1_MMX:
        push  ebp
        push  edi
        push  esi
        push  ebx

        mov  ecx, [esp+24]
        mov  edi, [esp+28]
        mov  ebx,15
        mov  edx, [esp+36]
        lea  edi, [edi+ecx*2]
        dec  ecx
        mov  esi, [esp+32]
        mov  eax, [edx]
        jecxz .l1
        dec  eax
        and  eax,ebx
        lea  esi, [esi+1088]
        mov  [edx],eax
.l1:
        lea  edx, [esi+eax*2]
        mov  ebp,eax
        inc  eax
        push  dword [esp+20]
        and  eax,ebx
        lea  ecx, [esi+eax*2+544]
        inc  ebx
        test  eax,1
        jnz short .l2
        xchg  ecx,edx
        inc  ebp
        lea  esi, [esi+544]
.l2:
        push  edx
        push  ecx
        call dct64_MMX
        add  esp,12

        lea  ecx, [ebx+1]
        sub  ebx,ebp
        push  eax
        mov  eax, [esp+44]
        lea  edx, [eax+ebx*2]
        pop  eax
.l3:
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
        loop .l3


        sub  esi,64
        mov  ecx,15
.l4:
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

        sub  esi,32
        add  edx,64
        lea  edi, [edi+4]
        loop .l4
        emms
        pop  ebx
        pop  esi
        pop  edi
        pop  ebp
        ret
; 141 "synth_mmx.S"


