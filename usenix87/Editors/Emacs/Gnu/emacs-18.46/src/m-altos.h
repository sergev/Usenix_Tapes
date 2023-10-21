/* m-altos	Altos 3068 Unix System V Release 2
   Copyright (C) 1985, 1986 Richard M. Stallman.

This file is part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Emacs General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
GNU Emacs General Public License.   A copy of this license is
supposed to have been given to you along with GNU Emacs so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */


/* The following three symbols give information on
 the size of various data types.  */

#define SHORTBITS 16		/* Number of bits in a short */

#define INTBITS 32		/* Number of bits in an int */

#define LONGBITS 32		/* Number of bits in a long */

/* Vax is not big-endian: lowest numbered byte is least significant,
   but 68000's are. */

#define BIG_ENDIAN

/* Define how to take a char and sign-extend into an int.
   On machines where char is signed, this is a no-op.  */

#define SIGN_EXTEND_CHAR(c) (c)

#define EXPLICIT_SIGN_EXTEND

/* Use type int rather than a union, to represent Lisp_Object */

#define NO_UNION_TYPE

#define LIB_STANDARD -lc
#define C_ALLOCA		/* we have -lPW and alloca but it's broken!
				   <vsedev!ron> */
#define SWITCH_ENUM_BUG

#define NO_REMAP
#define STACK_DIRECTION -1

#define TERMINFO

#undef CANNOT_DUMP
#undef SHORTNAMES
#undef TERMCAP
