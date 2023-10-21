/*
 * Convert a character to an integer, performing sign extension even on
 * machines that don't provide hardware support for signed characters.
 * This routine should be replaced by the macro
 *	#define schar(c)	((char)(c))
 * for compilers that support signed characters.
 */

/* The high bit of a character */
#define SIGNBIT (int)((unsigned char)(-1) &~ ((unsigned char)(-1) >> 1))
#define EXTBITS (int)(-1 &~ (unsigned char)(-1))

int
schar(c)
      char c ;
      {
      return (c & SIGNBIT) ? c | EXTBITS : c ;
}
