/* Unreal UMX loader written by O. Sezer, put into public domain
 *
 * UPKG parsing partially based on Unreal Media Ripper (UMR) v0.3
 * by Andy Ward <wardwh@swbell.net>, with additional updates
 * by O. Sezer - see git repo at https://github.com/sezero/umr/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "umxload.h"

typedef int _is_int32[2 * (sizeof(int) == 4) - 1];/* make sure int is 32 bits. */

#define UPKG_HDR_TAG	0x9e2a83c1
#pragma pack(1)
struct upkg_hdr {
  unsigned int tag;   /* UPKG_HDR_TAG */
  int file_version;   /* 61 for original unreal */
  int pkg_flags;      /* bitflags - none needed */
  int name_count;     /* number of names in name table (>= 0) */
  int name_offset;    /* offset to name table  (>= 0) */
  int export_count;   /* num. exports in export table  (>= 0) */
  int export_offset;  /* offset to export table (>= 0) */
  int import_count;   /* num. imports in export table  (>= 0) */
  int import_offset;  /* offset to import table (>= 0) */

  /* number of GUIDs in heritage table (>= 1) and table's offset:
   * only with versions < 68. */
  int heritage_count;
  int heritage_offset;
#if 0
  /* with versions >= 68:  a GUID, a dword for generation count
   * and export_count and name_count dwords for each generation: */
  unsigned int guid[4];
  int generation_count;
  struct _genhist {
    int export_count;
    int name_count;
  } genhist[0/* generation_count */];
#endif
};
#pragma pack()

const char *mustype[] = {
  "IT", "S3M", "XM", "MOD",
  "WAV", "MP2", NULL
};

/* decode an FCompactIndex. original documentation by Tim Sweeney
 * was at http://unreal.epicgames.com/Packages.htm
 */
static int get_fci(const char *in, int *pos)
{
  int a;
  int size;

  size = 1;
  a = in[0] & 0x3f;

  if (in[0] & 0x40) {
    size++;
    a |= (in[1] & 0x7f) << 6;

    if (in[1] & 0x80) {
      size++;
      a |= (in[2] & 0x7f) << 13;

      if (in[2] & 0x80) {
        size++;
        a |= (in[3] & 0x7f) << 20;

        if (in[3] & 0x80) {
          size++;
          a |= (in[4] & 0x3f) << 27;
        }
      }
    }
  }

  if (in[0] & 0x80)
    a = -a;

  *pos += size;

  return a;
}

static int get_objtype(FILE *f, int ofs, int type)
{
  char sig[16];
_retry:
  memset(sig, 0, sizeof(sig));
  fseek(f, ofs, SEEK_SET);
  fread(sig, 16, 1, f);
  if (type == UMUSIC_IT) {
    if (memcmp(sig, "IMPM", 4) == 0)
      return UMUSIC_IT;
    return -1;
  }
  if (type == UMUSIC_XM) {
    if (memcmp(sig, "Extended Module:", 16) != 0)
      return -1;
    fread(sig, 16, 1, f);
    if (sig[0] != ' ') return -1;
    fread(sig, 16, 1, f);
    if (sig[5] != 0x1a) return -1;
    return UMUSIC_XM;
  }
  if (type == UMUSIC_MP2) {
    unsigned char *p = (unsigned char *)sig;
    unsigned short u = ((p[0] << 8) | p[1]) & 0xFFFE;
    if (u == 0xFFFC || u == 0xFFF4)
      return UMUSIC_MP2;
    return -1;
  }
  if (type == UMUSIC_WAV) {
    if (memcmp(sig, "RIFF", 4) == 0 && memcmp(&sig[8], "WAVE", 4) == 0)
      return UMUSIC_WAV;
    return -1;
  }

  fseek(f, ofs + 44, SEEK_SET);
  fread(sig, 4, 1, f);
  if (type == UMUSIC_S3M) {
    if (memcmp(sig, "SCRM", 4) == 0)
      return UMUSIC_S3M;
    /*return -1;*/
    /* SpaceMarines.umx and Starseek.umx from Return to NaPali
     * report as "s3m" whereas the actual music format is "it" */
    type = UMUSIC_IT;
    goto _retry;
  }

  fseek(f, ofs + 1080, SEEK_SET);
  fread(sig, 4, 1, f);
  if (type == UMUSIC_MOD) {
    if (memcmp(sig, "M.K.", 4) == 0 || memcmp(sig, "M!K!", 4) == 0)
      return UMUSIC_MOD;
    return -1;
  }

  return -1;
}

static int read_export(FILE *f, const struct upkg_hdr *hdr,
                       int *ofs, int *objsize)
{
  char buf[40];
  int idx = 0, t;

  fseek(f, *ofs, SEEK_SET);
  if (fread(buf, 4, 10, f) < 10)
    return -1;

  if (hdr->file_version < 40) idx += 8;  /* 00 00 00 00 00 00 00 00 */
  if (hdr->file_version < 60) idx += 16; /* 81 00 00 00 00 00 FF FF FF FF FF FF FF FF 00 00 */
  get_fci(&buf[idx], &idx);              /* skip junk */
  t = get_fci(&buf[idx], &idx);          /* type_name */
  if (hdr->file_version > 61) idx += 4;  /* skip export size */
  *objsize = get_fci(&buf[idx], &idx);
  *ofs += idx; /* offset for real data */

  return t;  /* return type_name index */
}

static int read_typname(FILE *f, const struct upkg_hdr *hdr,
                        int idx, char *out)
{
  int i, s;
  long l;
  char buf[64];

  if (idx >= hdr->name_count) return -1;
  buf[63] = '\0';
  for (i = 0, l = 0; i <= idx; i++) {
    fseek(f, hdr->name_offset + l, SEEK_SET);
    fread(buf, 1, 63, f);
    if (hdr->file_version >= 64) {
      s = *(signed char *)buf; /* numchars *including* terminator */
      if (s <= 0 || s > 64) return -1;
      l += s + 5;   /* 1 for buf[0], 4 for int32_t name_flags */
    } else {
      l += (long)strlen(buf);
      l +=  5;  /* 1 for terminator, 4 for int32_t name_flags */
    }
  }

  strcpy(out, (hdr->file_version >= 64)? &buf[1] : buf);
  return 0;
}

static void umx_strupr(char *str)
{
  while (*str) {
    if (*str >= 'a' && *str <= 'z') {
        *str -= ('a' - 'A');
    }
    str++;
  }
}

static int probe_umx(FILE *f, const struct upkg_hdr *hdr,
                     int *ofs, int *objsize)
{
  int i, idx, t;
  int s, pos;
  long fsiz;
  char buf[64];

  idx = 0;
  fseek(f, 0, SEEK_END);
  fsiz = ftell(f);

  /* Find the offset and size of the first IT, S3M or XM
   * by parsing the exports table. The umx files should
   * have only one export. Kran32.umx from Unreal has two,
   * but both pointing to the same music. */
  if (hdr->export_offset >= fsiz) return -1;
  memset(buf, 0, 64);
  fseek(f, hdr->export_offset, SEEK_SET);
  fread(buf, 1, 64, f);

  get_fci(&buf[idx], &idx);  /* skip class_index */
  get_fci(&buf[idx], &idx);  /* skip super_index */
  if (hdr->file_version >= 60) idx += 4; /* skip int32 package_index */
  get_fci(&buf[idx], &idx);  /* skip object_name */
  idx += 4;                  /* skip int32 object_flags */

  s = get_fci(&buf[idx], &idx); /* get serial_size */
  if (s <= 0) return -1;
  pos = get_fci(&buf[idx],&idx);/* get serial_offset */
  if (pos < 0 || pos > fsiz - 40) return -1;

  if ((t = read_export(f, hdr, &pos, &s)) < 0) return -1;
  if (s <= 0 || s > fsiz - pos) return -1;

  if (read_typname(f, hdr, t, buf) < 0) return -1;
  umx_strupr(buf);
  for (i = 0; mustype[i] != NULL; i++) {
    if (!strcmp(buf, mustype[i])) {
      t = i;
      break;
    }
  }
  if (mustype[i] == NULL) return -1;
  if ((t = get_objtype(f, pos, t)) < 0) return -1;

  *ofs = pos;
  *objsize = s;
  return t;
}

static int probe_header(void *header)
{
  struct upkg_hdr *hdr;
  unsigned char *p;
  unsigned int *swp;
  int i;

  /* byte swap the header - all members are 32 bit LE values */
  p = (unsigned char *) header;
  swp = (unsigned int *) header;
  for (i = 0; i < (int)sizeof(struct upkg_hdr)/4; i++, p += 4) {
    swp[i] = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
  }

  hdr = (struct upkg_hdr *) header;
  if (hdr->tag != UPKG_HDR_TAG) {
    fprintf(stderr, "Unknown header tag 0x%x\n", hdr->tag);
    return -1;
  }
  if (hdr->name_count     < 0 ||
      hdr->export_count   < 0 ||
      hdr->import_count   < 0 ||
      hdr->name_offset   < 36 ||
      hdr->export_offset < 36 ||
      hdr->import_offset < 36) {
    fprintf(stderr, "Illegal values in header\n");
    return -1;
  }

#if 1 /* no need being overzealous */
  return 0;
#else
  switch (hdr->file_version) {
  case 35: case 37:  /* Unreal beta - */
  case 40: case 41:           /* 1998 */
  case 61:/* Unreal */
  case 62:/* Unreal Tournament */
  case 63:/* Return to NaPali */
  case 64:/* Unreal Tournament */
  case 66:/* Unreal Tournament */
  case 68:/* Unreal Tournament */
  case 69:/* Tactical Ops */
  case 75:/* Harry Potter and the Philosopher's Stone */
  case 76:                      /* mpeg layer II data */
  case 83:/* Mobile Forces */
    return 0;
  }

  fprintf(stderr, "Unknown upkg version %d\n", hdr->file_version);
  return -1;
#endif /* #if 0  */
}

int process_upkg(FILE *f, int *ofs, int *objsize)
{
  char header[64];

  if (fread(header, 1, 64, f) < 64)
    return -1;
  if (probe_header(header) < 0)
    return -1;

  return probe_umx(f, (struct upkg_hdr *)header, ofs, objsize);
}
