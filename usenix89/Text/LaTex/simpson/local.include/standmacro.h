/* @(#)standmacro.h	1.1 (TRW) 6/13/84 */
/* Maximum of two values. */
#define	MAX(x, y) ((x) > (y) ? (x) : (y))

/* Minimum of two values. */
#define	MIN(x, y) ((x) < (y) ? (x) : (y))

/* Absolute value. */
#define	ABS(x) ((x) < 0 ? -(x) : (x))
