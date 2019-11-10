; 1 "getcpuflags.S"
; 1 "<built-in>"
; 1 "<command line>"
; 1 "getcpuflags.S"
; 18 "getcpuflags.S"
; 1 "mangle.h" 1
; 13 "mangle.h"
; 1 "config.h" 1
; 14 "mangle.h" 2
; 1 "intsym.h" 1
; 15 "mangle.h" 2
; 19 "getcpuflags.S" 2

%include "asm_nasm.inc"

_sym_mangle getcpuflags

SECTION .text
 ALIGN 4

GLOBAL getcpuflags
getcpuflags:
 push  ebp
 mov  ebp,esp
 push  edx
 push  ecx
 push  ebx
 push  esi

 mov  esi, [ebp+8]

 mov  eax,080000000h

 pushfd
 pushfd
 pop  eax
 mov  ebx,eax

 xor  eax,000200000h
 push  eax
 popfd

 pushfd
 pop  eax
 popfd
 cmp  eax,ebx
 je .Lnocpuid


 mov  dword [esi+12],00h
 mov  dword [esi+16],00h

 mov  eax,080000000h
 cpuid

 cmp  eax,080000001h
 jb .Lnoextended

 mov  eax,080000001h
 cpuid
 mov  [esi+12],edx
.Lnoextended:

 mov  eax,000000001h
 cpuid
 mov  [esi],eax
 mov  [esi+4],ecx
 mov  [esi+8],edx

 test  ecx,004000000h
 jz .Lend
 test  ecx,008000000h
 jz .Lend
 xor  ecx,ecx
 db    00fh,001h,0d0h
 mov  [esi+16],eax
 mov  eax, [esi]
 jmp .Lend
 ALIGN 4
.Lnocpuid:

 mov  eax,0
 mov  dword [esi],0
 mov  dword [esi+4],0
 mov  dword [esi+8],0
 mov  dword [esi+12],0
 mov  dword [esi+16],0
 ALIGN 4
.Lend:

 pop  esi
 pop  ebx
 pop  ecx
 pop  edx
 mov  esp,ebp
 pop  ebp
 ret



