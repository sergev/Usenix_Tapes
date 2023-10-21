From CROFT@SUMEX-AIM.ARPA Thu Aug  2 19:22:19 1984
Received: from SUMEX-AIM.ARPA by safe with TCP; Thu, 2 Aug 84 19:22:10 pdt
Date: Thu 2 Aug 84 19:19:27-PDT
From: Bill Croft <CROFT@SUMEX-AIM.ARPA>
Subject: [Peterr%toronto.csnet@csnet-relay.arpa: MacPaint -> bitmap filter, in C]
To: croft@SUMEX-AIM.ARPA
Status: R

Mail-From: PATTERMANN created at  2-Aug-84 14:14:03
Return-Path: <Peterr%toronto.csnet@csnet-relay.arpa>
Received: from csnet-relay by SUMEX-AIM.ARPA with TCP; Tue 31 Jul 84 23:41:00-PDT
Received: From toronto.csnet by csnet-relay;  1 Aug 84 2:20 EDT
Date:     31 Jul 84 22:56:49-EDT (Tue)
From:     Peterr%toronto.csnet@csnet-relay.arpa
To:       info-mac@sumex-aim.arpa
cc:       furuta@washington.arpa
Subject:  MacPaint -> bitmap filter, in C
ReSent-date: Thu 2 Aug 84 14:14:03-PDT
ReSent-From: Ed Pattermann <PATTERMANN@SUMEX-AIM.ARPA>
ReSent-To: info-mac: ;

The following program allows printing of MacPaint documents on devices which
accept raw bitmaps as input.  It provides for simple scaling-up of the
images to accomodate the typical 200-300dpi resolutions of electrostatic
plotters and laser printers.  It requires a machine with a 32 bit integer,
but should be very portable otherwise.

------------------------
/*  MacPaint format -> bitmap filter
    P. Rowley, U. Toronto, July 30, 1984.
    peterr%toronto@CSNET-RELAY

    This program reads a standard MacPaint document on the standard input
    and produces a bitmap file on the standard output.  The bitmap is
    some magnification of the MacPaint 576 (horiz) x 720 (vertical) encoded
    image, possibly with horizontal 0 padding.

    The specific bitmap format is determined at compile time,
    for the sake of efficiency.
*/
#include <stdio.h>
/* The manifests define the output bitmap format as follows:
     The bitmap is OUT_HORIZ_SIZE bytes wide and 720*OUT_VERT_MUL tall,
     with each Mac pixel translated into a OUT_HORIZ_MUL x OUT_VERT_MUL
     rectangle.  There are OUT_HORIZ_PAD bytes of left margin on every
     output scan line.

   The following restrictions apply to the manifests:
	OUT_HORIZ_SIZE >= OUT_HORIZ_PAD + 72 * OUT_HORIZ_MUL
	OUT_HORIZ_PAD  >= 0
	OUT_HORIZ_MUL  = 1, 2, 3, or 4
        OUT_VERT_MUL   >= 1
   Sample values (for a Versatec plotter): 264, 24, 3, 3
*/
#define OUT_HORIZ_SIZE			264
#define OUT_HORIZ_PAD			24
#define OUT_HORIZ_MUL			3
#define OUT_VERT_MUL			3

char in_line[72], out_line[OUT_HORIZ_SIZE];
int  xlat[256];
main() {
	int i, j;

	/* initialize tables and buffers */
	init();

	/* read and discard 512 byte header */
	for (i=0; i<512; i++)
		getchar();

	/* read and process each of the 720 scan lines */
	for (i=0; i<720; i++) {
		read_scan();
		write_scan();
	}
}

read_scan() {
	int in_pos, count, data_byte;

	in_pos = 0;
	while (in_pos < 72) {
		count = getchar();
		if (count > 127) count -= 256;

		if (count >= 0) {		/* run of raw bytes */
			count++;		/* # of bytes to read */
			while (count--)
				in_line[in_pos++] = getchar();
		}
		else {				/* run of repeated byte */
			count = -count+1;	/* repetition factor */
			data_byte = getchar();  /* byte to repeat */

			while (count--)
				in_line[in_pos++] = data_byte;
		}
	}
}

write_scan() {
	int i, j, outword;

	/* prepare output scan line:
		OUT_HORIZ_PAD 0's,
		72 * OUT_HORIZ_MUL data bytes,
		enough 0's for a total of OUT_HORIZ_SIZE bytes
	*/

	/*  leading and trailing 0's are in out_line by default */

	j = OUT_HORIZ_PAD;
	for( i=0; i<72; i++ ) {
		outword = xlat[in_line[i] & 255];
#if (OUT_HORIZ_MUL == 4)
		out_line[j++] = (outword >> 24) & 255;
#endif
#if (OUT_HORIZ_MUL >= 3)
		out_line[j++] = (outword >> 16) & 255;
#endif
#if (OUT_HORIZ_MUL >= 2)
		out_line[j++] = (outword >>  8) & 255;
#endif
		out_line[j++] = (outword      ) & 255;

	}

	/* output the scan line, OUT_VERT_MUL times */
	for( i=0; i<OUT_VERT_MUL; i++ )
		for( j=0; j<OUT_HORIZ_SIZE; j++ )
			putchar( out_line[j] );
}

init() {
	int bits[8], i, j;

	/* initialize translation table */
	j = (1<<OUT_HORIZ_MUL) - 1;
	for( i=0; i<8; i++ ) {
		bits[i] = j;
		j *= (1 << OUT_HORIZ_MUL);
	}

	for( i=0; i<256; i++ ) {
		if( i &	  1 )	xlat[i] = bits[0];
		else		xlat[i] = 0;

		if( i &   2 )	xlat[i] += bits[1];
		if( i &   4 )	xlat[i] += bits[2];
		if( i &   8 )   xlat[i] += bits[3];
		if( i &  16 )	xlat[i] += bits[4];
		if( i &  32 )	xlat[i] += bits[5];
		if( i &  64 )	xlat[i] += bits[6];
		if( i & 128 )	xlat[i] += bits[7];
	}
}
-------

