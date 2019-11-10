; 1 "tabinit_mmx.S"
; 1 "<built-in>"
; 1 "<command line>"
; 1 "tabinit_mmx.S"
; 11 "tabinit_mmx.S"
; 1 "mangle.h" 1
; 13 "mangle.h"
; 1 "config.h" 1
; 14 "mangle.h" 2
; 1 "intsym.h" 1
; 15 "mangle.h" 2
; 12 "tabinit_mmx.S" 2

%include "asm_nasm.inc"

_sym_mangle costab_mmxsse
_sym_mangle make_decode_tables_mmx_asm

SECTION .data
 ALIGN 32
GLOBAL costab_mmxsse
costab_mmxsse:
 dd    1056974725
 dd    1057056395
 dd    1057223771
 dd    1057485416
 dd    1057855544
 dd    1058356026
 dd    1059019886
 dd    1059897405
 dd    1061067246
 dd    1062657950
 dd    1064892987
 dd    1066774581
 dd    1069414683
 dd    1073984175
 dd    1079645762
 dd    1092815430
 dd    1057005197
 dd    1057342072
 dd    1058087743
 dd    1059427869
 dd    1061799040
 dd    1065862217
 dd    1071413542
 dd    1084439708
 dd    1057128951
 dd    1058664893
 dd    1063675095
 dd    1076102863
 dd    1057655764
 dd    1067924853
 dd    1060439283
 dd    0000000000 ;filler

 ALIGN 32
intwinbase:
 dw    0, -1, -1, -1, -1, -1, -1, -2
 dw    -2, -2, -2, -3, -3, -4, -4, -5
 dw    -5, -6, -7, -7, -8, -9, -10, -11
 dw    -13, -14, -16, -17, -19, -21, -24, -26
 dw    -29, -31, -35, -38, -41, -45, -49, -53
 dw    -58, -63, -68, -73, -79, -85, -91, -97
 dw    -104, -111, -117, -125, -132, -139, -147, -154
 dw    -161, -169, -176, -183, -190, -196, -202, -208
 dw    -213, -218, -222, -225, -227, -228, -228, -227
 dw    -224, -221, -215, -208, -200, -189, -177, -163
 dw    -146, -127, -106, -83, -57, -29, 2, 36
 dw    72, 111, 153, 197, 244, 294, 347, 401
 dw    459, 519, 581, 645, 711, 779, 848, 919
 dw    991, 1064, 1137, 1210, 1283, 1356, 1428, 1498
 dw    1567, 1634, 1698, 1759, 1817, 1870, 1919, 1962
 dw    2001, 2032, 2057, 2075, 2085, 2087, 2080, 2063
 dw    2037, 2000, 1952, 1893, 1822, 1739, 1644, 1535
 dw    1414, 1280, 1131, 970, 794, 605, 402, 185
 dw    -45, -288, -545, -814, -1095, -1388, -1692, -2006
 dw    -2330, -2663, -3004, -3351, -3705, -4063, -4425, -4788
 dw    -5153, -5517, -5879, -6237, -6589, -6935, -7271, -7597
 dw    -7910, -8209, -8491, -8755, -8998, -9219, -9416, -9585
 dw    -9727, -9838, -9916, -9959, -9966, -9935, -9863, -9750
 dw    -9592, -9389, -9139, -8840, -8492, -8092, -7640, -7134
 dw    -6574, -5959, -5288, -4561, -3776, -2935, -2037, -1082
 dw    -70, 998, 2122, 3300, 4533, 5818, 7154, 8540
 dw    9975, 11455, 12980, 14548, 16155, 17799, 19478, 21189
 dw    22929, 24694, 26482, 28289, 30112, 31947,-26209,-24360
 dw    -22511,-20664,-18824,-16994,-15179,-13383,-11610, -9863
 dw    -8147, -6466, -4822, -3222, -1667, -162, 1289, 2684
 dw    4019, 5290, 6494, 7629, 8692, 9679, 10590, 11420
 dw    12169, 12835, 13415, 13908, 14313, 14630, 14856, 14992
 dw    15038

intwindiv:
 dd    047800000h ; 65536.0

SECTION .text
 ALIGN 32

GLOBAL make_decode_tables_mmx_asm
make_decode_tables_mmx_asm:
 push  ebp
 mov  ebp,esp
 sub  esp,12
 push  edi
 push  esi
 push  ebx


 lea  edi, [intwinbase]
 mov  ecx, [intwindiv]
 mov  [ebp-4],edi
 lea  eax, [edi+444]
 mov  [ebp-8],eax
 mov  [ebp-12],ecx
; 115 "tabinit_mmx.S"
 xor  ecx,ecx
 xor  ebx,ebx
 mov  esi,32

 neg  dword [ebp+8]
 push  dword 2
L00:
 cmp  ecx,528
 jnc L02
 movsx  eax,word [edi]
 cmp  edi, [ebp-8]
 jc L01
 add  eax,60000
L01:
 push  eax
 fild  dword [esp]
 fdiv  dword [ebp-12]
 fimul  dword [ebp+8]

 mov  eax, [ebp+12]
 fst  dword [eax+ecx*4]
 fstp  dword [eax+ecx*4+64]
 pop  eax
L02:
 lea  edx, [esi-1]
 and  edx,ebx
 cmp  edx,31
 jnz L03
 add  ecx,-1023
 test  ebx,esi
 jz L03
 neg  dword [ebp+8]
L03:
 add  ecx,esi
 add  edi, [esp]
 inc  ebx
 cmp  edi, [ebp-4]
 jz L04
 cmp  ebx,256
 jnz L00
 neg  dword [esp]
 jmp L00
L04:
 pop  eax

 xor  ecx,ecx
 xor  ebx,ebx
 push  dword 2
L05:
 cmp  ecx,528
 jnc L11
 movsx  eax,word [edi]
 cmp  edi, [ebp-8]
 jc L06
 add  eax,60000
L06:
 cdq
 imul  dword [ebp+8]
 shrd  eax,edx,17
 cmp  eax,32767
 mov  edx,1055
 jle L07
 mov  eax,32767
 jmp L08
L07:
 cmp  eax,-32767
 jge L08
 mov  eax,-32767
L08:

 push  ebx
 mov  ebx, [ebp+16]
 cmp  ecx,512
 jnc L09
 sub  edx,ecx
 mov  [ebx+edx*2],ax
 mov  [ebx+edx*2-32],ax
L09:
 test  ecx,1
 jnz L10
 neg  eax
L10:
 mov  [ebx+ecx*2],ax
 mov  [ebx+ecx*2+32],ax
 pop  ebx
L11:
 lea  edx, [esi-1]
 and  edx,ebx
 cmp  edx,31
 jnz L12
 add  ecx,-1023
 test  ebx,esi
 jz L12
 neg  dword [ebp+8]
L12:
 add  ecx,esi
 add  edi, [esp]
 inc  ebx
 cmp  edi, [ebp-4]
 jz L13
 cmp  ebx,256
 jnz L05
 neg  dword [esp]
 jmp L05
L13:
 pop  eax

 pop  ebx
 pop  esi
 pop  edi
 mov  esp,ebp
 pop  ebp
 ret

