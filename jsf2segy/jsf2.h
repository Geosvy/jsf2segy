#include <stdio.h>
#include <stdint.h>

#define Bit6 0x20
#define Bit9 0x200

#ifndef _JSF2_H_
#define _JSF2_H_
#endif

int  get_int (unsigned char *buf, int location);

short get_short (unsigned char *buf, int location);

void err_exit (void);
void ascebc (char *inbuf, char *outbuf, int nchars);
void *calloc (size_t count, size_t size);
void free (void *ptr);
void usage (void);
void do_ebcdic (void);
void do_bcd (void);
void do_calloc (void);
void do_start_new_file(void);

float floatFlip (float *value);
float floatSwap (char *value);

double get_double (unsigned char *inbuf, int location);
float get_float   (unsigned char *inbuf, int location);
int get_int       (unsigned char *buf, int location);
short get_short   (unsigned char *inbuf, int location);

//	Byte swapping routines

uint16_t swap_uint16 (uint16_t val);
int16_t  swap_int16  (int16_t val );
uint32_t swap_uint32 (uint32_t val );
int32_t  swap_int32  (int32_t val );
int64_t  swap_int64  (int64_t val );
uint64_t swap_uint64 (uint64_t val );


#if defined (mc68000) || defined (sony) || defined(sgi) || defined(sun) || defined (_BIG_ENDIAN)
#define BIG 1
#else
#define BIG 0
#endif

#if defined  (ultrix) || defined (__alpha) || defined (INTEL) ||  defined (__APPLE__)
#define LITTLE 1
#else
#define LITTLE 0
#endif


#define EBCHDLEN 3200    /* length of EBCDIC reel header block                   */
#define BCDHDLEN 400     /* length of BCD reel header block                      */
#define TRHDLEN 240      /* length of the binary header part of trace block      */

  int i = 0;
  int j = 0;
  int c = 0;
  int in_fd;                    /* file descriptor */
  int temp_i = 0;
  int outlu = 0;
  int bytes_written;
  int tseq_reel = 0;
  int tseq_line = 0;
  int asciiIndex;
  int doing_SB = 0;
  int do_Analytic = 0;
  int do_Envelope = 0;
  int do_Real = 0;
  int xt_Real = 0;
  int done_Calloc = 0;
  int sp_retn;
  int ProcessedPing = 0;
  int SeismicRecords = 0;
  int inbytes;
  int SegBytes = 0;
  int Year = 0;
  int Day = 0;
  int Hour = 0;
  int Second = 0;
  int Minute = 0;
  int got_start_time = 0;
  int start_sb_size = 0;
  int current_sb_size = 0;
  int iFirst = 0;
  int outOpened = 0;

  unsigned short sweepLength;
  unsigned short sampInterval;
  unsigned short VFlag = 0;
  unsigned short numberOfSamples = 0;

  unsigned int  pingNum = 0;

  size_t JSFmsgSize = 16;
  size_t trhedlen = 240;
  size_t DataSize = 0;
  size_t nval = 0;

  short Weighting;
  short Data_Fmt;

  off_t offset;                 /* offset for seeking starting record */
  off_t where;

  char samps_per_shot[10];
  char temp[255];
  char inputFileName[40] = "";
  char nextFileName[40] = "";
  char outFileName [40] = "";
  char ebcdic[3200];            /* ebcdic header */
  char ebcbuf[3200];
  char tempBuffer[21];
  char segy[] = ".sgy";
  char *outputFile;

  unsigned char *JSFData;
  unsigned char *JSFmsg;
  unsigned char JSFSEGYHead[TRHDLEN];

