/*
 *    	This include file describing the SEG-Y
 *    	tape format was compliments of Rudy Voncolln
 *    	Department of Geology and Geophysics, Princeton
 *    	University, Princeton, N.J.
 *
 *    	t.f.o.
 *    	r.v. gyre
 *    	some where gulf of maine
 *    	27 oct 1985
 *
 * taken from RECOMMENDED STANDARDS FOR DIGITAL TAPE FORMATS by K.M.Barry,
   D.A.Cavers and C.W.Kneale in GEOPHYSICS 40/2 344-352 1975.
   The structure for the trace has been written with the binary data
   part as a pointer, so as to leave it open ended

   This file modified 25 March 2003 to reflect SEG Y rev 1 standard of May 2002
   TFO
*/

typedef struct
{				/* BCD coded section of the SEGY reel header                 */
  int jobid;			/* 1-4 Job identification number                             */
  int line;			/* 5-8 Line number                                           */
  int reel;			/* 9-12 Reel number                                          */
  short ntr;			/* 13-14. Traces per ensemble                                */
  short naux;			/* 15-16. # auxiliary traces per ensembel                    */
  short mdt;			/* 17-18. Sample interval, mimusec                           */
  short omdt;			/* 19-20.    Ditto,     original fld recording               */
  short nt;			/* 21-22. Samples per trace. This reel                       */
  short ont;			/* 23-24.    Ditto,  original fld recording                  */
  short dform;			/* 25-26. 
				   Sample format:  1= 4 byte IBM floating,
				   2= 4 byte int, 3= 2 byte int,
				   4= 4 byte fixed, with gain code, 5=4 byte IEEE FP 
				   6= NA 7= NA 8=1 byte two's compliment int                 */
  short cdpfold;		/* 27-28. Expected # of traces per ensemble                  */
  short sortcd;			/* 29-30.
				   Sorting code.  1= as recorded,
				   2= CDP ensamble, 3= single fold continuous,
				   4= horizontally stacked                                   */
  short sumcd;			/* 31-32. Vertical sum code. n=# summed.                     */
  short stfr;			/* 33-34. Starting sweep frequency                           */
  short enfr;			/* 35-36. Ending sweep frequency                             */
  short swlen;			/* 37-38. Sweep length, ms                                   */
  short swtyp;			/* 39-40. 
				   Sweep type. 1= linear. 2=parabolic
				   3=exponential 4=other                                     */
  short swtr;			/* 41-42. Trace # of sweep channel                           */
  short sttplen;		/* 43-44 starting sweep taper length (ms)                    */
  short entplen;		/* 45-46 ending sweep taper length (ms)                      */
  short tptyp;			/* 47-48 taper type: 1-linear,2-cos*cos,3-other              */
  short corr;			/* 49-50. Correlated? 1= no, 2=yes.                          */
  short gainr;			/* 51-52. Binary gain recovered? 1=yes,2=no                  */
  short armet;			/* 53-54. 
				   Amp. Recovery method. 1=none,
				   2=spher. div., 3= AGC, 4= other.                          */
  short unit;			/* 55-56 1=meters, 2=feet                                    */
  short polarity;		/* 57-58 Impulse signal polarity                             */
  short vib_polty;		/* 59-60 Vibrator polarity code. see refer.                  */
  char spare[240];		/* 61-300  Unassigned                                        */
  short Rev;			/* 301-302 SEG Y Format Revision Number                      */
  short T_flag;			/* 303-304 Fixed length trace flag                           */
  short N_extend;		/* 305-306 Number of 3200 byte extended headers              */
  char spare1[94];		/* 307-400 Unassigned                                        */

} BCDHeader;


typedef struct
{				/* trace data block */
  int tseq_line;		/* 1-4. Trace seq number this line.                          */
  int tseq_reel;		/* 5-8. Trace seq number this reel.                          */
  int fldrec;			/* 9-12. Original field record #                             */
  int fldtr;			/* 13-16. Trace # in original fldrec.                        */
  int sorcpt;			/* 17-20. Energy source point #.                             */
  int cdpno;			/* 21-24. CDP ensemble number.                               */
  int cdptr;			/* 25-28. Trace in CDP ensemble.                             */
  short trcode;			/* 29-30. 
				   1=seismic data. 2= dead. 3=dummy,
				   4=time break. 5=uphole. 6=sweep.
				   7=timing. 8= water break. 9->N=optional                   */
  short nvsum;			/* 31-32. order # of vertical sum                            */
  short nhsum;			/* 33-34. order # of horizontal sum                          */
  short prod;			/* 35-36. 1= production. 2= test                             */
  int offset;			/* 37-40. source to reciever offset                          */
  int elev;			/* 41-44. Receiver group elevation                           */
  int selev;			/* 45-48. Source elevation                                   */
  int sdepth;			/* 49-52. Shot depth (>0).                                   */
  int rdatum;			/* 53-56. Datum elevation at receiver                        */
  int sdatum;			/* 57-60. Datum elevation at source                          */
  int swdepth;			/* 61-64. Water depth at source                              */
  int rwdepth;			/* 65-68. Water depth at receiver                            */
  short survey_scale;		/* 69-70.
				   Scale factor to be applied to all
				   quantities in 41-68 to give true value.
				   = 1,+-10,+-100,+-1000,+-10000. += *, -= /                 */
  short map_scale;		/* 71-72.  Same, but for 73-88                               */
  int xsc;			/* 73-76. Source. X coordinate.                              */
  int ysc;			/* 77-80.    "    Y      "    .                              */
  int xrc;			/* 81-84. Receiver. X coordinate.                            */
  int yrc;			/* 85-88.    "      Y    "      .                            */
  short map_unit;		/* 89-90. Units for map. 1=length,2=secofarc                 */
  short wvel;			/* 91-92. Weathering velocity.                               */
  short swvel;			/* 93-94. Subweathering velocity.                            */
  short suphole;		/* 95-96. Uphole time at source                              */
  short ruphole;		/* 97-98. Uphole time at receiver                            */
  short dummy1[5];		/* 99-108. Further parameters.                               */
  short delay;			/* 109-110. Delay recording time (m.s.)                      */
  short strtmute;		/* 111-112. Mute time - start                                */
  short endmute;		/* 113-114. Mute time - end                                  */
  short nttr;			/* 115-116. # of samples this trace                          */
  short dt;			/* 117-118. Sampling interval this trace                     */
  short gaintype;		/* 119-120 1-fixed,2-binary,3-fl.pt.,4-N                     */
  short gaincon;		/* 121-122 gain constant                                     */
  short initgain;		/* 123-124 initial gain                                      */
  short correl;			/* 125-126 1-uncorrelated, 2-correlated                      */
  short stfreq;			/* 127-128 starting sweep frequency                          */
  short enfreq;			/* 129-130 ending sweep frequency                            */
  short swplen;			/* 131-132 sweep length in milliseconds                      */
  short swptyp;			/* 133-134 
				   sweep type: 1-linear, 2-parabolic,
				   3=exponential, 4-other                                    */
  short sttaplen;		/* 135-136 starting taper in milliseconds                    */
  short entaplen;		/* 137-138 ending taper in milliseconds                      */
  short taptyp;			/* 139-140 taper type: 1-lin,2-cos*cos,3-other               */
  short aafilt;			/* 141-142 alias filter frequency                            */
  short aaslope;		/* 143-144 alias filter slope                                */
  short notfilt;		/* 145-146 notch filter frequency                            */
  short notslope;		/* 147-148 notch filter slope                                */
  short lcfilt;			/* 149-150 low cut frequency                                 */
  short hcfilt;			/* 151-152 high cut frequency                                */
  short lcslope;		/* 153-154 low cut slope                                     */
  short hcslope;		/* 155-156 high cut slope                                    */
  short year;			/* 157-158 year data was recorded                            */
  short julday;			/* 159-160 day of year  (julian day)                         */
  short hour;			/* 161-162 hour of day of source trigger                     */
  short minute;			/* 163-164 minute of hour of source trigger                  */
  short second;			/* 165-166 second of minute of source trigger                */
  short tbasis;			/* 167-168 time basis code 1= local time 2= gmt              */
  short tweight;		/* 169-170 trace wighting factor                             */
  short ggnr;			/* 171-172 Geophone group number of 
				   roll switch position one                                  */
  short ggnt;			/* 173-174 Geophone group number of 
				   trace number one within original field record             */
  short ggnlt;			/* 175-176 Geophone group number of 
				   last trace within original field record                   */
  short gap_size;		/* 177-178 Gap size (total number of groups dropped          */
  short over_trvl;		/* 179-180 Over travel associated 
				   with taper 1=down 2= ahead                                */
  int X_ens;			/* 181-184 X coordinate of ensemble                          */
  int Y_ens;			/* 185-188 Y coordinate of ensemble                          */
  int inline_numb;		/* 189-192 3-D poststack data in-line number                 */
  int xline_numb;		/* 193-196 3-D poststack data cross-line number              */
  int Sp_numb;			/* 197-200 Shotpoint number                                  */
  short Sp_scalar;		/* 201-202 Scalar applied to Shotpoint number                */
  short T_unit;			/* 203-204 Trace value measurement unit -1=other
				   0=unknown   1=Pascal(Pa)   2=Volts  3=Millivolts
				   4=Amperes   5=Meters       6=Meters/second
				   7=Meters/second squared    8=Newton 9=Watt                */
  int T_cons;			/* 205-210 Transduction Constant                             */
  short T_units;		/* 211-212 Transduction Units. Look it up!!                  */
  short DT_ident;		/* 213-214 Device/Trace Identifier                           */
  short UH_scalar;		/* 215-216 Scalar for Uphole time                            */
  short S_orient;		/* 217-218 Source Type/Orientation                           */
  int SE_dir;			/* 219-224 Source Energy Direction                           */
  int S_measure;		/* 225-230 Source Measurement                                */
  short S_unit;			/* 231-232 Source Measurement Unit                           */
  short Un_ass[4];		/* 233-240 Unassigned                                        */
} ShotHeader;

float floatSig[65535];
