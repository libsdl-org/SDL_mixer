/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 2026 Daniel K. O. <github.com/dkosmari>

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

/*
 * Comparison of channel orders:
 *
 * |  Num. | chan. | SDL | FLAC | MS/USB | Vorbis |
 * |------:|------:|-----|------|--------|--------|
 * |     2 |     1 | FL  | FL   | FL     | FL     |
 * |       |     2 | FR  | FR   | FR     | FR     |
 * |-------|-------|-----|------|--------|--------|
 * |     3 |     1 | FL  | FL   | FL     | FL     |
 * |       |     2 | FR  | FR   | FR     | FC     |
 * |       |     3 | LFE | FC   | FC/LFE | FR     |
 * |-------|-------|-----|------|--------|--------|
 * |     4 |     1 | FL  | FL   | FL     | FL     |
 * |       |     2 | FR  | FR   | FR     | FR     |
 * |       |     3 | RL  | RL   | RL     | RL     |
 * |       |     4 | RR  | RR   | RR     | RR     |
 * |-------|-------|-----|------|--------|--------|
 * |     5 |     1 | FL  | FL   | FL     | FL     |
 * | (5.0) |     2 | FR  | FR   | FR     | FC     |
 * |       |     3 | LFE | FC   | FC/LFE | FR     |
 * |       |     4 | RL  | RL   | RL     | RL     |
 * |       |     5 | RR  | RR   | RR     | RR     |
 * |-------|-------|-----|------|--------|--------|
 * |     6 |     1 | FL  | FL   | FL     | FL     |
 * | (5.1) |     2 | FR  | FR   | FR     | FC     |
 * |       |     3 | FC  | FC   | FC     | FR     |
 * |       |     4 | LFE | LFE  | LFE    | RL     |
 * |       |     5 | RL  | RL   | RR     | RR     |
 * |       |     6 | RR  | RR   | RR     | LFE    |
 * |-------|-------|-----|------|--------|--------|
 * |     7 |     1 | FL  | FL   | FL     | FL     |
 * |       |     2 | FR  | FR   | FR     | FC     |
 * |       |     3 | FC  | FC   | FC     | FR     |
 * |       |     4 | LFE | LFE  | LFE    | SL     |
 * |       |     5 | RC  | RC   | RC     | SR     |
 * |       |     6 | SL  | SL   | SL     | RC     |
 * |       |     7 | SR  | SR   | SR     | LFE    |
 * |-------|-------|-----|------|--------|--------|
 * |     8 |     1 | FL  | FL   | FL     | FL     |
 * | (7.1) |     2 | FR  | FR   | FR     | FC     |
 * |       |     3 | FC  | FC   | FC     | FR     |
 * |       |     4 | LFE | LFE  | LFE    | SL     |
 * |       |     5 | RL  | RL   | RL     | SR     |
 * |       |     6 | RR  | RR   | RR     | RL     |
 * |       |     7 | SL  | SL   | SL     | RR     |
 * |       |     8 | SR  | SR   | SR     | LFE    |
 *
 *
 * **Note:** USB/MS use a bitmask to indicate which channels are present. The only
 * requirement is that they must appear in a fixed order, if present:
 *
 *   - FL (Front Center)
 *   - FR (Front Right)
 *   - FC (Front Center)
 *   - LFE (Low Frequency Enhancement)
 *   - BL (Back Left) aka RL (Rear LEft)
 *   - BR (Back Right) aka RR (Rear Right)
 *   - FLC (Front Left of Center)
 *   - FRC (Front Right of Center)
 *   - BC (Back Center) aka RC (Rear Center)
 *   - SL (Side Left)
 *   - SR (Side Right)
 *   - TC (Top Center)
 *   - TFL (Top Front Left)
 *   - TFC (Top Front Center)
 *   - TFR (Top Front Right)
 *   - TBL (Top Back Left)
 *   - TBC (Top Back Center)
 *   - TBR (Top Back Right)
 *
 *
 * **Note:** WavPack documentation claims that ALL MS/USB channels (up to the max) must be
 * present. So to contain all the 7.1 channels, a .wv file must have 11 channels, with
 * silent FLC, FRC, BC/RC channels.
 *
 *
 * Sources:
 *   - Vorbis: https://www.rfc-editor.org/rfc/rfc7845.html#section-5.1.1.2
 *   - SDL: https://github.com/libsdl-org/SDL/blob/main/include/SDL3/SDL_audio.h
 *   - FLAC: https://www.rfc-editor.org/rfc/rfc9639.html#name-channels-bits
 *   - WavPack (WV): https://www.wavpack.com/wavpack_doc.html
 *   - USB: https://www.usb.org/sites/default/files/audio10.pdf (see "3.7.2.3 Audio Channel Cluster Format")
 *   - MS: https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible
 */

#include "remap_channels.h"


static void remap_channels_vorbis_3_s16(Sint16 *samples, int num_samples)
{
    /* Note: this isn't perfect, because we map FC to LFE */
    int i;
    for (i = 0; i < num_samples; i += 3) {
        Sint16 FC = samples[i + 1];
        Sint16 FR = samples[i + 2];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
    }
}

static void remap_channels_vorbis_3_flt(float *samples, int num_samples)
{
    /* Note: this isn't perfect, because we map FC to LFE */
    int i;
    for (i = 0; i < num_samples; i += 3) {
        float FC = samples[i + 1];
        float FR = samples[i + 2];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
    }
}

static void remap_channels_vorbis_5_s16(Sint16 *samples, int num_samples)
{
    /* Note: this isn't perfect, because we map FC to LFE. */
    int i;
    for (i = 0; i < num_samples; i += 5) {
        Sint16 FC = samples[i + 1];
        Sint16 FR = samples[i + 2];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
    }
}

static void remap_channels_vorbis_5_flt(float *samples, int num_samples)
{
    /* Note: this isn't perfect, because we map FC to LFE. */
    int i;
    for (i = 0; i < num_samples; i += 5) {
        float FC = samples[i + 1];
        float FR = samples[i + 2];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
    }
}

static void remap_channels_vorbis_5_1_s16(Sint16 *samples, int num_samples)
{
    int i;
    for (i = 0; i < num_samples; i += 6) {
        Sint16 FC  = samples[i + 1];
        Sint16 FR  = samples[i + 2];
        Sint16 RL  = samples[i + 3];
        Sint16 RR  = samples[i + 4];
        Sint16 LFE = samples[i + 5];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
        samples[i + 3] = LFE;
        samples[i + 4] = RL;
        samples[i + 5] = RR;
    }
}

static void remap_channels_vorbis_5_1_flt(float *samples, int num_samples)
{
    int i;
    for (i = 0; i < num_samples; i += 6) {
        float FC  = samples[i + 1];
        float FR  = samples[i + 2];
        float RL  = samples[i + 3];
        float RR  = samples[i + 4];
        float LFE = samples[i + 5];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
        samples[i + 3] = LFE;
        samples[i + 4] = RL;
        samples[i + 5] = RR;
    }
}

static void remap_channels_vorbis_7_s16(Sint16 *samples, int num_samples)
{
    int i = 0;
    for (i = 0; i < num_samples; i += 7) {
        Sint16 FC  = samples[i + 1];
        Sint16 FR  = samples[i + 2];
        Sint16 SL  = samples[i + 3];
        Sint16 SR  = samples[i + 4];
        Sint16 RC  = samples[i + 5];
        Sint16 LFE = samples[i + 6];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
        samples[i + 3] = LFE;
        samples[i + 4] = RC;
        samples[i + 5] = SL;
        samples[i + 6] = SR;
    }
}

static void remap_channels_vorbis_7_flt(float *samples, int num_samples)
{
    int i = 0;
    for (i = 0; i < num_samples; i += 7) {
        float FC  = samples[i + 1];
        float FR  = samples[i + 2];
        float SL  = samples[i + 3];
        float SR  = samples[i + 4];
        float RC  = samples[i + 5];
        float LFE = samples[i + 6];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
        samples[i + 3] = LFE;
        samples[i + 4] = RC;
        samples[i + 5] = SL;
        samples[i + 6] = SR;
    }
}

static void remap_channels_vorbis_7_1_s16(Sint16 *samples, int num_samples)
{
    int i = 0;
    for (i = 0; i < num_samples; i += 8) {
        Sint16 FC  = samples[i + 1];
        Sint16 FR  = samples[i + 2];
        Sint16 SL  = samples[i + 3];
        Sint16 SR  = samples[i + 4];
        Sint16 RL  = samples[i + 5];
        Sint16 RR  = samples[i + 6];
        Sint16 LFE = samples[i + 7];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
        samples[i + 3] = LFE;
        samples[i + 4] = RL;
        samples[i + 5] = RR;
        samples[i + 6] = SL;
        samples[i + 7] = SR;
    }
}

static void remap_channels_vorbis_7_1_flt(float *samples, int num_samples)
{
    int i = 0;
    for (i = 0; i < num_samples; i += 8) {
        float FC  = samples[i + 1];
        float FR  = samples[i + 2];
        float SL  = samples[i + 3];
        float SR  = samples[i + 4];
        float RL  = samples[i + 5];
        float RR  = samples[i + 6];
        float LFE = samples[i + 7];
        samples[i + 1] = FR;
        samples[i + 2] = FC;
        samples[i + 3] = LFE;
        samples[i + 4] = RL;
        samples[i + 5] = RR;
        samples[i + 6] = SL;
        samples[i + 7] = SR;
    }
}

void remap_channels_vorbis_s16(Sint16 *samples, int num_samples, int num_channels)
{
    switch (num_channels) {
    case 3:
        remap_channels_vorbis_3_s16(samples, num_samples);
        break;
    case 5:
        remap_channels_vorbis_5_s16(samples, num_samples);
        break;
    case 6:
        remap_channels_vorbis_5_1_s16(samples, num_samples);
        break;
    case 7:
        remap_channels_vorbis_7_s16(samples, num_samples);
        break;
    case 8:
        remap_channels_vorbis_7_1_s16(samples, num_samples);
        break;
    }
}

void remap_channels_vorbis_flt(float *samples, int num_samples, int num_channels)
{
    switch (num_channels) {
    case 3:
        remap_channels_vorbis_3_flt(samples, num_samples);
        break;
    case 5:
        remap_channels_vorbis_5_flt(samples, num_samples);
        break;
    case 6:
        remap_channels_vorbis_5_1_flt(samples, num_samples);
        break;
    case 7:
        remap_channels_vorbis_7_flt(samples, num_samples);
        break;
    case 8:
        remap_channels_vorbis_7_1_flt(samples, num_samples);
        break;
    }
}
