/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

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

#ifdef MP3_MAD_MUSIC

#include <string.h>

#include "music_mad.h"

static int
MAD_RWread(mad_data *music, void *ptr, int size, int maxnum) {
    int remaining = music->length - music->pos;
    int ret;
    maxnum *= size;
    if (maxnum > remaining) maxnum = remaining;
    ret = SDL_RWread(music->rw, ptr, 1, maxnum);
    if (ret > 0) music->pos += ret;
    return ret;
}

static int
MAD_RWseek(mad_data *music, int offset, int whence) {
    int ret;
    switch (whence) { /* assumes a legal whence value */
    case RW_SEEK_CUR:
        offset += music->pos;
        break;
    case RW_SEEK_END:
        offset = music->length + offset;
        break;
    }
    if (offset < 0) return -1;
    if (offset > music->length)
        offset = music->length;
    ret = SDL_RWseek(music->rw, music->start + offset, RW_SEEK_SET);
    if (ret < 0) return ret;
    music->pos = offset;
    return (music->pos - music->start);
}

/* SDL-1.2 doesn't have a SDL_RWsize() */
static int SDL12_RWsize(SDL_RWops *rw) {
  int pos, size;
  if ((pos=SDL_RWtell(rw))<0) return -1;
  size = SDL_RWseek(rw, 0, RW_SEEK_END);
  SDL_RWseek(rw, pos, RW_SEEK_SET);
  return size;
}

static int skip_tags (mad_data *);

mad_data *
mad_openFileRW(SDL_RWops *rw, SDL_AudioSpec *mixer, int freerw)
{
  mad_data *mp3_mad;

  mp3_mad = (mad_data *)SDL_malloc(sizeof(mad_data));
  if (mp3_mad) {
	mp3_mad->rw = rw;
	mp3_mad->start = 0;
	mp3_mad->pos = 0;
	mp3_mad->length = SDL12_RWsize(rw);
	if (skip_tags(mp3_mad) < 0) {
	    SDL_free(mp3_mad);
	    Mix_SetError("music_mad: corrupt mp3 file.");
	    return NULL;
	}
	mp3_mad->freerw = freerw;
	mad_stream_init(&mp3_mad->stream);
	mad_frame_init(&mp3_mad->frame);
	mad_synth_init(&mp3_mad->synth);
	mp3_mad->frames_read = 0;
	mad_timer_reset(&mp3_mad->next_frame_start);
	mp3_mad->volume = MIX_MAX_VOLUME;
	mp3_mad->status = 0;
	mp3_mad->output_begin = 0;
	mp3_mad->output_end = 0;
	mp3_mad->mixer = *mixer;
  }
  return mp3_mad;
}

void
mad_closeFile(mad_data *mp3_mad)
{
  mad_stream_finish(&mp3_mad->stream);
  mad_frame_finish(&mp3_mad->frame);
  mad_synth_finish(&mp3_mad->synth);

  if (mp3_mad->freerw) {
	SDL_RWclose(mp3_mad->rw);
  }
  SDL_free(mp3_mad);
}

/* Starts the playback. */
void
mad_start(mad_data *mp3_mad) {
  mp3_mad->status |= MS_playing;
}

/* Stops the playback. */
void 
mad_stop(mad_data *mp3_mad) {
  mp3_mad->status &= ~MS_playing;
}

/* Returns true if the playing is engaged, false otherwise. */
int
mad_isPlaying(mad_data *mp3_mad) {
  return ((mp3_mad->status & MS_playing) != 0);
}


/*************************** TAG HANDLING: ******************************/

static __inline__ SDL_bool is_id3v1(const unsigned char *data, int length)
{
    /* http://id3.org/ID3v1 :  3 bytes "TAG" identifier and 125 bytes tag data */
    if (length < 3 || SDL_memcmp(data,"TAG",3) != 0) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}
static __inline__ SDL_bool is_id3v1ext(const unsigned char *data, int length)
{
    /* ID3v1 extended tag: just before ID3v1, always 227 bytes.
     * https://www.getid3.org/phpBB3/viewtopic.php?t=1202
     * https://en.wikipedia.org/wiki/ID3v1#Enhanced_tag
     * Not an official standard, is only supported by few programs. */
    if (length < 4 || SDL_memcmp(data,"TAG+",4) != 0) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}
static __inline__ SDL_bool is_id3v2(const unsigned char *data, int length)
{
    /* ID3v2 header is 10 bytes:  http://id3.org/id3v2.4.0-structure */
    /* bytes 0-2: "ID3" identifier */
    if (length < 10 || SDL_memcmp(data,"ID3",3) != 0) {
        return SDL_FALSE;
    }
    /* bytes 3-4: version num (major,revision), each byte always less than 0xff. */
    if (data[3] == 0xff || data[4] == 0xff) {
        return SDL_FALSE;
    }
    /* bytes 6-9 are the ID3v2 tag size: a 32 bit 'synchsafe' integer, i.e. the
     * highest bit 7 in each byte zeroed.  i.e.: 7 bit information in each byte ->
     * effectively a 28 bit value.  */
    if (data[6] >= 0x80 || data[7] >= 0x80 || data[8] >= 0x80 || data[9] >= 0x80) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}
static __inline__ int get_id3v2_len(const unsigned char *data, int length)
{
    /* size is a 'synchsafe' integer (see above) */
    int size = (int)((data[6]<<21) + (data[7]<<14) + (data[8]<<7) + data[9]);
    size += 10; /* header size */
    /* ID3v2 header[5] is flags (bits 4-7 only, 0-3 are zero).
     * bit 4 set: footer is present (a copy of the header but
     * with "3DI" as ident.)  */
    if (data[5] & 0x10) {
        size += 10; /* footer size */
    }
    /* optional padding (always zeroes) */
    while (size < length && data[size] == 0) {
        ++size;
    }
    return size;
}
static __inline__ SDL_bool is_apetag(const unsigned char *data, int length)
{
   /* http://wiki.hydrogenaud.io/index.php?title=APEv2_specification
    * Header/footer is 32 bytes: bytes 0-7 ident, bytes 8-11 version,
    * bytes 12-17 size. bytes 24-31 are reserved: must be all zeroes. */
    Uint32 v;

    if (length < 32 || SDL_memcmp(data,"APETAGEX",8) != 0) {
        return SDL_FALSE;
    }
    v = (data[11]<<24) | (data[10]<<16) | (data[9]<<8) | data[8]; /* version */
    if (v != 2000U && v != 1000U) {
        return SDL_FALSE;
    }
    v = 0; /* reserved bits : */
    if (SDL_memcmp(&data[24],&v,4) != 0 || SDL_memcmp(&data[28],&v,4) != 0) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}
static __inline__ int get_ape_len(const unsigned char *data)
{
    Uint32 flags, version;
    int size = (int)((data[15]<<24) | (data[14]<<16) | (data[13]<<8) | data[12]);
    version = (data[11]<<24) | (data[10]<<16) | (data[9]<<8) | data[8];
    flags = (data[23]<<24) | (data[22]<<16) | (data[21]<<8) | data[20];
    if (version == 2000U && (flags & (1U<<31))) size += 32; /* header present. */
    return size;
}

static int skip_tags(mad_data *music)
{
    int len, readsize;

    readsize = MAD_RWread(music, music->input_buffer, 1, MAD_INPUT_BUFFER_SIZE);
    if (readsize <= 0) return -1;

    /* ID3v2 tag is at the start */
    if (is_id3v2(music->input_buffer, readsize)) {
        len = get_id3v2_len(music->input_buffer, readsize);
        if (len >= music->length) return -1;
        music->start += len;
        music->length -= len;
        MAD_RWseek(music, 0, RW_SEEK_SET);
    }
    /* APE tag _might_ be at the start (discouraged
     * but not forbidden, either.)  read the header. */
    else if (is_apetag(music->input_buffer, readsize)) {
        len = get_ape_len(music->input_buffer);
        if (len >= music->length) return -1;
        music->start += len;
        music->length -= len;
        MAD_RWseek(music, 0, RW_SEEK_SET);
    }

    /* ID3v1 tag is at the end */
    if (music->length < 128) goto ape;
    MAD_RWseek(music, -128, RW_SEEK_END);
    readsize = MAD_RWread(music, music->input_buffer, 1, 128);
    MAD_RWseek(music, 0, RW_SEEK_SET);
    if (readsize != 128) return -1;
    if (is_id3v1(music->input_buffer, 128)) {
        music->length -= 128;

        /* extended ID3v1 just before the ID3v1 tag? (unlikely)
         * if found, assume no additional tags: this stupidity
         * is non-standard..  */
        if (music->length < 227) goto ape;
        MAD_RWseek(music, -227, RW_SEEK_END);
        readsize = MAD_RWread(music, music->input_buffer, 1, 227);
        MAD_RWseek(music, 0, RW_SEEK_SET);
        if (readsize != 227) return -1;
        if (is_id3v1ext(music->input_buffer, 227)) {
            music->length -= 227;
            goto end;
        }

        /* FIXME: handle possible double-ID3v1 tags? */
    }

    ape: /* APE tag may be at the end: read the footer */
    if (music->length >= 32) {
        MAD_RWseek(music, -32, RW_SEEK_END);
        readsize = MAD_RWread(music, music->input_buffer, 1, 32);
        MAD_RWseek(music, 0, RW_SEEK_SET);
        if (readsize != 32) return -1;
        if (is_apetag(music->input_buffer, 32)) {
            len = get_ape_len(music->input_buffer);
            if (len >= music->length) return -1;
            music->length -= len;
        }
    }

    end:
    return (music->length > 0)? 0: -1;
}

/* Reads the next frame from the file.  Returns true on success or
   false on failure. */
static int
read_next_frame(mad_data *mp3_mad) {
  if (mp3_mad->stream.buffer == NULL || 
	  mp3_mad->stream.error == MAD_ERROR_BUFLEN) {
	size_t read_size;
	size_t remaining;
	unsigned char *read_start;
	
	/* There might be some bytes in the buffer left over from last
	   time.  If so, move them down and read more bytes following
	   them. */
	if (mp3_mad->stream.next_frame != NULL) {
	  remaining = mp3_mad->stream.bufend - mp3_mad->stream.next_frame;
	  memmove(mp3_mad->input_buffer, mp3_mad->stream.next_frame, remaining);
	  read_start = mp3_mad->input_buffer + remaining;
	  read_size = MAD_INPUT_BUFFER_SIZE - remaining;

	} else {
	  read_size = MAD_INPUT_BUFFER_SIZE;
	  read_start = mp3_mad->input_buffer;
	  remaining = 0;
	}

	/* Now read additional bytes from the input file. */
	read_size = MAD_RWread(mp3_mad, read_start, 1, read_size);

	if (read_size <= 0) {
	  if ((mp3_mad->status & (MS_input_eof | MS_input_error)) == 0) {
		if (read_size == 0) {
		  mp3_mad->status |= MS_input_eof;
		} else {
		  mp3_mad->status |= MS_input_error;
		}

		/* At the end of the file, we must stuff MAD_BUFFER_GUARD
		   number of 0 bytes. */
		memset(read_start + read_size, 0, MAD_BUFFER_GUARD);
		read_size += MAD_BUFFER_GUARD;
	  }
	}

	/* Now feed those bytes into the libmad stream. */
	mad_stream_buffer(&mp3_mad->stream, mp3_mad->input_buffer,
					  read_size + remaining);
	mp3_mad->stream.error = MAD_ERROR_NONE;
  }

  /* Now ask libmad to extract a frame from the data we just put in
	 its buffer. */
  if (mad_frame_decode(&mp3_mad->frame, &mp3_mad->stream)) {
	if (MAD_RECOVERABLE(mp3_mad->stream.error)) {
	  mad_stream_sync(&mp3_mad->stream); /* to frame seek mode */
	  return 0;

	} else if (mp3_mad->stream.error == MAD_ERROR_BUFLEN) {
	  return 0;

	} else {
	  mp3_mad->status |= MS_decode_error;
	  return 0;
	}
  }

  mp3_mad->frames_read++;
  mad_timer_add(&mp3_mad->next_frame_start, mp3_mad->frame.header.duration);

  return 1;
}

/* Scale a MAD sample to 16 bits for output. */
static signed int
scale(mad_fixed_t sample) {
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/* Once the frame has been read, copies its samples into the output
   buffer. */
static void
decode_frame(mad_data *mp3_mad) {
  struct mad_pcm *pcm;
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;
  unsigned char *out;

  mad_synth_frame(&mp3_mad->synth, &mp3_mad->frame);
  pcm = &mp3_mad->synth.pcm;
  out = mp3_mad->output_buffer + mp3_mad->output_end;

  if ((mp3_mad->status & MS_cvt_decoded) == 0) {
	mp3_mad->status |= MS_cvt_decoded;

	/* The first frame determines some key properties of the stream.
	   In particular, it tells us enough to set up the convert
	   structure now. */
	SDL_BuildAudioCVT(&mp3_mad->cvt, AUDIO_S16, pcm->channels, mp3_mad->frame.header.samplerate, mp3_mad->mixer.format, mp3_mad->mixer.channels, mp3_mad->mixer.freq);
  }

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  while (nsamples--) {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */

    sample = scale(*left_ch++);
    *out++ = ((sample >> 0) & 0xff);
    *out++ = ((sample >> 8) & 0xff);

    if (nchannels == 2) {
      sample = scale(*right_ch++);
      *out++ = ((sample >> 0) & 0xff);
      *out++ = ((sample >> 8) & 0xff);
    }
  }

  mp3_mad->output_end = out - mp3_mad->output_buffer;
  /*assert(mp3_mad->output_end <= MAD_OUTPUT_BUFFER_SIZE);*/
}

int
mad_getSamples(mad_data *mp3_mad, Uint8 *stream, int len) {
  int bytes_remaining;
  int num_bytes;
  Uint8 *out;

  if ((mp3_mad->status & MS_playing) == 0) {
	/* We're not supposed to be playing, so send silence instead. */
	memset(stream, 0, len);
	return 0;
  }

  out = stream;
  bytes_remaining = len;
  while (bytes_remaining > 0) {
	if (mp3_mad->output_end == mp3_mad->output_begin) {
	  /* We need to get a new frame. */
	  mp3_mad->output_begin = 0;
	  mp3_mad->output_end = 0;
	  if (!read_next_frame(mp3_mad)) {
		if ((mp3_mad->status & MS_error_flags) != 0) {
		  /* Couldn't read a frame; either an error condition or
			 end-of-file.  Stop. */
		  memset(out, 0, bytes_remaining);
		  mp3_mad->status &= ~MS_playing;
		  return bytes_remaining;
		}
	  } else {
		decode_frame(mp3_mad);

		/* Now convert the frame data to the appropriate format for
		   output. */
		mp3_mad->cvt.buf = mp3_mad->output_buffer;
		mp3_mad->cvt.len = mp3_mad->output_end;
		
		mp3_mad->output_end = (int)(mp3_mad->output_end * mp3_mad->cvt.len_ratio);
		/*assert(mp3_mad->output_end <= MAD_OUTPUT_BUFFER_SIZE);*/
		SDL_ConvertAudio(&mp3_mad->cvt);
	  }
	}

	num_bytes = mp3_mad->output_end - mp3_mad->output_begin;
	if (bytes_remaining < num_bytes) {
	  num_bytes = bytes_remaining;
	}

	if (mp3_mad->volume == MIX_MAX_VOLUME) {
	  memcpy(out, mp3_mad->output_buffer + mp3_mad->output_begin, num_bytes);
	} else {
	  SDL_MixAudio(out, mp3_mad->output_buffer + mp3_mad->output_begin,
				   num_bytes, mp3_mad->volume);
	}
	out += num_bytes;
	mp3_mad->output_begin += num_bytes;
	bytes_remaining -= num_bytes;
  }
  return 0;
}

void
mad_seek(mad_data *mp3_mad, double position) {
  mad_timer_t target;
  int int_part;

  int_part = (int)position;
  mad_timer_set(&target, int_part, 
				(int)((position - int_part) * 1000000), 1000000);

  if (mad_timer_compare(mp3_mad->next_frame_start, target) > 0) {
	/* In order to seek backwards in a VBR file, we have to rewind and
	   start again from the beginning.  This isn't necessary if the
	   file happens to be CBR, of course; in that case we could seek
	   directly to the frame we want.  But I leave that little
	   optimization for the future developer who discovers she really
	   needs it. */
	mp3_mad->frames_read = 0;
	mad_timer_reset(&mp3_mad->next_frame_start);
	mp3_mad->status &= ~MS_error_flags;
	mp3_mad->output_begin = 0;
	mp3_mad->output_end = 0;

	MAD_RWseek(mp3_mad, 0, RW_SEEK_SET);
  }

  /* Now we have to skip frames until we come to the right one.
	 Again, only truly necessary if the file is VBR. */
  while (mad_timer_compare(mp3_mad->next_frame_start, target) < 0) {
	if (!read_next_frame(mp3_mad)) {
	  if ((mp3_mad->status & MS_error_flags) != 0) {
		/* Couldn't read a frame; either an error condition or
		   end-of-file.  Stop. */
		mp3_mad->status &= ~MS_playing;
		return;
	  }
	}
  }

  /* Here we are, at the beginning of the frame that contains the
	 target time.  Ehh, I say that's close enough.  If we wanted to,
	 we could get more precise by decoding the frame now and counting
	 the appropriate number of samples out of it. */
}

void
mad_setVolume(mad_data *mp3_mad, int volume) {
  mp3_mad->volume = volume;
}

#endif  /* MP3_MAD_MUSIC */
