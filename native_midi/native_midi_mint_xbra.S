/*
  native_midi:  Hardware Midi support on Atari for the SDL_mixer library
  Copyright (C) 2026  Miro Kropacek <miro.kropacek@gmail.com>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifdef __MINT__

#include "mint_asm_symbols.h"

/*
 * kb_midisys entry with an XBRA header: calls the C handler for the
 * transmit side, then jumps to the previous vector for the receive
 * side.
 *
 * No specification defines which registers a kbdvecs routine may
 * clobber, but both Atari TOS and EmuTOS save d0-d3/a0-a3 around these
 * calls. This handler clobbers only d0-d1/a0-a1 (C call) plus a0
 * (chain jump), so nothing has to be saved here under either reading.
 */

	.globl	SYM(midisys_oldvec)
	.globl	SYM(midisys_handler)

	.text

	.ascii	"XBRA"
	.ascii	"LSDM"
SYM(midisys_oldvec):
	.long	0
SYM(midisys_handler):
	jsr		SYM(midi_acia_hook)
	move.l	SYM(midisys_oldvec),a0
	jmp		(a0)

#endif
