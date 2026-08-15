/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.
*/

#ifndef TIMIDITY_SNDFONT_H /* sndfont.h: soundfont loader public api */
#define TIMIDITY_SNDFONT_H

#define init_sbk          TIMI_NAMESPACE(init_sbk)
#define end_sbk           TIMI_NAMESPACE(end_sbk)
#define init_soundfont    TIMI_NAMESPACE(init_soundfont)
#define end_soundfont     TIMI_NAMESPACE(end_soundfont)
#define load_soundfont    TIMI_NAMESPACE(load_soundfont)
#define exclude_soundfont TIMI_NAMESPACE(exclude_soundfont)
#define order_soundfont   TIMI_NAMESPACE(order_soundfont)

int init_sbk(const char *fname);
void end_sbk(void);
int init_soundfont(MidiSong *song, int order);
void end_soundfont(void);
Instrument *load_soundfont(MidiSong *song, int order, int bank, int preset, int keynote);
int exclude_soundfont(int bank, int preset, int keynote);
int order_soundfont(int bank, int preset, int keynote, int order);

#endif /* TIMIDITY_SNDFONT_H */
