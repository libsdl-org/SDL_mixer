/*
    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the Perl Artistic License, available in COPYING.

    common.h
*/

extern SDL_RWops *open_file(const char *name);
extern void add_to_pathlist(const char *s, size_t len);
extern void free_pathlist(void);
