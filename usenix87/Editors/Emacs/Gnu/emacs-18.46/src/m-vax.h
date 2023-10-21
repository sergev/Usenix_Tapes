/* m- file for vax.
   Copyright (C) 1985, 1986 Free Software Foundation, Inc.

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

/* Vax is not big-endian: lowest numbered byte is least significant. */

/* #undef BIG_ENDIAN */

/* Define how to take a char and sign-extend into an int.
   On machines where char is signed, this is a no-op.  */

#define SIGN_EXTEND_CHAR(c) (c)

/* #define vax    -- appears to be done automatically  */

/* Use type int rather than a union, to represent Lisp_Object */

#define NO_UNION_TYPE

/* crt0.c should use the vax-bsd style of entry, with no dummy args.  */

#define CRT0_DUMMIES

/* crt0.c should define a symbol `start' and do .globl with a dot.  */

#define DOT_GLOBAL_START

#ifdef BSD
/* USG systems I know of running on Vaxes do not actually
   support the load average, so disable it for them.  */

/* Data type of load average, as read out of kmem.  */

#define LOAD_AVE_TYPE double

/* Convert that into an integer that is 100 for a load average of 1.0  */

#define LOAD_AVE_CVT(x) ((int) ((x) * 100.0))

#endif /* BSD */

#ifdef VMS

/* Data type of load average, as read out of driver.  */

#define LOAD_AVE_TYPE float

/* Convert that into an integer that is 100 for a load average of 1.0  */

#define LOAD_AVE_CVT(x) ((int) ((x) * 100.0))

#endif /* VMS */

/* Vax sysV has alloca in the PW library.  */

#ifdef USG
#define LIB_STANDARD -lPW -lc
#define HAVE_ALLOCA

/* There is some bug in unexec in for usg 5.2 on a vax
   which nobody who runs such a system has yet tracked down. */
#ifndef USG5_0
#define NO_REMAP
#endif /* USG 5_0 */

#define TEXT_START 0
#endif /* USG */

#ifdef BSD
#define HAVE_ALLOCA
#endif /* BSD */

#ifdef VMS
#define C_ALLOCA
#endif
