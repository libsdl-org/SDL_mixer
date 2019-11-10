; 1 "synth_i586.S"
; 1 "<built-in>"
; 1 "<command line>"
; 1 "synth_i586.S"
; 33 "synth_i586.S"
; 1 "mangle.h" 1
; 13 "mangle.h"
; 1 "config.h" 1
; 14 "mangle.h" 2
; 1 "intsym.h" 1
; 15 "mangle.h" 2
; 34 "synth_i586.S" 2

%include "asm_nasm.inc"

_sym_mangle dct64_i386
_sym_mangle synth_1to1_i586_asm

EXTERN dct64_i386

SECTION .data

 ALIGN 8
_LC0:
 dd    00h,040dfffc0h
 ALIGN 8
_LC1:
 dd    00h,0c0e00000h
 ALIGN 8
SECTION .text

GLOBAL synth_1to1_i586_asm
synth_1to1_i586_asm:
 sub  esp,12
 push  ebp
 push  edi
 push  esi
 push  ebx

 mov  eax, [esp+32]
 mov  esi, [esp+40]
 mov  edi, [esp+48]
 mov  ebp, [edi]
 xor  edi,edi
 cmp  [esp+36],edi
 jne L48
 dec  ebp
 and  ebp,15
 mov  edi, [esp+48]
 mov  [edi],ebp
 xor  edi,edi
 mov  ecx, [esp+44]
 jmp L49
L48:
 add  esi,2
 mov  ecx, [esp+44]
 add  ecx,2176
L49:
 test  ebp,1
 je L50
 mov  ebx,ecx
 mov  [esp+16],ebp
 push  eax
 mov  edx, [esp+20]
 lea  eax, [ebx+edx*4]
 push  eax
 mov  eax, [esp+24]
 inc  eax
 and  eax,15
 lea  eax, [eax*4+1088]
 add  eax,ebx
 jmp L74
L50:
 lea  ebx, [ecx+1088]
 lea  edx, [ebp+1]
 mov  [esp+16],edx
 push  eax
 lea  eax, [ecx+ebp*4+1092]
 push  eax
 lea  eax, [ecx+ebp*4]
L74:
 push  eax
 call dct64_i386
 add  esp,12

 mov  edx, [esp+16]
 lea  edx, [edx*4+0]
 mov  eax, [esp+52]
 add  eax,64
 mov  ecx,eax
 sub  ecx,edx
 mov  ebp,16
L55:
 fld  dword [ecx]
 fmul  dword [ebx]
 fld  dword [ecx+4]
 fmul  dword [ebx+4]
 fxch st1
 fld  dword [ecx+8]
 fmul  dword [ebx+8]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx+12]
 fmul  dword [ebx+12]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+16]
 fmul  dword [ebx+16]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx+20]
 fmul  dword [ebx+20]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+24]
 fmul  dword [ebx+24]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx+28]
 fmul  dword [ebx+28]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+32]
 fmul  dword [ebx+32]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx+36]
 fmul  dword [ebx+36]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+40]
 fmul  dword [ebx+40]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx+44]
 fmul  dword [ebx+44]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+48]
 fmul  dword [ebx+48]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx+52]
 fmul  dword [ebx+52]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+56]
 fmul  dword [ebx+56]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx+60]
 fmul  dword [ebx+60]
 fxch st2
 sub  esp,4
 faddp st1,st0
 fxch st1
 fsubrp st1,st0
 fistp  dword [esp]
 pop  eax
 cmp  eax,32767
 jg short .l1
 cmp  eax,-32768
 jl short .l2
 mov  [esi],ax
 jmp short .l4
.l1: mov  word [esi],32767
 jmp short .l3
.l2: mov  word [esi],-32768
.l3: inc  edi
.l4:
L54:
 add  ebx,64
 sub  ecx,-128
 add  esi,4
 dec  ebp
 jnz L55
 fld  dword [ecx]
 fmul  dword [ebx]
 fld  dword [ecx+8]
 fmul  dword [ebx+8]
 fld  dword [ecx+16]
 fmul  dword [ebx+16]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+24]
 fmul  dword [ebx+24]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+32]
 fmul  dword [ebx+32]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+40]
 fmul  dword [ebx+40]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+48]
 fmul  dword [ebx+48]
 fxch st2
 faddp st1,st0
 fld  dword [ecx+56]
 fmul  dword [ebx+56]
 fxch st2
 sub  esp,4
 faddp st1,st0
 fxch st1
 faddp st1,st0
 fistp  dword [esp]
 pop  eax
 cmp  eax,32767
 jg short .l1
 cmp  eax,-32768
 jl short .l2
 mov  [esi],ax
 jmp short .l4
.l1: mov  word [esi],32767
 jmp short .l3
.l2: mov  word [esi],-32768
.l3: inc  edi
.l4:
L62:
 add  ebx,-64
 add  esi,4
 mov  edx, [esp+16]
 lea  ecx, [ecx+edx*8-128]
 mov  ebp,15
L68:
 fld  dword [ecx-4]
 fchs
 fmul  dword [ebx]
 fld  dword [ecx-8]
 fmul  dword [ebx+4]
 fxch st1
 fld  dword [ecx-12]
 fmul  dword [ebx+8]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-16]
 fmul  dword [ebx+12]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-20]
 fmul  dword [ebx+16]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-24]
 fmul  dword [ebx+20]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-28]
 fmul  dword [ebx+24]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-32]
 fmul  dword [ebx+28]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-36]
 fmul  dword [ebx+32]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-40]
 fmul  dword [ebx+36]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-44]
 fmul  dword [ebx+40]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-48]
 fmul  dword [ebx+44]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-52]
 fmul  dword [ebx+48]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-56]
 fmul  dword [ebx+52]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx-60]
 fmul  dword [ebx+56]
 fxch st2
 fsubrp st1,st0
 fld  dword [ecx]
 fmul  dword [ebx+60]
 fxch st2
 sub  esp,4
 fsubrp st1,st0
 fxch st1
 fsubrp st1,st0
 fistp  dword [esp]
 pop  eax
 cmp  eax,32767
 jg short .l1
 cmp  eax,-32768
 jl short .l2
 mov  [esi],ax
 jmp short .l4
.l1: mov  word [esi],32767
 jmp short .l3
.l2: mov  word [esi],-32768
.l3: inc  edi
.l4:
L67:
 add  ebx,-64
 add  ecx,-128
 add  esi,4
 dec  ebp
 jnz L68
 mov  eax,edi
 pop  ebx
 pop  esi
 pop  edi
 pop  ebp
 add  esp,12
 ret
; 343 "synth_i586.S"


