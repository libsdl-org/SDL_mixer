; 1 "dct36_sse.S"
; 1 "<built-in>"
; 1 "<command line>"
; 1 "dct36_sse.S"
; 9 "dct36_sse.S"
; 1 "mangle.h" 1
; 13 "mangle.h"
; 1 "config.h" 1
; 14 "mangle.h" 2
; 1 "intsym.h" 1
; 15 "mangle.h" 2
; 10 "dct36_sse.S" 2
; 23 "dct36_sse.S"

%include "asm_nasm.inc"

_sym_mangle dct36_sse

 SECTION .data

 ALIGN 16
dct36_sse_COS9:
 dd    03f5db3d7h
 dd    03f5db3d7h
 dd    03f000000h
 dd    03f000000h
 dd    03f7c1c5ch
 dd    03f7c1c5ch
 dd    03f708fb2h
 dd    03f708fb2h
 dd    03f248dbbh
 dd    03f248dbbh
 dd    03e31d0d4h
 dd    03e31d0d4h
 dd    03eaf1d44h
 dd    03eaf1d44h
 dd    03f441b7dh
 dd    03f441b7dh

 ALIGN 16
dct36_sse_tfcos36:
 dd    03f007d2bh
 dd    03f0483eeh
 dd    03f0d3b7dh
 dd    03f1c4257h
 dd    040b79454h
 dd    03ff746eah
 dd    03f976fd9h
 dd    03f5f2944h
 dd    03f3504f3h
;filler:
 dd    000000000h
 dd    000000000h
 dd    000000000h

 ALIGN 16
dct36_sse_mask:
 dd    0,0ffffffffh,0,0ffffffffh

 ALIGN 16
dct36_sse_sign:
 dd    080000000h,080000000h,080000000h,080000000h

 SECTION .text
 ALIGN 16
GLOBAL dct36_sse
dct36_sse:
 push  ebp
 mov  ebp,esp
 and  esp,-16
 sub  esp,80
 push  ebx
 push  esi
 push  edi
 lea  esi, [esp+12]
 mov  edi, [ebp+8]

 lea  eax, [dct36_sse_COS9]
 lea  edx, [dct36_sse_tfcos36]

 xorps xmm0, xmm0
 xorps xmm5, xmm5
 movlps xmm5, [edi+64]
 movups xmm4, [edi+48]
 movups xmm3, [edi+32]
 movups xmm2, [edi+16]
 movups xmm1, [edi]
 movaps xmm6, xmm5
 shufps xmm6, xmm6, 0xe1
 movaps xmm7, xmm4
 shufps xmm7, xmm7, 0x93
 movss xmm6, xmm7
 addps xmm5, xmm6
 movaps xmm6, xmm3
 shufps xmm6, xmm6, 0x93
 movss xmm7, xmm6
 addps xmm4, xmm7
 movaps xmm7, xmm2
 shufps xmm7, xmm7, 0x93
 movss xmm6, xmm7
 addps xmm3, xmm6
 movaps xmm6, xmm1
 shufps xmm6, xmm6, 0x93
 movss xmm7, xmm6
 addps xmm2, xmm7
 movss xmm6, xmm0
 addps xmm1, xmm6

 movaps xmm0, [dct36_sse_mask]
 movaps xmm6, xmm4
 shufps xmm4, xmm5, 0x4e
 movaps xmm7, xmm3
 shufps xmm3, xmm6, 0x4e
 andps xmm6, xmm0
 addps xmm4, xmm6
 movaps xmm6, xmm2
 shufps xmm2, xmm7, 0x4e
 andps xmm7, xmm0
 addps xmm3, xmm7
 movaps xmm7, xmm1
 shufps xmm1, xmm6, 0x4e
 andps xmm6, xmm0
 addps xmm2, xmm6
 movaps xmm6, xmm7
 andps xmm7, xmm0
 xorps xmm0, xmm0
 addps xmm1, xmm7
 movlhps xmm0, xmm6
; 138 "dct36_sse.S"
 movaps xmm5, xmm2
 shufps xmm5, xmm3, 0xe4
 shufps xmm3, xmm4, 0xe4
 shufps xmm4, xmm2, 0xe4
 movaps xmm2, xmm5

 mulps xmm5, [eax]
 addps xmm5, xmm0

 movaps [esi], xmm0
 movaps [esi+16], xmm2

 movaps xmm6, xmm1
 subps xmm6, xmm3
 subps xmm6, xmm4
 xorps xmm7, xmm7
 shufps xmm7, xmm2, 0xe0
 mulps xmm6, [eax]
 subps xmm0, xmm7
 addps xmm6, xmm0
 movaps [esi+48], xmm6

 movaps xmm2, [eax+16]

 movaps xmm0, xmm1
 movaps xmm6, xmm3
 movaps xmm7, xmm4
 mulps xmm0, xmm2
 mulps xmm6, [eax+32]
 mulps xmm7, [eax+48]
 addps xmm0, xmm5
 addps xmm6, xmm7
 addps xmm0, xmm6
 movaps [esi+32], xmm0

 movaps xmm0, xmm1
 movaps xmm6, xmm3
 movaps xmm7, xmm4
 mulps xmm0, [eax+32]
 mulps xmm6, [eax+48]
 mulps xmm7, xmm2
 subps xmm0, xmm5
 subps xmm7, xmm6
 addps xmm0, xmm7
 movaps [esi+64], xmm0

 movaps xmm6, xmm1
 movaps xmm7, xmm4
 mulps xmm6, [eax+48]
 mulps xmm2, xmm3
 mulps xmm7, [eax+32]
 subps xmm6, xmm5
 subps xmm2, xmm7
 addps xmm6, xmm2

 movaps xmm0, [esi]
 movss xmm5, [edx+32]
 subps xmm0, xmm1
 subps xmm4, [esi+16]
 addps xmm0, xmm3
 addps xmm0, xmm4
 shufps xmm0, xmm0, 0xaf
 mulss xmm0, xmm5
 movaps [esi], xmm0

 movaps xmm0, [esi+32]
 movaps xmm1, [esi+48]
 movaps xmm2, [esi+64]
; 225 "dct36_sse.S"
 movaps xmm3, xmm0
 unpcklps xmm0, xmm1
 unpckhps xmm3, xmm1
 movaps xmm5, xmm2
 unpcklps xmm2, xmm6
 unpckhps xmm5, xmm6
 xorps xmm5, [dct36_sse_sign]
; 240 "dct36_sse.S"
 movaps xmm1, xmm0
 movlhps xmm0, xmm2
 movhlps xmm2, xmm1
 movaps xmm4, xmm3
 movlhps xmm3, xmm5
 movhlps xmm5, xmm4
; 254 "dct36_sse.S"
 movaps xmm6, [edx]
 movaps xmm7, [edx+16]
 movaps xmm1, xmm5
 addps xmm5, xmm2
 subps xmm1, xmm2
 movaps xmm2, xmm3
 addps xmm3, xmm0
 subps xmm2, xmm0
 mulps xmm5, xmm6
 mulps xmm7, xmm1

 movaps [esi+16], xmm2
; 275 "dct36_sse.S"
 mov  edi, [ebp+12]
 mov  edx, [ebp+16]
 mov  ecx, [ebp+20]
 mov  eax, [ebp+24]

 movaps xmm0, xmm3
 movaps xmm1, xmm5
 movups xmm2, [ecx+108]
 movups xmm3, [ecx+92]
 shufps xmm3, xmm3, 0x1b
 movups xmm4, [ecx+36]
 movups xmm5, [ecx+20]
 shufps xmm5, xmm5, 0x1b
 movaps xmm6, xmm0
 addps xmm0, xmm1
 subps xmm6, xmm1
 mulps xmm2, xmm0
 mulps xmm0, xmm3
 mulps xmm4, xmm6
 mulps xmm6, xmm5
 movups xmm1, [edi+36]
 movups xmm3, [edi+20]
 shufps xmm6, xmm6, 0x1b
 addps xmm1, xmm4
 addps xmm3, xmm6
 shufps xmm0, xmm0, 0x1b
 movups [edx+36], xmm2
 movups [edx+20], xmm0
 movss [eax+32*36], xmm1
 movss [eax+32*20], xmm3
 movhlps xmm2, xmm1
 movhlps xmm4, xmm3
 movss [eax+32*44], xmm2
 movss [eax+32*28], xmm4
 shufps xmm1, xmm1, 0xb1
 shufps xmm3, xmm3, 0xb1
 movss [eax+32*40], xmm1
 movss [eax+32*24], xmm3
 movhlps xmm2, xmm1
 movhlps xmm4, xmm3
 movss [eax+32*48], xmm2
 movss [eax+32*32], xmm4

 movss xmm0, [esi+8]
 movss xmm1, [esi]
 movss xmm2, [ecx+124]
 movss xmm3, [ecx+88]
 movss xmm4, [ecx+52]
 movss xmm5, [ecx+16]
 movss xmm6, xmm0
 addss xmm0, xmm1
 subss xmm6, xmm1
 mulss xmm2, xmm0
 mulss xmm0, xmm3
 mulss xmm4, xmm6
 mulss xmm6, xmm5
 addss xmm4, [edi+52]
 addss xmm6, [edi+16]
 movss [edx+52], xmm2
 movss [edx+16], xmm0
 movss [eax+32*52], xmm4
 movss [eax+32*16], xmm6

 movaps xmm0, [esi+16]
 movaps xmm1, xmm7
 MOVUAPS xmm2, [ecx+128]
 movups xmm3, [ecx+72]
 shufps xmm2, xmm2, 0x1b
 movlps xmm4, [ecx+56]
 movhps xmm4, [ecx+64]
 MOVUAPS xmm5, [ecx]
 shufps xmm4, xmm4, 0x1b
 movaps xmm6, xmm0
 addps xmm0, xmm1
 subps xmm6, xmm1
 mulps xmm2, xmm0
 mulps xmm0, xmm3
 mulps xmm4, xmm6
 mulps xmm6, xmm5
 movlps xmm1, [edi+56]
 movhps xmm1, [edi+64]
 movups xmm3, [edi]
 shufps xmm4, xmm4, 0x1b
 addps xmm3, xmm6
 addps xmm1, xmm4
 shufps xmm2, xmm2, 0x1b
 movups [edx], xmm0
 movlps [edx+56], xmm2
 movhps [edx+64], xmm2
 movss [eax+32*56], xmm1
 movss [eax], xmm3
 movhlps xmm2, xmm1
 movhlps xmm4, xmm3
 movss [eax+32*64], xmm2
 movss [eax+32*8], xmm4
 shufps xmm1, xmm1, 0xb1
 shufps xmm3, xmm3, 0xb1
 movss [eax+32*60], xmm1
 movss [eax+32*4], xmm3
 movhlps xmm2, xmm1
 movhlps xmm4, xmm3
 movss [eax+32*68], xmm2
 movss [eax+32*12], xmm4

 pop  edi
 pop  esi
 pop  ebx
 mov  esp,ebp
 pop  ebp

 ret
