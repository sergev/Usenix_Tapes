/* Definitions for data structures callers pass the regex library.
   Copyright (C) 1985 Free Software Foundation, Inc.

		       NO WARRANTY

  BECAUSE THIS PROGRAM IS LICENSED FREE OF CHARGE, WE PROVIDE ABSOLUTELY
NO WARRANTY, TO THE EXTENT PERMITTED BY APPLICABLE STATE LAW.  EXCEPT
WHEN OTHERWISE STATED IN WRITING, FREE SOFTWARE FOUNDATION, INC,
RICHARD M. STALLMAN AND/OR OTHER PARTIES PROVIDE THIS PROGRAM "AS IS"
WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY
AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM PROVE
DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR
CORRECTION.

 IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW WILL RICHARD M.
STALLMAN, THE FREE SOFTWARE FOUNDATION, INC., AND/OR ANY OTHER PARTY
WHO MAY MODIFY AND REDISTRIBUTE THIS PROGRAM AS PERMITTED BELOW, BE
LIABLE TO YOU FOR DAMAGES, INCLUDING ANY LOST PROFITS, LOST MONIES, OR
OTHER SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE
USE OR INABILITY TO USE (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR
DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY THIRD PARTIES OR
A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS) THIS
PROGRAM, EVEN IF YOU HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES, OR FOR ANY CLAIM BY ANY OTHER PARTY.

		GENERAL PUBLIC LICENSE TO COPY

  1. You may copy and distribute verbatim copies of this source file
as you receive it, in any medium, provided that you conspicuously and
appropriately publish on each copy a valid copyright notice "Copyright
(C) 1985 Free Software Foundation, Inc."; and include following the
copyright notice a verbatim copy of the above disclaimer of warranty
and of this License.  You may charge a distribution fee for the
physical act of transferring a copy.

  2. You may modify your copy or copies of this source file or
any portion of it, and copy and distribute such modifications under
the terms of Paragraph 1 above, provided that you also do the following:

    a) cause the modified files to carry prominent notices stating
    that you changed the files and the date of any change; and

    b) cause the whole of any work that you distribute or publish,
    that in whole or in part contains or is a derivative of this
    program or any part thereof, to be licensed at no charge to all
    third parties on terms identical to those contained in this
    License Agreement (except that you may choose to grant more
    extensive warranty protection to third parties, at your option).

    c) You may charge a distribution fee for the physical act of
    transferring a copy, and you may at your option offer warranty
    protection in exchange for a fee.

  3. You may copy and distribute this program or any portion of it in
compiled, executable or object code form under the terms of Paragraphs
1 and 2 above provided that you do the following:

    a) cause each such copy to be accompanied by the
    corresponding machine-readable source code, which must
    be distributed under the terms of Paragraphs 1 and 2 above; or,

    b) cause each such copy to be accompanied by a
    written offer, with no time limit, to give any third party
    free (except for a nominal shipping charge) a machine readable
    copy of the corresponding source code, to be distributed
    under the terms of Paragraphs 1 and 2 above; or,

    c) in the case of a recipient of this program in compiled, executable
    or object code form (without the corresponding source code) you
    shall cause copies you distribute to be accompanied by a copy
    of the written offer of source code which you received along
    with the copy you received.

  4. You may not copy, sublicense, distribute or transfer this program
except as expressly provided under this License Agreement.  Any attempt
otherwise to copy, sublicense, distribute or transfer this program is void and
your rights to use the program under this License agreement shall be
automatically terminated.  However, parties who have received computer
software programs from you with this License Agreement will not have
their licenses terminated so long as such parties remain in full compliance.

  5. If you wish to incorporate parts of this program into other free
programs whose distribution conditions are different, write to the Free
Software Foundation at 1000 Mass Ave, Cambridge, MA 02138.  We have not yet
worked out a simple rule that can be stated here, but we will often permit
this.  We will be guided by the two goals of preserving the free status of
all derivatives our free software and of promoting the sharing and reuse of
software.


In other words, you are welcome to use, share and improve this program.
You are forbidden to forbid anyone else to use, share and improve
what you give them.   Help stamp out software-hoarding!  */

/*
Modified on 6-Jan-88 by K. E. Gorlen

Add C++ -style declarations for extern functions.
*/

#ifndef RE_NREGS
#define RE_NREGS 10
#endif

/* This data structure is used to represent a compiled pattern. */

struct re_pattern_buffer
  {
    char *buffer;	/* Space holding the compiled pattern commands. */
    int allocated;	/* Size of space that  buffer  points to */
    int used;		/* Length of portion of buffer actually occupied */
    char *fastmap;	/* Pointer to fastmap, if any, or zero if none. */
			/* re_search uses the fastmap, if there is one,
			   to skip quickly over totally implausible characters */
    char *translate;	/* Translate table to apply to all characters before comparing.
			   Or zero for no translation.
			   The translation is applied to a pattern when it is compiled
			   and to data when it is matched. */
    char fastmap_accurate;
			/* Set to zero when a new pattern is stored,
			   set to one when the fastmap is updated from it. */
    char can_be_null;   /* Set to one by compiling fastmap
			   if this pattern might match the null string.
			   It does not necessarily match the null string
			   in that case, but if this is zero, it cannot.
			   2 as value means can match null string
			   but at end of range or before a character
			   listed in the fastmap.  */
  };

/* Structure to store "register" contents data in.

   Pass the address of such a structure as an argument to re_match, etc.,
   if you want this information back.

   start[i] and end[i] record the string matched by \( ... \) grouping i,
   for i from 1 to RE_NREGS - 1.
   start[0] and end[0] record the entire string matched. */

struct re_registers
  {
    int start[RE_NREGS];
    int end[RE_NREGS];
  };

/* These are the command codes that appear in compiled regular expressions, one per byte.
  Some command codes are followed by argument bytes.
  A command code can specify any interpretation whatever for its arguments.
  Zero-bytes may appear in the compiled regular expression. */

enum regexpcode
  {
    unused,
    exactn,    /* followed by one byte giving n, and then by n literal bytes */
    begline,   /* fails unless at beginning of line */
    endline,   /* fails unless at end of line */
    jump,	 /* followed by two bytes giving relative address to jump to */
    on_failure_jump,	 /* followed by two bytes giving relative address of place
		            to resume at in case of failure. */
    finalize_jump,	 /* Throw away latest failure point and then jump to address. */
    maybe_finalize_jump, /* Like jump but finalize if safe to do so.
			    This is used to jump back to the beginning
			    of a repeat.  If the command that follows
			    this jump is clearly incompatible with the
			    one at the beginning of the repeat, such that
			    we can be sure that there is no use backtracking
			    out of repetitions already completed,
			    then we finalize. */
    dummy_failure_jump,  /* jump, and push a dummy failure point.
			    This failure point will be thrown away
			    if an attempt is made to use it for a failure.
			    A + construct makes this before the first repeat.  */
    anychar,	 /* matches any one character */
    charset,     /* matches any one char belonging to specified set.
		    First following byte is # bitmap bytes.
		    Then come bytes for a bit-map saying which chars are in.
		    Bits in each byte are ordered low-bit-first.
		    A character is in the set if its bit is 1.
		    A character too large to have a bit in the map
		    is automatically not in the set */
    charset_not, /* similar but match any character that is NOT one of those specified */
    start_memory, /* starts remembering the text that is matched
		    and stores it in a memory register.
		    followed by one byte containing the register number.
		    Register numbers must be in the range 0 through NREGS. */
    stop_memory, /* stops remembering the text that is matched
		    and stores it in a memory register.
		    followed by one byte containing the register number.
		    Register numbers must be in the range 0 through NREGS. */
    duplicate,    /* match a duplicate of something remembered.
		    Followed by one byte containing the index of the memory register. */
    before_dot,	 /* Succeeds if before dot */
    at_dot,	 /* Succeeds if at dot */
    after_dot,	 /* Succeeds if after dot */
    begbuf,      /* Succeeds if at beginning of buffer */
    endbuf,      /* Succeeds if at end of buffer */
    wordchar,    /* Matches any word-constituent character */
    notwordchar, /* Matches any char that is not a word-constituent */
    wordbeg,	 /* Succeeds if at word beginning */
    wordend,	 /* Succeeds if at word end */
    wordbound,   /* Succeeds if at a word boundary */
    notwordbound, /* Succeeds if not at a word boundary */
    syntaxspec,  /* Matches any character whose syntax is specified.
		    followed by a byte which contains a syntax code, Sword or such like */
    notsyntaxspec /* Matches any character whose syntax differs from the specified. */
  };


#ifndef c_plusplus
extern char *re_compile_pattern ();
/* Is this really advertised? */
extern void re_compile_fastmap ();
extern int re_search (), re_search_2 ();
extern int re_match (), re_match_2 ();
#else
extern const char* re_compile_pattern(const char*, int size, struct re_pattern_buffer*);
extern void re_compile_fastmap(struct re_pattern_buffer);
extern int re_search(struct re_pattern_buffer*,	const char*, int size, int startpos, int range, struct re_registers*);
extern int re_search_2(struct re_pattern_buffer*, const char*, int size1, const char*, int size2, int startpos, int range, struct re_registers*, int mstop);
extern int re_match(struct re_pattern_buffer*, const char*, int size, int pos, struct re_registers*);
extern int re_match_2(struct re_pattern_buffer*, const char* string1, int size1, const char* string2, int size2, int pos, struct re_registers*, int mstop);
#endif

/* 4.2 bsd compatibility (yuck) */
extern char *re_comp ();
extern int re_exec ();

#ifdef SYNTAX_TABLE
extern char *re_syntax_table;
#endif
