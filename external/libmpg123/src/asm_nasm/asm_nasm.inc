%ifdef CCALIGN
%define MOVUAPS movaps
%else
%define MOVUAPS movups
%endif

%ifdef UNDERSCORE
%macro _sym_mangle 1
%define %1 _INT123_ %+ %1
%endmacro
%else
%macro _sym_mangle 1
%define %1  INT123_ %+ %1
%endmacro
%endif

BITS 32

%ifdef __OS2__
; declare segments with proper attributes for OS/2 builds
SEGMENT .data CLASS=DATA ALIGN=16 USE32 FLAT
SEGMENT .text CLASS=CODE ALIGN=16 USE32 FLAT
%endif

