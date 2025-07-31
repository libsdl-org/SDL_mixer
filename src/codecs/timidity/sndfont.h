/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.
*/

#ifndef TIMIDITY_SNDFONT_H /* sndfont.h: soundfont loader public api */
#define TIMIDITY_SNDFONT_H

void init_soundfont(MidiSong *song, const char *fname, int order);
void end_soundfont(void);
Instrument *load_soundfont(MidiSong *song, int order, int bank, int preset, int keynote);
void exclude_soundfont(int bank, int preset, int keynote);
void order_soundfont(int bank, int preset, int keynote, int order);

#endif /* TIMIDITY_SNDFONT_H */
