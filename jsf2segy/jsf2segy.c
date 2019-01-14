/*
 * TFO 16 April 2004 US Geologicl Survey 384 Woods Hole Road Woods Hole, MA
 * 02543
 * 
 * November 11 2005 Rolling in features to identify subbottom channels and to
 * unravel to new output file
 * 
 * September 20 2012 Add code to check for changes in record length. 
 *
 * October 24, 2018 Rearrange code inorder to accommodate 
 * subbottom record length changes. If detected, will close current
 * SEGY file and open a new file.
 *
 */

#define PMODE		0666	/* Read write permissons */
#define ZERO		0	/* End of file on read */
// #define Sonar_Data_Msg       80      /* JSF Message type = Sonar data could be
//                               * subbottom or sidescan */
#define Sidescan_Msg	82	/* JSF Message type = Side Scan Data Message */
#define Pitch_Roll_Msg	2020	/* JSF Message type = Pitch/Roll from MRU */
#define NMEA_Msg	2002	/* JSF Message type = NMEA string (time
				 * stamped NMEA string) */
#define Press_Msg	2060	/* JSF Message type = Pressure sensor reading */
#define Analog_Msg      2040	/* JSF Message type = Misc. Analog Sensor
				 * (ignore) */
#define Doppler_Msg	2080	/* JSF Message type = Doppler Velocity log */
#define Situation_Msg	2090	/* JSF Message type = SAS only systems */
#define SAS_Msg		86	/* JSF Message type = SAS processed data */

/*
 * Following are defined in 16 byte JSF header
 */

#define Start_Of_Message 0x1601	/* Marker for JSF 16 byte header */
// #define SubBottom    0       /* Subbottom data type identifier */
#define Low_Sidescan	20	/* Low frequency (75 or 120 KHZ) sidescan
				 * subsystem */
#define Hi_Sidescan	21	/* High Frequency (410 KHZ) sidescan
				 * subsystem */
#define	Port_SS		0	/* Port sidescan channel identifier */
#define Stbd_SS		1	/* Starboard sidescan channel identifier */
#define Env_Data	0	/* Subbottom envelope data (1 short data
				 * element) */
#define Ana_Data	1	/* Subbottom analytic data (2 short data
				 * elements Real, Imaginary) */
#define Raw_Data	2	/* Subbottom raw unmatched filter data (1
				 * short data element) */
#define Real_Data	3	/* Subbottom Real portion of analytic data (1
				 * short data element) */
#define Pix_Data	4	/* Subbottom Pixel data (1 short data
				 * element) */


#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "jsf2.h"
#include "ebcdic.h"
#include "segy_rev_1.h"

unsigned short Sonar_Data_Msg = 80;
unsigned short SubBottom = 0;
char *progname;

BCDHeader bhead, *bcdhead = &bhead;

/*
 * Let's get things aligned
 */

typedef union
{
  ShotHeader thead;
  float dummies[60];
} ForceFloat;

ForceFloat floatSegy;

int
main (int argc, char *argv[])
{				/* START MAIN */
  ShotHeader *THead;

  THead = &floatSegy.thead;

  progname = argv[0];

  if ((argc - optind) < 1)
    usage ();

  /*
   * Process arguments 8/6/2006  Added an option (-o) to write the output
   * file to the current directory - bwd
   */

  while ((c = getopt (argc, argv, "earxo:")) != -1)
    {
      switch (c)
	{
	case 'e':
	  do_Envelope++;
	  break;
	case 'a':
	  do_Analytic++;
	  break;
	case 'r':
	  do_Real++;
	  break;
	case 'x':
	  xt_Real++;
	  break;
	case 'o':
	  outputFile = (char *) optarg;
	  break;
	case '?':
	  err_exit ();
	  break;
	}
    }

  /*
   * open the input jsf file
   */

  if ((in_fd = open (argv[optind], O_RDONLY)) == -1)
    {
      fprintf (stderr, "%s: cannot open %s\n", argv[optind], progname);
      perror ("open");
      err_exit ();
    }

  /*
   * Get some memory for JSF header
   */

  JSFmsg = (unsigned char *) calloc (JSFmsgSize, sizeof (unsigned char));

  /*
   * Copy input file name to a temp buffer
   */

  strcpy (inputFileName, argv[optind]);

// copy output file name to prep for record length change

  strcpy (nextFileName, outputFile);
  strcat (outputFile, ".sgy");
  strcpy (outFileName, outputFile);

  /*
   * MAIN WORKING LOOP
   */

  while (1)
    {
      ProcessedPing = 0;	/* Setup lseek flag */

      inbytes = read (in_fd, JSFmsg, JSFmsgSize);

      if (inbytes == ZERO)
	{
	  fprintf (stdout,
		   "%s End of File reached %d seismic records processed\n",
		   inputFileName, SeismicRecords);
	  fprintf (stdout, "Start Time:\t%d:%d:%d:%d:%d\n", Year, Day, Hour,
		   Minute, Second);
	  fprintf (stdout, "End Time:\t%d:%d:%d:%d:%d\n",
		   get_short (JSFSEGYHead, 198), get_short (JSFSEGYHead, 196),
		   get_short (JSFSEGYHead, 186), get_short (JSFSEGYHead, 188),
		   get_short (JSFSEGYHead, 190));
	  exit (EXIT_SUCCESS);
	}

      if (inbytes != (int) JSFmsgSize)
	{
	  fprintf (stderr, "%s: error reading JSF message header\n",
		   progname);
	  perror ("read");
	  err_exit ();
	}

      if (get_short (JSFmsg, 0) != 0x1601)
	{
	  fprintf (stdout, "Invalid file format \n");
	  fprintf (stdout,
		   "%s Record Length change? %d seismic records processed\n",
		   inputFileName, SeismicRecords);
	  fprintf (stdout, "Start Time:\t%d:%d:%d:%d:%d\n", Year, Day, Hour,
		   Minute, Second);
	  fprintf (stdout, "End Time:\t%d:%d:%d:%d:%d\n",
		   get_short (JSFSEGYHead, 198), get_short (JSFSEGYHead, 196),
		   get_short (JSFSEGYHead, 186), get_short (JSFSEGYHead, 188),
		   get_short (JSFSEGYHead, 190));
	  err_exit ();
	}

      /*
       * Is it subbottom?
       */

      if (get_short (JSFmsg, 4) == Sonar_Data_Msg
	  && (int) JSFmsg[7] == SubBottom)
	{
	  if (!iFirst)
	    {
	      start_sb_size = get_int (JSFmsg, 12);
	      iFirst++;
	    }
	  current_sb_size = get_int (JSFmsg, 12);

	  if (current_sb_size != start_sb_size)
	    {
	      start_sb_size = current_sb_size;
	      do_start_new_file ();
	    }


	  /*
	   * Get the Edgetech "SEGY trace header"
	   */

	  SegBytes = read (in_fd, JSFSEGYHead, trhedlen);
	  if (SegBytes != (int) trhedlen)
	    {
	      fprintf (stderr,
		       "%s: error reading JSF trace header and data\n",
		       progname);
	      perror ("read");
	      err_exit ();
	    }

          /*
	   * Get the number of samples trace header
	   */

           numberOfSamples = get_short (JSFSEGYHead, 114);

          if (numberOfSamples  > 65535) {
		fprintf(stdout, "Number of samples exceeds SEGY standard\n");
	        close(outlu);
		err_exit();
          }	

	  /*
	   * Let's get the file start time first
	   */

	  if (!got_start_time)
	    {
	      Year = get_short (JSFSEGYHead, 198);
	      Day = get_short (JSFSEGYHead, 196);
	      Hour = get_short (JSFSEGYHead, 186);
	      Minute = get_short (JSFSEGYHead, 188);
	      Second = get_short (JSFSEGYHead, 190);
	      got_start_time++;
	    }

	  /*
	   * Get input data format
	   */

	  Data_Fmt = get_short (JSFSEGYHead, 34);

	  /*
	   * Lets first check if this is the data we want
	   */

	  if ((do_Envelope && Data_Fmt == Env_Data) ||
	      (do_Analytic && Data_Fmt == Ana_Data) ||
	      (do_Real && Data_Fmt == Real_Data) ||
	      (xt_Real && Data_Fmt == Ana_Data))
	    {


	      /*
	       * Check if first time through if yes setup the EBCDIC and
	       * BCD Header and send to disk file.
	       */

	      if (!doing_SB)
		{
		  if (!outlu)
		    {
		      if ((outlu =
			   open (outFileName, O_WRONLY | O_CREAT | O_EXCL,
				 PMODE)) == -1)
			{
			  fprintf (stderr, "%s: cannot open %s\n",
				   outFileName, progname);
			  perror ("open");
			  err_exit ();
			}
		    }		// End !outlu
	      sampInterval =
		(unsigned short) get_int (JSFSEGYHead, 116) / 1000;
	      sweepLength = (unsigned short) get_short (JSFSEGYHead, 130);

		  do_ebcdic ();
		  do_bcd ();
		  do_calloc ();

		  /*
		   * Write EBCDIC header to output file
		   */

		  if ((bytes_written =
		       write (outlu, ebcdic, EBCHDLEN)) != EBCHDLEN)
		    {
		      fprintf (stderr, "error writing EBCDIC header\n");
		      perror ("write");
		      err_exit ();
		    }

		  /*
		   * Write BCD header to output file
		   */

		  if (write (outlu, bcdhead, BCDHDLEN) != BCDHDLEN)
		    {
		      fprintf (stderr, "error writing BCD header \n");
		      perror ("write");
		      err_exit ();
		    }
		  doing_SB++;	/* Set flag that we only want to go through here once */
		}		/* END ! doing_SB */

	      /*
	       * Get trace Weighting factor
	       */

	      Weighting = get_short (JSFSEGYHead, 168);
	      Weighting = -Weighting;

	      /*
	       * Lets start processing Edgetech subbottom data
	       */

	      inbytes = read (in_fd, JSFData, DataSize);
	      if (inbytes != (int) DataSize)
		{
		  fprintf (stdout,
			   "%s: Error reading JSF seismic data\n", progname);
		  perror ("read");
		  err_exit ();
		}

	      /*
	       * Bump some Segy trace header entries
	       */

	      pingNum++;

	      /*
	       * If Analytic data Start normalizing the real and imaginary
	       * parts of the signal.
	       */

	      if (do_Analytic && Data_Fmt == Ana_Data)
		{
		  for (i = 0, j = 0; i < (int) DataSize; i += 4, j++)
		    {
		      floatSig[j] = (float)
			(sqrt ((double)
			       ldexp ((double) get_short (JSFData, i),
				      Weighting) *
			       (double) ldexp ((double)
					       get_short (JSFData, i),
					       Weighting)
			       + (double) ldexp ((double)
						 get_short (JSFData,
							    i + 2),
						 Weighting) *
			       (double) ldexp ((double)
					       get_short (JSFData, i + 2),
					       Weighting)));
		      if (LITTLE)
			floatSig[j] = floatFlip (&floatSig[j]);
		    }
		  nval = (size_t) numberOfSamples * (int) sizeof (float);
		}		// End do_Analytic

	      /*
	       * Check if Real data
	       */

	      if (do_Real && Data_Fmt == Real_Data)
		{
		  for (i = j = 0; j < (int) DataSize; i++, j += 2)
		    {
		      floatSig[i] =
			ldexpf ((float) get_short (JSFData, j), Weighting);
		      if (LITTLE)
			floatSig[i] = floatFlip (&floatSig[i]);
		    }
		  nval = (size_t) numberOfSamples * (int) sizeof (float);
		}		// End do_Real

	      /*
	       * Check if extracting Real from Analytic
	       */

	      if (xt_Real && Data_Fmt == Ana_Data)
		{
		  for (i = 0, j = 0; i < (int) DataSize; i += 4, j++)
		    {
		      floatSig[j] =
			ldexpf ((float) get_short (JSFData, i), Weighting);
		      if (LITTLE)
			floatSig[j] = floatFlip (&floatSig[j]);
		    }
		  nval = (size_t) numberOfSamples * (int) sizeof (float);
		}		// End xt_Real

	      /*
	       * Check if Envelope Data
	       */

	      if (do_Envelope && Data_Fmt == Env_Data)
		{
		  for (i = j = 0; j < (int) DataSize; i++, j += 2)
		    {
		      floatSig[i] =
			ldexpf ((float) get_short (JSFData, j), Weighting);
		      if (LITTLE)
			floatSig[i] = floatFlip (&floatSig[i]);
		    }
		  nval = (size_t) numberOfSamples * (int) sizeof (float);
		}		// End do_Envelope

	      /*
	       * OK, done seismic data conversion let's get the SEGY Trace
	       * Header setup
	       */

	      floatSegy.thead.tseq_line = swap_uint32 (tseq_line++);                    /* sequence number */
	      floatSegy.thead.tseq_reel = swap_uint32 (tseq_reel++);                    /* bump again */
	      floatSegy.thead.fldrec = swap_uint32 (pingNum);                           /* ping number */
	      floatSegy.thead.fldtr = swap_uint32 (1);                                  /* trace number */
	      floatSegy.thead.trcode = swap_uint16 (1);                                 /* Seismic data */
              floatSegy.thead.elev = swap_int32 (get_int (JSFSEGYHead, 136));           /* receiver pressure depth (mm)*/
              floatSegy.thead.selev = swap_int32 (get_int (JSFSEGYHead, 136));          /* source pressure depth (mm) */
              floatSegy.thead.swdepth = swap_int32 (get_int (JSFSEGYHead, 144));        /* water depth at source (mm)*/
              floatSegy.thead.rwdepth = swap_int32 (get_int (JSFSEGYHead, 144));        /* water depth at receiver (mm) */
	      floatSegy.thead.offset = swap_int32 (get_short (JSFSEGYHead, 38));	/* s - r offset */
	      floatSegy.thead.nttr = swap_uint16 (numberOfSamples);                     /* samples this trace */
	      floatSegy.thead.dt = bhead.mdt;                                           /* sampling interval */
	      floatSegy.thead.gaincon = swap_uint16 (get_short (JSFSEGYHead, 120));	/* gain constant */
	      floatSegy.thead.year = swap_uint16 (get_short (JSFSEGYHead, 198));	/* year of recording */
	      floatSegy.thead.julday = swap_uint16 (get_short (JSFSEGYHead, 196));	/* day of recording */
	      floatSegy.thead.hour = swap_uint16 (get_short (JSFSEGYHead, 186));	/* hour of recording */
	      floatSegy.thead.minute = swap_uint16 (get_short (JSFSEGYHead, 188));	/* minute of recording */
	      floatSegy.thead.second = swap_uint16 (get_short (JSFSEGYHead, 190));	/* second of recording */
	      floatSegy.thead.tbasis = swap_uint16 (4);                                 /* UTC time */
	      floatSegy.thead.map_scale = swap_int16 (-1000);
	      floatSegy.thead.xsc = floatSegy.thead.xrc = swap_int32 (get_int (JSFSEGYHead, 80));	/* Longitude */
	      floatSegy.thead.ysc = floatSegy.thead.yrc = swap_int32 (get_int (JSFSEGYHead, 84));	/* Latitude */
	      floatSegy.thead.map_unit = swap_uint16 (2);                               /* Lon, Lat */
	      floatSegy.thead.survey_scale = swap_int16 (-1000);                        /* depth values in * millimeters */
	      floatSegy.thead.correl = swap_uint16 (2);                                 /* Correlated */
	      floatSegy.thead.stfreq = swap_uint16 (get_short (JSFSEGYHead, 126) * 10);	/* Start Frequency of * Chirp */
	      floatSegy.thead.enfreq = swap_uint16 (get_short (JSFSEGYHead, 128) * 10);	/* End Frequency of * Chirp */
	      floatSegy.thead.swplen = swap_uint16 (get_short (JSFSEGYHead, 130));	/* Sweep length in * milliseconds */
	      floatSegy.thead.swptyp = swap_uint16 (1);	/* Linear Sweep */

	      /*
	       * Now send out the Trace header
	       */

	      if (write (outlu, THead, trhedlen) != (int) trhedlen)
		{
		  fprintf (stdout, "error writing trace header \n");
		  perror ("write");
		  err_exit ();
		}

	      /*
	       * Now send Seismic data to disk file
	       */

	      if (write (outlu, floatSig, nval) != (int) nval)
		{
		  fprintf (stdout, "Error writing SEGY trace\n");
		  perror ("write");
		  err_exit ();
		}
	      ++SeismicRecords;	/* Bump seismic record count */
	      ProcessedPing++;	/* Set flag for lseek below */
	    }			/* END IS SUBBOTTOM */

	  if (!ProcessedPing)
	    {
	      offset = (off_t) (get_int (JSFmsg, 12) - SegBytes);
	      where = lseek (in_fd, offset, SEEK_CUR);
	      ProcessedPing++;	/* Set flag for lseek below */
	    }
	}			/* End Analytic or Envelope data check */

      /*
       * Not subbottom so skip record Already have header just skip data
       */

      if (!ProcessedPing)
	{
	  offset = (off_t) get_int (JSFmsg, 12);
	  where = lseek (in_fd, offset, SEEK_CUR);
	}
    }				/* End while(1) Go back for more */
}				/* End main() */

void
err_exit (void)
{
  fprintf (stdout, "Err_exit \n");
  exit (EXIT_FAILURE);
}

void
usage (void)
{

  fprintf (stdout,
	   "jsf2segy ... extracts subbottom data from Edgetech JSF formatted files\n");
  fprintf (stdout, "\nUse lstjsf to find out input data format\n");
  fprintf (stdout,
	   "and to make sure sidescan record sizes and subbotton record sizes are consistant\n");
  fprintf (stdout,
	   "\nUsage:	jsf2segy - options first then full path to input file name\n");
  fprintf (stdout, "\nIE: jsf2segy -a -o outfile infile.jsf\n");
  fprintf (stdout, "\nOptions: \t-e Get Envelope subbottom data\n");
  fprintf (stdout, "\t\t-a Get Analytic subbottom data and make Envelope\n");
  fprintf (stdout, "\t\t-r Get Real subbottom data\n");
  fprintf (stdout,
	   "\t\t-x Extract real value from Analytic subbottom data\n");
  fprintf (stdout,
	   "\t\t-o Path and name of output file (use no file extension ie .sgy) \n\n");
  exit (EXIT_FAILURE);
}

float
floatSwap (char *value)
{
  char buffer[4];

  buffer[0] = value[3];
  buffer[1] = value[2];
  buffer[2] = value[1];
  buffer[3] = value[0];
  return *((float *) &buffer);
}

float
floatFlip (float *value)
{
  int i;
  unsigned char *ptr1, *ptr3;
  float *ptr2;
  float returnValue;

  ptr1 = (unsigned char *) value;
  ptr3 = (unsigned char *) calloc (1, sizeof (float));
  ptr1 += 3;
  for (i = 0; i < 4; i++)
    *ptr3++ = *ptr1--;
  ptr3 -= 4;
  ptr2 = (float *) ptr3;
  returnValue = *ptr2;
  free (ptr3);
  return (returnValue);
}

void
do_ebcdic (void)
{
  extern int asciiIndex, i;
  extern char ebcbuf[];
  extern char samps_per_shot[], segy[], tempBuffer[], temp[];

  /*
   * Get copy the EBCDIC template to an ASCII buffer that
   * can be manipulated before converting back to EBCDIC.
   */

  /*
   * memcpy(ebcbuf, ebcdicTemplate[0], 80); ebcbuf[80] =
   * '\0';
   */
  for (i = 0, asciiIndex = 80; i < 39; i++, asciiIndex += 80)
    {
      strcat (ebcbuf, ebcdicTemplate[i]);
      ebcbuf[asciiIndex] = '\0';
    }
  ebcbuf[3199] = '\0';

  /*
   * Now fill in the EBCDIC header with stuff that we need
   * C1 Client
   */

  strncpy (&ebcbuf[11], "U.S.G.S.", (size_t) 8);

  /*
   * C1 Company
   */

  strncpy (&ebcbuf[42], "WHSC", (size_t) 4);
  i = (int) strlen (outFileName);
  strncpy (&ebcbuf[89], outFileName, (size_t) i);

  /*
   * C4 Instrument Manufacturer
   */

  strncpy (&ebcbuf[257], "Edgetech", (size_t) 8);

  /*
   * C6 Bytes per sample
   */

  strncpy (&ebcbuf[475], "4", (size_t) 1);

  /*
   * C21
   */

  strncpy (&ebcbuf[1701], "Edgetech FSSB", (size_t) 20);

  /*
   * C22
   */

  strncpy (&ebcbuf[1764], "Altitude in Trace header byte 41 in millimeters",
	   (size_t) 47);

  /*
   * C6 Number of samples per shot
   */

  i = sprintf (samps_per_shot, "%d", numberOfSamples);
  strncpy (&ebcbuf[442], samps_per_shot, (size_t) i);

  /*
   * C6 Digitizer Sample Interval in microseconds Make
   * Edgetech nanoseconds into microseconds
   */

  i = (int) (get_int (JSFSEGYHead, 116) / 1000);
  sp_retn = sprintf (tempBuffer, "%d", i);
  if (strlen (tempBuffer) < (size_t) 7)
    strncpy (&ebcbuf[420], tempBuffer, (size_t) strlen (tempBuffer));
  else
    strncpy (&ebcbuf[420], tempBuffer, (size_t) 7);

  /*
   * C13 Chirp sweep length
   */

  i = sprintf (tempBuffer, "%d", get_short (JSFSEGYHead, 130));
  if (strlen (tempBuffer) < (size_t) 5)
    strncpy (&ebcbuf[1004], tempBuffer, (size_t) strlen (tempBuffer));
  else
    strncpy (&ebcbuf[1004], tempBuffer, (size_t) 4);

  /*
   * C13 Start Frequency
   */

  i = sprintf (tempBuffer, "%d", get_short (JSFSEGYHead, 126) * 10);
  strncpy (&ebcbuf[976], tempBuffer, (size_t) 4);

  /*
   * C13 End Frequency
   */

  i = sprintf (tempBuffer, "%d", get_short (JSFSEGYHead, 128) * 10);
  strncpy (&ebcbuf[988], tempBuffer, (size_t) 4);

  /*
   * C3 NMEA Year of recording
   */

  i = sprintf (tempBuffer, "%d", get_short (JSFSEGYHead, 198));
  strncpy (&ebcbuf[209], tempBuffer, (size_t) 4);

  /*
   * C3 NMEA Day start of reel
   */

  i = sprintf (tempBuffer, "%d", get_short (JSFSEGYHead, 196));
  strncpy (&ebcbuf[200], tempBuffer, (size_t) 2);

  /*
   * C4 Instrument Model
   */

  strncpy (&ebcbuf[277], "JStar", (size_t) 5);

  /*
   * C7 Recording format
   */

  strncpy (&ebcbuf[501], "1", (size_t) 1);

  /*
   * C8 Sample Code:
   */

  strncpy (&ebcbuf[577], "IEEE Floating Point", (size_t) 19);

  /*
   * C39 SEG Y REV_1
   */

  strncpy (&ebcbuf[3044], "SEG Y REV_1", (size_t) 11);

  /*
   * C38 Big Endian Byte Order
   */

  strncpy (&ebcbuf[2964], "Big Endian Byte Order", (size_t) 21);

  /*
   * End Textual Header
   */

  strncpy (&ebcbuf[3124], "C40 END TEXTUAL HEADER", (size_t) 22);

  /*
   * Done with Textual header.
   */

  /*
   * convert ascii to ebcdic
   */

  ascebc (ebcbuf, ebcdic, EBCHDLEN);
}				// End do_ebcdic()

void
do_bcd (void)
{
  /*
   * Now get the BCD Header sorted out
   */

  bhead.line = swap_uint32 (1);	/* line number 1 */
  bhead.reel = swap_uint32 (1);	/* reel number */
  bhead.ntr = swap_uint16 (1);	/* number of traces */
  bhead.mdt = swap_uint16 (sampInterval);	/* sample interval in * microsec */
  bhead.swlen = swap_uint16 (sweepLength);	/* Sweep length of Chirp * pulse */
  bhead.nt = swap_uint16 (numberOfSamples);	/* number of samples per * * channel */
  bhead.dform = swap_uint16 (5);	/* IEEE 4 byte floating * point */
  bhead.omdt = swap_uint16 (sampInterval);
  bhead.stfr = swap_uint16 (get_short (JSFSEGYHead, 126) * 10);	/* Start Frequency */
  bhead.enfr = swap_uint16 (get_short (JSFSEGYHead, 128) * 10);	/* End frequency */
  bhead.naux = swap_uint16 (0);	/* Number of Aux traces */
  bhead.sortcd = swap_uint16 (1);	/* Sort Code, As * recorded */
  bhead.unit = swap_uint16 (1);	/* Measurement system, 1 * = meters */
  bhead.Rev = swap_uint16 (0x100);	/* Segy Rev 1 */
  bhead.T_flag = swap_uint16 (1);	/* Fixed length trace * flag */
  bhead.N_extend = swap_uint16 (0);	/* No extend textual * headers */
}

void
do_calloc (void)
{

  free (JSFData);
  /*
   * Lets allocate some memory for JSF input trace data First
   * lets check if we are to work on Envelope or Analytic data.
   * Also check if we have already alloated storage for the
   * seismic data. We only need to do the allocation once.
   */


  if (do_Envelope && Data_Fmt == Env_Data)
    {
      DataSize = (size_t) (get_int (JSFmsg, 12) - 240);
      JSFData = (unsigned char *) calloc ((size_t) DataSize,
					  sizeof (unsigned char));
    }
  if (do_Analytic && Data_Fmt == Ana_Data)
    {
      DataSize = (size_t) (get_int (JSFmsg, 12) - 240);
      JSFData = (unsigned char *) calloc ((size_t) DataSize,
					  sizeof (unsigned char));
    }
  if (do_Real && Data_Fmt == Real_Data)
    {
      DataSize = (size_t) (get_int (JSFmsg, 12) - 240);
      JSFData = (unsigned char *) calloc ((size_t) DataSize,
					  sizeof (unsigned char));
    }
  if (xt_Real && Data_Fmt == Ana_Data)
    {
      DataSize = (size_t) (get_int (JSFmsg, 12) - 240);
      JSFData = (unsigned char *) calloc ((size_t) DataSize,
					  sizeof (unsigned char));
    }
  if (JSFData == NULL)
    {
      fprintf (stdout, "Error allocating JSFData storage\n");
      (void) fflush (stdout);
      perror ("reason");
      err_exit ();
    }
  done_Calloc++;
}

void
do_start_new_file (void)
{
  int byte_count = 0;
  iFirst = 0;			// reset flag
  doing_SB = 0;			// reset flag

  fprintf (stdout,
	   "Record length change detected. Closing output segy file %s \n",
	   outFileName);
  close (outlu);
  outlu = 0;

  memset (outFileName, 0, sizeof (outFileName));
  memcpy (outFileName, nextFileName, strlen (nextFileName));

  // fprintf (stdout, "outFileName = %s\n", outFileName);

  byte_count = strlen (outFileName);

  while (outlu == 0)
    {
      for (i = 0; i < 100; i++)
	{
	  outFileName[byte_count] = '0' + i / 10;
	  outFileName[byte_count + 1] = '0' + i % 10;
	  strcat (outFileName, segy);
//        fprintf (stdout, "byte_count = %d\n", byte_count);
	  outlu = open (outFileName, O_WRONLY | O_CREAT | O_EXCL, PMODE);
	  if (outlu > 0)
	    break;
	  memset (outFileName, 0, sizeof (outFileName));
	  memcpy (outFileName, nextFileName, strlen (nextFileName));
	  byte_count = strlen (outFileName);
// fprintf(stdout, "outlu = %d \n", outlu);
	}
    }
}


/*    get_double()
 *
 *    Subroutine to enable Intel based processors
 *    (little-endian) to read IEEE double precision
 *    floating point numbers (8 bytes) in files
 *    transferred from a mc680xx machine.
 *
 */

double
get_double (unsigned char *inbuf, int location)
{
  int i;
  unsigned char *ptr1, *ptr3;
  double *ptr2;
  double value;

  ptr1 = inbuf;
  ptr3 = (unsigned char *) calloc (1, sizeof (double));

  if (BIG)
    {
      ptr1 += location;
      for (i = 0; i < 8; i++)
	*ptr3++ = *ptr1++;
    }
  else
    {
      ptr1 += location;
      for (i = 0; i < 8; i++)
	*ptr3++ = *ptr1++;
    }

  ptr3 -= 8;

  ptr2 = (double *) ptr3;

  value = *ptr2;

  free (ptr3);

  return (value);
}

/*    get_float()
 *
 *    Subroutine to enable Intel based processors
 *    (little-endian) to read IEEE single precision
 *    floating point numbers (4 bytes) in files
 *    transferred from a mc680xx machine.
 */

float
get_float (unsigned char *inbuf, int location)
{
  int i;
  unsigned char *ptr1, *ptr3;
  float *ptr2;
  float value;

  ptr1 = inbuf;
  ptr3 = (unsigned char *) calloc (1, sizeof (float));

  if (BIG)
    {
      ptr1 += (location + 3);
      for (i = 0; i < 4; i++)
	*ptr3++ = *ptr1--;
    }
  else
    {
      ptr1 += location;
      for (i = 0; i < 4; i++)
	*ptr3++ = *ptr1++;
    }

  ptr3 -= 4;

  ptr2 = (float *) ptr3;

  value = *ptr2;

  free (ptr3);

  return (value);
}

/*    get_int()
 *
 *    Subroutine to enable Intel based processors

 *    (little-endian) to read IEEE integer
 *    numbers (4 bytes) in files
 *    transferred from a mc680xx machine and vice versa.
 */

int
get_int (unsigned char *buf, int location)
{
  int i;
  unsigned char *ptr1, *ptr3;
  int *ptr2;
  int value;

  ptr1 = buf;
  ptr3 = (unsigned char *) calloc (1, sizeof (int));

  if (BIG)
    {
      ptr1 += (location + 3);
      for (i = 0; i < 4; i++)
	*ptr3++ = *ptr1--;
    }
  else
    {
      ptr1 += location;
      for (i = 0; i < 4; i++)
	*ptr3++ = *ptr1++;
    }

  ptr3 -= 4;

  ptr2 = (int *) ptr3;

  value = *ptr2;

  free (ptr3);

  return (value);
}

/*    get_short()
 *
 *    Subroutine to enable Intel based processors to
 *    read short integer numbers (2 bytes)
 *    in files transferred from a mc680xx machine.
 *
 */

short
get_short (unsigned char *inbuf, int location)
{
  int i;
  unsigned char *ptr1, *ptr3;
  short value;
  short *ptr2;

  ptr1 = inbuf;
  ptr3 = (unsigned char *) calloc (1, sizeof (short));

  if (BIG)
    {
      ptr1 += (location + 1);
      for (i = 0; i < 2; i++)
	*ptr3++ = *ptr1--;
    }
  else
    {
      ptr1 += location;
      for (i = 0; i < 2; i++)
	*ptr3++ = *ptr1++;
    }

  ptr3 -= 2;

  ptr2 = (short *) ptr3;

  value = *ptr2;

  free (ptr3);

  return (value);
}

//! Byte swap unsigned short
uint16_t
swap_uint16 (uint16_t val)
{
  return (val << 8) | (val >> 8);
}

//! Byte swap short
int16_t
swap_int16 (int16_t val)
{
  return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
uint32_t
swap_uint32 (uint32_t val)
{
  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
  return (val << 16) | (val >> 16);
}

//! Byte swap int
int32_t
swap_int32 (int32_t val)
{
  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
  return (val << 16) | ((val >> 16) & 0xFFFF);
}

//! Byte swap 64 bit signed int
int64_t
swap_int64 (int64_t val)
{
  val =
    ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) &
					    0x00FF00FF00FF00FFULL);
  val =
    ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) &
					     0x0000FFFF0000FFFFULL);
  return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

//! Byte swap 64 bit unsigned int
uint64_t
swap_uint64 (uint64_t val)
{
  val =
    ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) &
					    0x00FF00FF00FF00FFULL);
  val =
    ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) &
					     0x0000FFFF0000FFFFULL);
  return (val << 32) | (val >> 32);
}
