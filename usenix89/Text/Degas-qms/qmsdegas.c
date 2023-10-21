
/* QMSDEGAS
 *
 *	Convert a file from Degas format (32034 bytes) to
 *	QMS QUIC 8-bit free format bitmap.
 *
 *	Copyright 1987, Todd Aven
 *	May be freely distributed provided no fee is charged
 *	for its use or possession. Send modifications to
 *
 * Internet:	todd@cincom.umd.edu
 * Bitnet:	todd@umcincom
 *
 * MODIFICATIONS:
 *
 *	This is brand new. If you look for 'MODIFY:' you'll
 *	find suggestions for what to work on next. Please keep
 *	a record of changes, use #ifdef's copiously.
 *
 *	Version 0.1 runs on VAX/VMS. /tsa
 *
 * ORIGINAL AUTHOR:
 *
 *	Todd Aven
 *	the Softwear Sweatshop
 *	7833 Walker Drive, Suite 308
 *	Greenbelt, MD 20770
 *
 */

#include <stdio.h>

/* MODIFY: If you know what these bytes mean, add some code. */
#define SKIP_BYTES 34		/* Presently ignore first 34 bytes. */

#define USAGE "\n usage:  qmsdegas <degas file> <qms file>\n\n"

#define QMSHEADER "^PY^-\n"	/* Turns on QUIC commands. */
#define QMSTRAILER "^PN^-\n"	/* Turns 'em off again. */
#define QMSENDLINE "^X\n"	/* Ignore the CRLF. */
#define QMSBEGINLINE "^A"	/* Pay attention to data now. */
#define EIGHTFREE "\n^(^X\n"	/* Go to 8-bit free format mode. */
#define NOEIGHTFREE "^A^)^-\n"	/* No more free format. */
/* MODIFY: Add options for selecting magnification factors. */
#define QMSPLOTSTART "^IP0404^P0640" /* Begin plot mode, 4X expanded.*/
#define QMSPLOTEND "^G"		/* End plot mode. */

main(argc,argv)
     int argc;
     char *argv[];
{
  FILE *infile,*outfile;	/* infile = DEGAS, outfile = QMS */
  char bitmap[400][80],throwaway; /* 400 rows, 640 columns (8 columns/char) */
  register int i,row,column;	/* i is temp, row and column are for bitmap */

  if(argc != 3) {
    fprintf(stderr,USAGE);
    exit ();
  }

  /* Open the DEGAS picture for reading. */
  if( !(infile = fopen (argv[1],"rb"))){
    fprintf (stderr,"\nCouldn't open %s for reading.",argv[1]);
    exit ();
  }
    
  /* Open the QMS file for writing. */
  if( !(outfile = fopen (argv[2],"wb","rfm=var"))){
    fprintf (stderr,"\nCouldn't open %s for writing.",argv[2]);
    exit ();
  }
    
  /* Skip the first N bytes. */
  for( i = 0; i < SKIP_BYTES; i++) throwaway = fgetc(infile);

  /* Read DEGAS picture into the bitmap for manipulation. */
  for( row = 0; row < 400; row++){
    for( column = 0; column < 80; column++){
      bitmap[row][column] = fgetc(infile);
    }
  }

  /* Put the preliminary garbage into the QMS file. */
  fprintf(outfile,QMSHEADER);

  /* Put the printer in 8-bit free format. */
  fprintf(outfile,EIGHTFREE);

  /* Put the printer in plot mode. */
  fprintf(outfile,QMSBEGINLINE);
  fprintf(outfile,QMSPLOTSTART);
  fprintf(outfile,QMSENDLINE);

  /* Put the bitmap into the output file. */
  for( row = 0; row < 400; row++){
    fprintf(outfile,QMSBEGINLINE);
    for( column = 0; column < 80; column++){
      fputc(bitmap[row][column],outfile);
      if( bitmap[row][column] == '^' )
	fputc(bitmap[row][column],outfile); /* '^' must be doubled. */
    }
    fprintf(outfile,QMSENDLINE);
  }
  
  /* End plot mode. */
  fprintf(outfile,QMSBEGINLINE);
  fprintf(outfile,QMSPLOTEND);
  fprintf(outfile,QMSENDLINE);

  /* Kill 8-bit free format */
  fprintf(outfile,NOEIGHTFREE);

  /* Add trailer garbage for the QMS file. */
  fprintf(outfile,QMSTRAILER);

  /* Close the files. */
  fclose(infile);
  fclose(outfile);

  /* All done. */
}
