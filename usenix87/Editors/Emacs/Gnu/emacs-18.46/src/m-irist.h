/* m- file for Silicon Graphics Iris 2500 Turbos;
   also possibly for non-turbo Irises with system release 2.5.
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


#if 0
  Message-Id: <8704150216.AA21012@orville.arpa>
  Subject: 18.41 on iris 3.5 turbos
  Date: 14 Apr 87 18:16:41 PST (Tue)
  From: raible@orville.arpa

  I have 18.41 working fairly well on the IRIS - everything except for
  asynchronous subprocesses work just fine.  They fail in a strange manner:
  about 1/10 of the time when making a new process (i.e. with 'shell'),
  emacs receives a SIGIOT.  This has the effect of interfering with
  terminal IO, redisplay, the minibuffer, and meta characters so badly
  you have to do 'ESC-x kill-emacs' (^X^C doesn't work).  I hacked
  around this by ignoring SIGIOT in process.c, which make it all useable,
  but a little ugly.  Do either of you know where SIGIOT comes from?

  Message-Id: <8705050653.AA20004@orville.arpa>
  Subject: gnu emacs 18.41 on iris [23].5 machines
  Date: 04 May 87 23:53:11 PDT (Mon)
  From: raible@orville.arpa

  Aside from the SIGIOT, I know of only one bug, a real strange one:
  I wrote a utimes interface, which copies elements from timevals
  to utimbufs. This code is known good.  The problem is that in
  emacs, the utime doesn't seem to take effect (i.e. doesn't change the
  dates at all) unless I call report_file_error *after* the utime returns!

    if (utime (name, &utb) < 0)
      return;
    else
      /* XXX XXX XXX */
      /* For some reason, if this is taken out, then the utime above breaks! */
      /* (i.e. it doesn't set the time. This just makes no sense... */
      /* Eric - May 4, 1987 */
      report_file_error ("Worked just find\n", Qnil);

  Without any sort of debugger that works on emacs (I know... but I don't have
  *time* right now to start with gdb), it was quite time consuming to track
  it down to this.

  But since this code is only used for an optional 4th argument to one command
  (copy-file), it would say that it is non-critical...
#endif /* 0 */

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

/* #define NO_ARG_ARRAY */

/* Define WORD_MACHINE if addresses and such have
 * to be corrected before they can be used as byte counts.  */

/* #define WORD_MACHINE */

/* Define how to take a char and sign-extend into an int.
   On machines where char is signed, this is a no-op.  */

#define SIGN_EXTEND_CHAR(c) (c)

/* Now define a symbol for the cpu type, if your compiler
   does not define it automatically:
   Ones defined so far include vax, m68000, ns16000, pyramid,
   orion, tahoe, APOLLO and many others */

#ifndef m68000
#define m68000
#endif

/* Use type int rather than a union, to represent Lisp_Object */
/* This is desirable for most machines.  */

#define NO_UNION_TYPE

/* Define EXPLICIT_SIGN_EXTEND if XINT must explicitly sign-extend
   the 24-bit bit field into an int.  In other words, if bit fields
   are always unsigned.

   If you use NO_UNION_TYPE, this flag does not matter.  */

#define EXPLICIT_SIGN_EXTEND

/* Data type of load average, as read out of kmem.  */

#define LOAD_AVE_TYPE long

/* Convert that into an integer that is 100 for a load average of 1.0  */

#define FSCALE 1.0
#define LOAD_AVE_CVT(x) (int) (((double) (x)) * 100.0 / FSCALE)

/* Define CANNOT_DUMP on machines where unexec does not work.
   Then the function dump-emacs will not be defined
   and temacs will do (load "loadup") automatically unless told otherwise.  */

/* #define CANNOT_DUMP */

/* Define VIRT_ADDR_VARIES if the virtual addresses of
   pure and impure space as loaded can vary, and even their
   relative order cannot be relied on.

   Otherwise Emacs assumes that data space precedes text space,
   numerically.  */

/* #define VIRT_ADDR_VARIES */

/* Define C_ALLOCA if this machine does not support a true alloca
   and the one written in C should be used instead.
   Define HAVE_ALLOCA to say that the system provides a properly
   working alloca function and it should be used.
   Define neither one if an assembler-language alloca
   in the file alloca.s should be used.  */

/* #define C_ALLOCA */
#define HAVE_ALLOCA

/* Define NO_REMAP if memory segmentation makes it not work well
   to change the boundary between the text section and data section
   when Emacs is dumped.  If you define this, the preloaded Lisp
   code will not be sharable; but that's better than failing completely.  */

/* #define NO_REMAP */
