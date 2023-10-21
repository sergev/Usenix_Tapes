/*
 * Translate phone number(s) to letter combinations.
 */

#include <stdio.h>
#include <ctype.h>

#define chNull  '\0'
#define LINE    79              /* columns per line */

char    sbBuf[LINE];            /* partial results  */
int     numsize;                /* digits in number */
int     cols;                   /* printed on line  */

char    *sbMap[] = { "0",   "1",   "ABC", "DEF", "GHI",
                     "JKL", "MNO", "PRS", "TUV", "WXY",
                   };


/****************************************************************
 * M A I N
 */

main (argc, argv)
    int         argc;
    char        **argv;
{
    char        *sbStart;               /* start of number */
    char        *sbEnd;                 /* end of number   */

/*
 * CHECK USAGE:
 */

    if (argc < 2)
    {
        fprintf (stderr, "usage: %s phonenumbers...\n", *argv);
        exit (1);
    }

/*
 * FIND NUMBERS IN ARGUMENTS:
 */

    while (*(++argv))                           /* more args to do */
    {
        sbStart = *argv;

        while (1)                               /* until arg done */
        {
            while ((*sbStart != chNull) && (! isdigit (*sbStart)))
                sbStart++;                      /* find start of number */

            if (*sbStart == chNull)             /* arg is done */
                break;

            sbEnd = sbStart;
            while (isdigit (*(++sbEnd)))        /* find end of number */
                ;
            if (*sbEnd  != chNull)              /* not end of arg         */
                *sbEnd++ = chNull;              /* plant null and advance */

/*
 * DO ONE NUMBER (STRING OF DIGITS):
 */

            printf ("\n%s:\n", sbStart);
            cols    = 0;                        /* none printed yet     */
            numsize = strlen (sbStart);         /* total length of num  */
            sbBuf [numsize - 1] = chNull;       /* terminal for later   */
            DoRest (sbStart, sbBuf);            /* uses cols, numsize   */
            printf ("\n");
            sbStart = sbEnd;                    /* advance to next */
        }
    }
} /* main */


/****************************************************************
 * D O   R E S T
 *
 * Show combinations for the rest of a number (rightmost digits).
 * Calls itself recursively.  Each call eats one digit and builds
 * results in sbBuf (pointed into by sbNext).  When on rightmost
 * digit, dumps each combination for that digit.  Uses cols and
 * numsize to control printing (line wrap).  Leaves last line
 * unfinished.
 */

DoRest (sbNum, sbNext)
    char        *sbNum;                 /* number to do     */
    char        *sbNext;                /* next pos in buf  */
{
    int         digit     = *sbNum - '0';       /* current digit value  */
    char        *sbMapPos = sbMap [digit];      /* where in map         */

/*
 * GO THROUGH COMBINATIONS FOR DIGIT:
 */

    while (*sbMapPos)                   /* more combinations for digit */
    {
        if (sbNum[1] != chNull)         /* not last digit */
        {
            *sbNext = *sbMapPos;        /* save combination for digit */
            DoRest (sbNum+1, sbNext+1);
        }

/*
 * LAST DIGIT OF NUMBER:
 */

        else
        {
            if (cols > 0)               /* some printed already */
            {
                if ((cols + numsize + 1) <= LINE)
                    printf (" ");       /* end last combination */
                else
                {
                    printf ("\n");      /* finish line */
                    cols = 0;
                }
            }
            cols += numsize + 1;

            /* combinations for previous digits plus last digit */
            printf ("%s%c", sbBuf, *sbMapPos);

        } /* if */

        sbMapPos++;

    } /* while */
} /* DoRest */
