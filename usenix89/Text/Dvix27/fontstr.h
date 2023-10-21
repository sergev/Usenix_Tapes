/****************************************************************************/
/*                                                                          */
/*	Structures defining the format of Xerox 2700 (EPS 1200)             */
/*	laser printer font files.                                           */
/*                                                                          */
/*      Note: these structures only match the format of the external        */
/*      file when used on PDP-11s, VAXes, or other machines with the        */
/*      same size data types and same byte ordering.                        */
/*                                                                          */
/*	The structures are based on investigative work done by the          */
/*	author, and include help from sources at least one of whom 	    */
/*	wishes to remain anonymous. This help is gratefully		    */
/*	acknowledged.							    */
/*									    */
/*	Tim Clark  Computer Unit  University of Warwick  June 1985          */
/****************************************************************************/

struct xheader {        /* The font header - 24 (16 bit) words */
	unsigned short magic;		/* should be 0xaaaa		*/
	unsigned char rev;		/* revision number - unused	*/
	unsigned char flags;		/* font type flags		*/
	unsigned short lo_length;       /* length of file in bytes lsb  */
	char fname[20];			/* font name in ascii		*/
	unsigned char ulinedepth;	/* posn of underline below base */
	unsigned char ulinesize;	/* thickness of underline	*/
        unsigned char hi_length;        /* length of file in bytes msb  */
	unsigned char strike_through;	/* unused ?                     */
	unsigned char superdist;	/* superscript distance		*/
	unsigned char subdist;		/* subscript distance		*/
	short descend;			/* maximum descender		*/
	short ascend;			/* maximum ascender		*/
	short fheight;			/* font height - 1		*/
	unsigned char lowchar;		/* first defined character	*/
	unsigned char highchar;		/* last defined character	*/
	char partno[5];			/* part number - unused		*/
	char nulls[3];			/* reserved - set to null	*/
};

struct xchardesc {      /* The "look up table" for each character.
                           Each is 4 (16 bit words). */

	unsigned short nbyte; 	/* the length of the pattern in bytes       */

        unsigned short lo_loc;  /* the lowest 16 bits of the location of    */
                                /* the pattern, expressed as a 24-bit       */
                                /* quantity, which is the offset into the   */
                                /* font file                                */

        unsigned char hi_loc;   /* the highest 8 bits of the pattern        */
                                /* location offset (0xff equivalent to 0)   */

        unsigned char blocking; /* if "blocking" is zero then there is no   */
                                /* pattern information. Otherwise the value */
                                /* 63-blocking gives the number of bytes in */
                                /* each row of the character pattern        */

        char orgy;  		/* in two's complement. How much the        */
                                /* character is above (or if -ve, below)    */
                                /* the baseline (portrait) or from the      */
                                /* typing position (landscape). Units are   */
                                /* DOUBLE rasters.                          */

	unsigned char width;    /* the amount to move the typing position   */
                                /* on after drawing this character, in      */
                                /* units of rasters.                        */
	};

struct def_str		/* This structure is not found in the font file -
			   it is for convenience, holding information for
			   each character derived from the "look up table". */

	{
	int bytes_per_row;	/* derived from xchardesc.blocking to give  */
				/* the number of bytes needed for each row  */
				/* of the pattern			    */

	char *pattern;		/* the start of the pattern		    */

	long location;		/* derived from hi_loc and lo_loc to give   */
				/* the 24-bit index into the font file      */
	};

/* Font type flags */
#define	PORTRAIT	0		/* 8 1/2 is width */
#define LANDSCAPE	1		/* 8 1/2 is length */
#define FIXED_SP	0		/* fixed pitch */
#define PROP_SP		2		/* proportional spacing */
#define SHORT_FONT      0		/* font fits into 0xffff bytes */
#define LONG_FONT	4		/* font is more than 0xffff bytes */

/****************************************************************************/
/*                                                                          */
/*      The overall structure of the font is then as follows:-              */
/*                                                                          */
/*      A 24 (16 bit)word header of struct xheader			    */
/*                                                                          */
/*      A 256 byte "qualification table?"                                   */
/*                                                                          */
/*      A number of "loook up tables" of struct xchardesc. The number	    */
/*      is xheader.highchar + 1.					    */
/*                                                                          */
/*      The character patterns.                                             */
/*                                                                          */
/****************************************************************************/


/****************************************************************************/
/*                                                                          */
/*	vfont format. Taken from vfont(5)				    */
/*                                                                          */
/****************************************************************************/

struct vfontheader {
	short		magic;
	unsigned short	size;
	short		maxx;
	short		maxy;
	short		xtnd;
	};

struct dispatch {
	unsigned short	addr;
	short		nbytes;
	char		up;
	char		down;
	char		left;
	char		right;
	short		width;
	};
