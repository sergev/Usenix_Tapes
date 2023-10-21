/* m-amdahl_uts file 
   Copyright (C) 1987 Free Software Foundation, Inc.

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

/*
This file for amdahl_uts created by modifying the m-template.h
by Jishnu Mukerji 3/1/87

Use s-usg5-2-2.h with this file.

This file works with the Amdahl uts native C compiler. The 5.2u370
compiler is so brain damaged that it is not even worth trying to use it.
*/

/* The following three symbols give information on
 the size of various data types.  */

#define SHORTBITS 16		/* Number of bits in a short */

#define INTBITS 32		/* Number of bits in an int */

#define LONGBITS 32		/* Number of bits in a long */

/* Define BIG_ENDIAN iff lowest-numbered byte in a word
   is the most significant byte.  */

#define BIG_ENDIAN

/* Define NO_ARG_ARRAY if you cannot take the address of the first of a
 * group of arguments and treat it as an array of the arguments.  */

#undef NO_ARG_ARRAY

/* Define WORD_MACHINE if addresses and such have
 * to be corrected before they can be used as byte counts.  */

#define WORD_MACHINE /* not actually used anywhere yet! */

/* Define how to take a char and sign-extend into an int.
   On machines where char is signed, this is a no-op.  */

#define SIGN_EXTEND_CHAR(c) (((sign_extend_temp=(c)) & 0x80) \
			     ? (sign_extend_temp | 0xFFFFFF00) \
			     : (sign_extend_temp))

/* Now define a symbol for the cpu type, if your compiler
   does not define it automatically:
   vax, m68000, ns16000, pyramid, orion, tahoe and APOLLO
   are the ones defined so far.  */

/* uts gets defined automatically */
/* However for clarity define amdahl_uts */
#define amdahl_uts

/* Use type int rather than a union, to represent Lisp_Object */
/* This is desirable for most machines.  */

#define NO_UNION_TYPE

/* Define EXPLICIT_SIGN_EXTEND if XINT must explicitly sign-extend
   the 24-bit bit field into an int.  In other words, if bit fields
   are always unsigned.

   If you use NO_UNION_TYPE, this flag does not matter.  */

#define EXPLICIT_SIGN_EXTEND

/* Data type of load average, as read out of kmem.  */

/* #define LOAD_AVE_TYPE long*/

/* Convert that into an integer that is 100 for a load average of 1.0  */

/*#define LOAD_AVE_CVT(x) (int) (((double) (x)) * 100.0)*/

/* Define CANNOT_DUMP on machines where unexec does not work.
   Then the function dump-emacs will not be defined
   and temacs will do (load "loadup") automatically unless told otherwise.  */

/* #define CANNOT_DUMP

/* Define VIRT_ADDR_VARIES if the virtual addresses of
   pure and impure space as loaded can vary, and even their
   relative order cannot be relied on.

   Otherwise Emacs assumes that data space precedes text space,
   numerically.  */

/* #define VIRT_ADDR_VARIES*/

/* Define C_ALLOCA if this machine does not support a true alloca
   and the one written in C should be used instead.
   Define HAVE_ALLOCA to say that the system provides a properly
   working alloca function and it should be used.
   Define neither one if an assembler-language alloca
   in the file alloca.s should be used.  */

#define C_ALLOCA
/*#define HAVE_ALLOCA */

#ifdef HAVE_ALLOCA
#define LIB_STANDARD -lPW -lc
#endif

/* Define NO_REMAP if memory segmentation makes it not work well
   to change the boundary between the text section and data section
   when Emacs is dumped.  If you define this, the preloaded Lisp
   code will not be sharable; but that's better than failing completely.  */

/*#define NO_REMAP*/

#define TERMINFO

/* The usual definition of XINT, which involves shifting, does not
   sign-extend properly on this machine.  */

#define XINT(i) (((sign_extend_temp=(i)) & 0x00800000) \
		 ? (sign_extend_temp | 0xFF000000) \
		 : (sign_extend_temp & 0x00FFFFFF))

#ifdef emacs /* Don't do this when making xmakefile! */
extern int sign_extend_temp;
#endif

/* The following needed to load the proper crt0.o and to get the
   proper declaration of data_start in the #undef NO_REMAP case */

#ifndef NO_REMAP
#define START_FILES pre-crt0.o /lib/crt0.o
#endif

/* Perhaps this means that the optimizer isn't safe to use.  */

#define C_OPTIMIZE_SWITCH

/* Put text and data on non-segment boundary; makes image smaller */

#define LD_SWITCH_MACHINE	-N 

/* When writing the 'xemacs' file, make text segment ro */
#define EXEC_MAGIC	0410

/* Mask for address bits within a memory segment */
#define SEGSIZ 0x10000		/* Should this not be defined elsewhere ? */
#define SEGMENT_MASK (SEGSIZ - 1)
