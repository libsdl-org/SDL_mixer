#ifndef umxload_h
#define umxload_h

extern int process_upkg(FILE *f, int *offset, int *objsize);
extern const char *mustype[];

#define UMUSIC_IT   0
#define UMUSIC_S3M  1
#define UMUSIC_XM   2
#define UMUSIC_MOD  3
#define UMUSIC_WAV  4
#define UMUSIC_MP2  5

#endif /* umxload_h */
