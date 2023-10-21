/*
 *  Top - a top users display for Berkeley Unix
 *
 *  General (global) definitions
 */

/* Number of lines of header information on the standard screen */
#define Header_lines	6

/* Number of columns needed for display */
#define Display_width	80

/* Log base 2 of 1024 is 10 (2^10 == 1024) */
#define LOG1024		10

/* Convert clicks (kernel pages) to kbytes ... */
/* If there is no PGSHIFT defined, assume it is 11 */
/* Is this needed for compatability with some old flavor of 4.2 or 4.1? */
#ifndef PGSHIFT
#define pagetok(size)	((size) << 1)
#else
#if PGSHIFT>10
#define pagetok(size)	((size) << (PGSHIFT - LOG1024))
#else
#define pagetok(size)	((size) >> (LOG1024 - PGSHIFT))
#endif
#endif

extern double logcpu;

double log();
double exp();

extern char (* screenbuf)[Display_width];
