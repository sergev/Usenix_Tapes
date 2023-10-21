#ifndef lint
static	char sccsid[] = "@(#)paintps.c	1.7 6/11/86 (UT)";
#endif

/*
 * paintps -- Convert MacPaint document to PostScript document 
 * 
 * Brian H. Powell, University of Texas at Austin
 *	brian@sally.UTEXAS.EDU
 *	brian@ut-sally.UUCP
 *	CS.Powell@r20.UTEXAS.EDU
 *
 * This program may be used but not sold without permission.
 *
 * Modification History
 * 	BHP	7/25/85	Created.
 *	BHP	7/31/85	Optimized output routines for execution speed.
 *	BHP	4/22/86	Added -P option, extracted spool command to Makefile.
 *	BHP	6/ 3/86 Fixed bug causing (rare) small white gaps between
 *			bitmaps.
 */

#include <stdio.h>
#include <ctype.h>

#define FALSE 0
#define TRUE  (-1)

#define MACPAINT_HDRSIZE	512
#define MACRASTER_BITWIDTH	576
#define	MACPAINT_BITHEIGHT	720

			/* DEFAULT OPTIONS */
int smooth = TRUE;		/* smoothing enabled */
int copies = 1;			/* one copy of the painting */
char *printername = NULL;	/* default printer */

char hexbuffer[MACPAINT_BITHEIGHT][MACRASTER_BITWIDTH/8];
int blank[MACPAINT_BITHEIGHT + 4];	/* we need the four extra blank */
					/* lines as filler for output */

char usage[] = "paintps [-d] [-#<n>] [-P<printer>] file";

main(ac, av)
char **av;
{
   ac--; av++;
   while (ac && (av[0][0] == '-') && av[0][1]) {
      switch (av[0][1]) {
	 case 'd':			/* "draft" option */
	    smooth = FALSE;
	    break;

	 case '#':			/* number of copies */
	    copies = atoi (&av[0][2]);
	    break;

	 case 'P':			/* printer name */
	    printername = av[0];
	    break;

	 default:
	    goto bad_usage;
      }
      ac--; av++;
   }
   open_output();		/* also starts spooling to the spool cmd. */
   paintfilter(av[0]);
   exit(0);

bad_usage:
   fprintf(stderr, "usage: %s\n", usage);
   exit(1);
}

/* convert from MacPaint to PostScript */

paintfilter(name)
char *name;
{
   register int x, y;
   FILE *fp;
   register int c;
   int blankline;

   fp = fopen(name, "r");		/* open the MacPaint file. */
   if (fp == NULL) {
      perror(name);
      return;
   }

   (void) fseek(fp, (long)MACPAINT_HDRSIZE, 0);	/* skip the MacPaint patterns */

   begin_image();				/* print postscript header */

/*  We skip horizontal whitespace.  "blankline" is used to diagnose this
    for the current line.  The boolean array "blank[y]" shows which lines
    in the entire painting are blank. */

   for (y = 0; y < MACPAINT_BITHEIGHT; y++) {
				/* for each pixel row... */
      blankline = TRUE;
      for (x = 0; x < MACRASTER_BITWIDTH/8; x++) {

         if ((c = getbits(fp)) == EOF) {
            fprintf(stderr, "Unexpected EOF, stopped.\n");
            for ( ; y < MACPAINT_BITHEIGHT; y++)		/* Mark the */
               blank[y] = TRUE;				/* rest of the page */
            blankline = TRUE;		       /* blank, print it and quit. */
            break;

         } else {
            hexbuffer[y][x] = (char) c;	  /* else record the character and */
            if (c)			  /* mark the line as non-blank. */
               blankline = FALSE;
         }
      }
      blank[y] = blankline;
   }
   blank[y++] = TRUE;		/* put four blank lines at end */
   blank[y++] = TRUE;		/* (I think this is needed for smoothing.) */
   blank[y++] = TRUE;
   blank[y] = TRUE;
   dump_buffer(hexbuffer, blank);	/* print the buffer (entire page) */

   end_imag(copies);		/* print the postscript trailer. */
   (void) fclose(fp);		/* close the file. */
}

/* macpaint input routines */

getbits(fp)		/* this routine scarfed from paintimp. */
FILE *fp;
{
/* This routine expands the PackBits encoded MacPaint file,
   delivering one byte per call.
*/

   static int count, rep, chr;
   int c;

   if (rep) {		/* if we are repeating a previous character, */
      rep--;		/* reduce the repeat count and return the char. */
      return chr;
   }
   if (count) {		/* if we are in an unrepeated section, reduce */
      count--;		/* the count and return next char from the file. */
      return getc(fp);
   }
   c = getc(fp);	/* otherwise, get the next repeat count.  */
   if (c & 0x80) {	/* if negative, repeat the next */
      rep = 0x100 - c;	/* byte (2's comp(c)+1) times. */
      chr = getc(fp);		/* the character to repeat */
      return chr;
   }
   else {		/* if repeat count is positive, next "count" bytes */
      count = c;   	/* are unencoded.  */
      return getc(fp);
   }
}

/* PostScript output routines */

open_output()
{
   char lpr [64];
   FILE *popen();

/* if stdout is going to the terminal, call the spool command to spool the
   output, otherwise assume it's being piped somewhere else
   or directed into a file.
*/
   if (isatty (fileno (stdout)))
   {
      (void) sprintf (lpr, "%s %s", SPOOLCMD,
			printername ? printername : "");
		/* include printer name if it was specified.  */

	/* disconnect stdout from the tty and pipe it to lpr */
      (void) fclose (stdout);
      if (popen (lpr, "w") == NULL) {
         fprintf(stderr, "%s failed, giving up.\n", lpr);
         exit(2);
      }
      fprintf(stderr, "Spooling to '%s'.\n", lpr);
   }

}

begin_image()		/* print the postscript header. */
{
   printf("%%!\n");		/* TranScript PostScript flag "%!" */
   printf("md begin\n");	/* standard apple header */
   printf("1320 od\n");
   printf("(; user: )jn\n");
   printf("%%%%EndProlog\n");
   printf("%%%%Page: ? ?\n");
   printf("op\n");
   printf("0 0 moveto\n");
}

/* The reason the following routine isn't called "end_image" is because */
/* of a bug in the Sequent C compiler.  */
end_imag(copies)	/* print the postscript trailer. */
int copies;		/* copies == number of copies to print */
{
   printf("%d page\n", copies - 1);	/* "page" takes # of pages to */
   printf("cp\n");           		/* print minus 1 */
   printf("%%%%Trailer\n");
   printf("end\n");
}

#define OUTBUF_SIZE	28
#define XSCALE		578
#define XLOC		0
#define ROWBYTES	74
#define SRCMODE		1

	/* Modular code purists can close their eyes for the following */
	/* routine.  */
dump_buffer(hexbuffer, blank)
char hexbuffer[MACPAINT_BITHEIGHT][MACRASTER_BITWIDTH/8];
int blank[MACPAINT_BITHEIGHT];
   /* hexbuffer contains the entire unencoded MacPaint file. */
   /* blank is a boolean array for each row.  TRUE means the horizontal
      line is blank. */
{
   int xscale, yscale, xloc, yloc, xactual, yactual;
   char smoothing;
   int i, j;
   int curline, top_of_sect, begin_line, numblanks, lines_used;
   char outstring[MACRASTER_BITWIDTH/4];

   xscale = XSCALE;		/* Postscript parameters describing the */
   xloc = XLOC;			/* size of the image area.  */
   xactual = XSCALE;
   smoothing = smooth ? 'T' : 'F';

/*  The postscript output consists of calls to "dobits" for each group of */
/* non-blank horizontal lines.  Due to LaserWriter memory limitations,    */
/* only about 28 lines at a time can be sent per command.  Also, lines    */
/* adjacent to the current section must be included (presumably for the   */
/* smoothing function to work.)						  */

   curline = 0;
   top_of_sect = TRUE;		/* begin a "dobits" call. */

   while (curline < MACPAINT_BITHEIGHT) {
			/* for the entire picture... */

/*  If a previous section terminated because of white space, skip the */
/* blank lines that precede the next section.  */
      if (top_of_sect)
	 while ((curline < MACPAINT_BITHEIGHT) && blank[curline])
            curline++;

/* check for end of page. */
      if (curline >= MACPAINT_BITHEIGHT)
         break;

      if (top_of_sect) {	/* At top of non-white section, include */
         begin_line = curline - 2;	/* previous two lines; */
         lines_used = 2;
         top_of_sect = FALSE;
      } else {			/* if continuing a non-white section, don't. */
         begin_line = curline;
         lines_used = 0;
      }

      numblanks = 0;		/* compute height of non-white area. */
      for (i = lines_used; i < (OUTBUF_SIZE - 1); i++) {
         if (blank[curline++])
            numblanks++;
         else
            numblanks = 0;   /* reset counter */
         if (numblanks >= 4) {	/* Too much white space: break and print it. */
      				/* The four blanks at the end of the */
				/* page will stop us at the bottom */
            curline--;		/* We want curline to point */
      				/* to the fourth blank line. */
            break;
         }
      }
      if (numblanks > 0)	/* If the bitmap ended in any whitespace, */
	 top_of_sect = TRUE;	/* we skip it the next time around.  */
      curline++;
      yactual = i - 3;		/* Height of current bitmap section. */
      yscale = yactual;		/* Scale it 1 to 1.  */
      yloc = begin_line + 2;

/* print the "dobits" call. */
      printf("%d %d %d %d %d %d %d %c %d dobits\n",
		xscale, yscale, xloc, yloc, ROWBYTES,
		xactual, yactual, smoothing, SRCMODE);
/* print the corresponding horizontal lines. */
      for (i = begin_line; i < curline; i++) {
         if ((i < 0) || blank[i])
			/* if blank, print all zeros. */
            for (j = 0; j< MACRASTER_BITWIDTH/4; j++)
               putchar('0');
         else {
			/* else convert to a hex string and print it. */
            tohex(hexbuffer[i], MACRASTER_BITWIDTH/8,
               outstring);
            printf("%s", outstring);
         }
		/* Put four extra nibbles at the ends of lines.  I think */
		/* are for smoothing.  */
	 if ((i == (-2)) || blank[i+1])	/* Include the first four nibbles of */
            printf("0000\n");		/* the next line.  */
	 else {
	    tohex(hexbuffer[i+1], 2, outstring);
	    printf("%s\n", outstring);
	 }
      }
      if (!top_of_sect)
         curline -= 4;		/* we'll have to overlap the next section */
   }				/* with this one, so back up four lines. */
}

tohex(hexline, size, outstring)   /* convert binary bytes to a hex string. */
char hexline[];
int size;		/* number of bytes to convert. */
char outstring[];
{
   int i, j;
   register char c, c1;

   j = 0;
   for (i = 0 ; i < size ; i++) {
      c = hexline[i];
      c1 = (c >> 4) & 0x0F;
      c1 = (c1 <= 9) ? (c1 + '0') : (c1 - 10 + 'A');
      outstring[j++] = c1;
      c1 = c & 0x0F;
      c1 = (c1 <= 9) ? (c1 + '0') : (c1 - 10 + 'A');
      outstring[j++] = c1;
   }
   outstring[j] = NULL;
}
