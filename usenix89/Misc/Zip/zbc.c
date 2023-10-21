
/*
 *   Program:      zbc.c   ZIP Bar Code
 *   Author:       Jim Trethewey
 *                 Intel OMS BITBUS Engineering.
 *   Version:      V1.0.
 *   Date:         02-Apr-1987.
 *   Language:     Microsoft C 286.
 *   Ppn:          ..!argent!mithril!flamer  [or]  flamer@mithril.hf.intel.COM
 *
 *   Description:
 *
 *      This program will provide you interest for about two minutes.
 *      It was an exercise in reverse engineering the little bar codes
 *      that the U.S. Postal Service puts at the bottom of envelopes,
 *      and this program was written primarily for hack value.
 *
 *      This program takes one of four options, summarized as follows:
 *
 *         1.  Encode in Bar Code the given ZIP.
 *                zbc -e <zip>
 *                zbc -e <zip+4>
 *         2.  Encode in Bar Code the given ZIP, and be verbose about it.
 *                zbc -ev <zip>
 *                zbc -ev <zip+4>
 *         3.  Decode the given Bar Code to ZIP.
 *                zbc -d "<barcode>"
 *         4.  Decode the given Bar Code to ZIP, and be verbose about it.
 *                zbc -dv "<barcode>"
 *
 *      Because the two functions are inverses, you can do neat things like:
 *
 *         zbc -d "`zbc -e 94601`"
 *
 *   Modification History:
 *
 *      M000   02-Apr-1987   flamer           Original issue.
 *
 */

#include <stdio.h>
#include <ctype.h>

#ifdef UCB
#define strchr          index
#endif
extern char *strchr ();

/*
 *   The definitions for ZERO and ONE indicate which characters are
 *   used and accepted for the bar code bits.  They can be anything
 *   you want so long as they're different.  Depending upon your
 *   CRT or printer, '.' is best for ZERO and '|' for ONE.
 */

#ifdef LASERJET
#define ZERO            ','
#define ONE             '|'
#else
#define ZERO            '.'
#define ONE             '|'
#endif

#define FALSE           0
#define TRUE            !FALSE
#define SAME            0


char zbc [10][6] = {
   { ONE,  ONE,  ZERO, ZERO, ZERO, 0 },   /* 0 = "||..." */
   { ZERO, ZERO, ZERO, ONE,  ONE,  0 },   /* 1 = "...||" */
   { ZERO, ZERO, ONE,  ZERO, ONE,  0 },   /* 2 = "..|.|" */
   { ZERO, ZERO, ONE,  ONE,  ZERO, 0 },   /* 3 = "..||." */
   { ZERO, ONE,  ZERO, ZERO, ONE,  0 },   /* 4 = ".|..|" */
   { ZERO, ONE,  ZERO, ONE,  ZERO, 0 },   /* 5 = ".|.|." */
   { ZERO, ONE,  ONE,  ZERO, ZERO, 0 },   /* 6 = ".||.." */
   { ONE,  ZERO, ZERO, ZERO, ONE,  0 },   /* 7 = "|...|" */
   { ONE,  ZERO, ZERO, ONE,  ZERO, 0 },   /* 8 = "|..|." */
   { ONE,  ZERO, ONE,  ZERO, ZERO, 0 }    /* 9 = "|.|.." */
};

int encode_flag, decode_flag, verbose_flag;
int valid_flag;


value_of (bit)
   
   char bit;

   {
      if (bit == ZERO)
	 {
	    return (0);
	 }
      else if (bit == ONE)
	 {
	    return (1);
	 }
      else
	 {
	    return (-1);
	 }
   }

put_start_bit (position)

   char *position;

   {
      *position = (ONE);
   }

put_stop_bit (position)

   char *position;

   {
      *position = (ONE);
   }

char *ztob (zipcode)

   char *zipcode;

   {
      static char barcode [100];
      char *ch_p;
      char *out_p;
      int checksum;
      int i, j;

      checksum = 0;
      out_p = &barcode [0];
      put_start_bit (out_p++);
      for (ch_p = zipcode, i = 0; ch_p && *ch_p; ch_p++, i++)
	 {
	    if (isdigit (*ch_p))
	       {
		  strcpy (out_p, zbc [*ch_p - '0']);
		  out_p += 5;
		  checksum += (*ch_p - '0');
	       }
	 }
      checksum = 10 - checksum % 10;
      strcpy (out_p, zbc [checksum]);
      out_p += 5;
      put_stop_bit (out_p++);
      *out_p = '\0';
      return (barcode);
   }

char *btoz (barcode)

   char *barcode;

   {
      static char zipcode [100];
      char *ch_p;
      char *out_p;
      int i, j;
      int checksum;
      int expected_checksum;
      int parity;
      int found;
      int digits_seen;
      int digit_length;
      int checksum_flag;
      int error_flag;
      int new_digit;

      error_flag = FALSE;
      checksum = 0;
      expected_checksum = -1;
      out_p = &zipcode [0];
      digits_seen = 0;
      digit_length = (strlen (barcode) - 2) / 5;
      for (ch_p = barcode, i = 0; ch_p && *ch_p; ch_p++, i++)
	 {
	    if (ch_p == barcode)   /* at the start bit */
	       {
		  if (*ch_p != ONE)
		     {
			fprintf (stderr, "Invalid start bit.\n");
		     }
		  else if (verbose_flag)
		     {
			*out_p++ = ('_');
		     }
	       }
	    else if (*(ch_p + 1) == 0)   /* at the stop bit */
	       {
		  if (checksum % 10 != 0)
		     {
			fprintf (stderr, 
			   "Checksum error (actual = %d, expected = %d).\n", 
			   checksum % 10, expected_checksum);
			if (error_flag == 0)
			   {
			      fprintf (stderr, "Too many bit errors to fix.\n");
			      fprintf (stderr, "Can't find position of error,");
			      fprintf (stderr, " bad ZIP code returned.\n");
			   }
		     }
		  if (*ch_p != ONE)
		     {
			fprintf (stderr, "Invalid stop bit.\n");
		     }
		  else
		     {
			if (verbose_flag)
			   {
			      *out_p++ = ('_');
			   }
		     }
		  if (error_flag > 1)
		     {
			fprintf (stderr, "Too many bit errors to fix.\n");
		     }
		  else if (error_flag == 1)
		     {
			new_digit = (10 - checksum % 10) % 10;
			fprintf (stderr, "Correcting digit to a \"%d\".\n",
			   new_digit);
			if (!checksum_flag || verbose_flag)
			   {
			      *strchr (zipcode, '?') = new_digit + '0';
			   }
			else
			   {
			      *strchr (zipcode, '?') = '\0';
			   }
		     }
	       }
	    else   /* in the middle somewhere */
	       {
		  checksum_flag = (digits_seen == digit_length - 1);
		  if ((digits_seen == 5) && !checksum_flag)
		     {
			*out_p++ = ('-');
		     }
		  found = FALSE;
		  for (j = 0; j < 10; j++)
		     {
			if (strncmp (ch_p, zbc [j], 5) == SAME)
			   {
			      found = TRUE;
			      if (checksum_flag && verbose_flag)
				 {
				    *out_p++ = ('[');
				 }
			      if (!checksum_flag || verbose_flag)
				 {
				    *out_p++ = (j + '0');
				 }
			      if (checksum_flag)
				 {
				    expected_checksum = j;
				 }
			      if (checksum_flag && verbose_flag)
				 {
				    *out_p++ = (']');
				 }
			      break;
			   }
		     }
		  if (!found)
		     {
			/*
			 *   Check parity.
			 */
			parity = value_of (*ch_p) 
			       + value_of (*(ch_p + 1))
			       + value_of (*(ch_p + 2))
			       + value_of (*(ch_p + 3))
			       + value_of (*(ch_p + 4));
			if (parity != 2)
			   {
			      error_flag++;
			      fprintf (stderr, 
      "Parity error (actual = %d, expected = 2) in \"%5.5s\" at position %d.\n",
				 parity, ch_p, digits_seen + 1);
			      if (checksum_flag && verbose_flag)
				 {
				    *out_p++ = ('[');
				 }
			      *out_p++ = '?';
			      if (checksum_flag && verbose_flag)
				 {
				    *out_p++ = (']');
				 }
			   }
		     }
		  else
		     {
			checksum += j;
		     }
		  digits_seen++;
		  ch_p += 4;
	       }
	 }
      *out_p = '\0';
      return (zipcode);
   }



main (argc, argv)

   int argc;
   char *argv [];

   {
      /*
       *   Check out the command line arguments.
       */
      valid_flag = FALSE;
      encode_flag = FALSE;
      decode_flag = FALSE;
      verbose_flag = FALSE;
      if (argc != 3)
	 {
	    valid_flag = FALSE;
	 }
      else if (strncmp (argv [1], "-e", 2) == SAME)
	 {
	    valid_flag++;
	    encode_flag++;
	    if (argv [1][2] == 'v')
	       {
		  verbose_flag++;
	       }
	 }
      else if (strncmp (argv [1], "-d", 2) == SAME)
	 {
	    valid_flag++;
	    decode_flag++;
	    if (argv [1][2] == 'v')
	       {
		  verbose_flag++;
	       }
	 }
      if (!valid_flag)
	 {
fprintf (stderr, 
   "Usage: %s -e <zipcode>        Encode ZIPcode in bar code.\n",
   argv [0]);
fprintf (stderr, 
   "       %s -ev <zipcode>       Encode ZIPcode in bar code verbosely.\n",
   argv [0]);
fprintf (stderr, 
   "       %s -d \"<barcode>\"      Decode bar code to ZIPcode.\n", 
   argv [0]);
fprintf (stderr, 
   "       %s -dv \"<barcode>\"     Decode bar code to ZIPcode verbosely.\n", 
   argv [0]);
	    exit (1);
	 }

      /*
       *   Do the requested conversion.
       */
      if (encode_flag)
	 {
	    printf ("%s\n", ztob (argv [2]));
	 }
      else if (decode_flag)
	 {
	    printf ("%s\n", btoz (argv [2]));
	 }
   }
