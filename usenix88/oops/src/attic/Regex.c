/* Regex.c  -- implementation of OOPS class Regex

	THIS SOFTWARE FITS THE DESCRIPTION IN THE U.S. COPYRIGHT ACT OF A
	"UNITED STATES GOVERNMENT WORK".  IT WAS WRITTEN AS A PART OF THE
	AUTHOR'S OFFICIAL DUTIES AS A GOVERNMENT EMPLOYEE.  THIS MEANS IT
	CANNOT BE COPYRIGHTED.  THIS SOFTWARE IS FREELY AVAILABLE TO THE
	PUBLIC FOR USE WITHOUT A COPYRIGHT NOTICE, AND THERE ARE NO
	RESTRICTIONS ON ITS USE, NOW OR SUBSEQUENTLY.

Author:
        C. J. Eppich
	Bg. 12A, Rm. 2025
	Computer Systems Laboratory
	Division of Computer Research and Technology
	National Institutes of Health
	Bethesda, Maryland 20892
	Phone: (301) 496-5361
	uucp: {decvax!}seismo!elsie!cecil!connie
	September, 1987

Function:

Regex is a class derived from String and containing a regular expression and 
its compiled form.  It implements functions that search Strings for and 
match Strings with regular expressions.  These functions rely heavily on
code taken from GNU Emacs Version 18.41, regex.c, in particular the
re_compile_pattern, re_compile_fastmap, re_match_2, and re_search_2 function
(rewritten for use with class Regex as protected functions re_compile_pattern,
re_compile_fastmap, re_match, and re_search, respectively).  Note that when
a Regex is printed, only the String portion is printed, not the compiled form.
	
Modification History:

*/

#include "Regex.h"
#include "oopsIO.h"

#define	THIS	Regex
#define	BASE	String
DEFINE_CLASS(Regex,String,1,NULL,NULL);

extern const int OOPS_BADREGEX, OOPS_REGEXERR;

#define NFAILURES 80
/* Define how to take a char and sign-extend into an int.
   On machines where char is signed, this is a no-op.  */

#if mc300|sun
#define SIGN_EXTEND_CHAR(x)  (x)
#else if ibm032
#define SIGN_EXTEND_CHAR(c) (((c) & 0x80) ? ((c) | 0xffffff80) : (c))
#endif

// The following macros are used by re_compile_pattern().

#define INIT_ALLOC  28
#define STACKSIZE  40        /* This must be a multiple of 4. Divide by 4 to 
			      obtain the number of levels of nesting allowed
			      for a Regex. */

#define PATPUSH(ch) (*b++ = (char) (ch))

#define PATFETCH(c) \
 {if (p == pend) errRegex("Premature end of regular expression"); \
  c = * (unsigned char *) p++; }

#define PATUNFETCH p--

#define EXTEND_BUFFER \
  { char *old_buffer = cmpexp.buffer; \
    if (cmpexp.allocated == (1<<16)) errRegex("Regular expression too big"); \
    cmpexp.allocated *= 2; \
    if (cmpexp.allocated > (1<<16)) cmpexp.allocated = (1<<16); \
    cmpexp.buffer = (char *) realloc (cmpexp.buffer, cmpexp.allocated);\
    c = cmpexp.buffer - old_buffer; \
    b += c; \
    if (fixup_jump) \
      fixup_jump += c; \
    if (laststart) \
      laststart += c; \
    begalt += c; \
    if (pending_exact) \
      pending_exact += c; \
  }

extern int bcmp(char*,char*,int);
extern char *alloca(int);

//======= Non-member functions:

inline void storeJump(char* from, char opcode, char* to)
/* Store where `from' points a jump operation to jump to where `to' points.
  `opcode' is the opcode to store. */
{ 
    from[0] = opcode;
    from[1] = (to - (from + 3)) & 0377;
    from[2] = (to - (from + 3)) >> 8; 
}
    
void insertJump (char op,char* from,char* to,char* current_end)
/* Open up space at char FROM, and insert there a jump to TO.
   CURRENT_END gives te end of the storage no in use,
   so we know how much data to copy up.
   OP is the opcode of the jump to insert.
   If you call this function, you must zero out pending_exact.  */
{
  register char *pto = current_end + 3;
  register char *pfrom = current_end;
  while (pfrom != from)
    *--pto = *--pfrom;
  storeJump (from, op, to);
}


//======= private functions:

void Regex::cmpexp_deepenShallowCopy()
// Deepen shallow copy of cmpexp.
{
    register char* oldp = cmpexp.buffer;
    register int i = cmpexp.allocated;
    cmpexp.buffer = malloc(cmpexp.allocated);
    register char* newp = cmpexp.buffer;
    while (i--) *newp++ = *oldp++;
}

void Regex::re_compile_pattern()
/* Takes a regular-expression descriptor string and converts it into a string 
   of byte commands for matching.*/
{
    register const char *p = *(String*)this;
    const char *pend = p + size();
    register unsigned c, c1;
    const char *p1;
    /* address of the count-byte of the most recently inserted "exactn" command.
       This makes it possible to tell whether a new exact-match character
       can be added to that command or requires a new "exactn" command. */
     
    char *pending_exact = 0;

    /* address of the place where a forward-jump should go
       to the end of the containing expression.
       Each alternative of an "or", except the last, ends with a forward-jump
       of this sort. */
    
    char *fixup_jump = 0;
    
    /* address of start of the most recently finished expression.
       This tells postfix * where to find the start of its operand. */

    char *laststart = 0;

    /* In processing a repeat, 1 means zero matches is allowed */

    char zero_times_ok;

    /* In processing a repeat, 1 means many matches is allowed */

    char many_times_ok;

    /* Stack of information saved by \( and restored by \).
       Four stack elements are pushed by each \(:
       First, the value of b.
       Second, the value of fixup_jump.
       Third, the value of regnum.
       Fourth, the value of begalt.  */

    int stackb[STACKSIZE];
    int *stackp = stackb;
    int *stacke = stackb + STACKSIZE;
    int *stackt;

    /* Counts \('s as they are encountered.  Remembered for the matching \),
       where it becomes the "register number" to put in the stop_memory command */

    int regnum = 1;

    cmpexp.fastmap_accurate = 0;

    if (cmpexp.allocated == 0)
    {
	cmpexp.allocated = INIT_ALLOC;
	if (cmpexp.buffer)
	    /* EXTEND_BUFFER loses when cmpexp.allocated is 0 */
	    cmpexp.buffer = (char *) realloc (cmpexp.buffer, INIT_ALLOC);
	else 
	    /* Caller did not allocate a buffer.  Do it for him */
	    cmpexp.buffer = (char *) malloc (INIT_ALLOC);
    }
    char* b = cmpexp.buffer;

    /* address of beginning of regexp, or inside of last \( */
    char *begalt = b;
    
    while (p != pend)
    {
	if (b - cmpexp.buffer > cmpexp.allocated - 10)
 	    /* Note that EXTEND_BUFFER clobbers c */
	    EXTEND_BUFFER;
	PATFETCH (c);
	switch (c)
	{
	  case '$':
	    /* $ means succeed if at end of line, but only in special contexts.
	       If randonly in the middle of a pattern, it is a normal character. */
	    if (p == pend || (*p == '\\' && (p[1] == ')' || p[1] == '|')))
	    {
		PATPUSH (endline);
		break;
	    }
	    goto normal_char;

	  case '^':
	    /* ^ means succeed if at beg of line, but only if no preceding pattern. */
	    if (laststart) goto normal_char;
	    PATPUSH (begline);
	    break;

	  case '*':
	  case '+':
	  case '?':
	    /* If there is no previous pattern, char not special. */
	    if (!laststart)
	    goto normal_char;
	    /* If there is a sequence of repetition chars,
	       collapse it down to equivalent to just one.  */
	    zero_times_ok = 0;
	    many_times_ok = 0;
	    while (1)
	    {
		zero_times_ok |= c != '+';
		many_times_ok |= c != '?';
		if (p == pend)
		break;
		PATFETCH (c);
		if (!(c == '*' || c == '+' || c == '?'))
		{
		    PATUNFETCH;
		    break;
		}
	    }

	    /* Now we know whether 0 matches is allowed,
	       and whether 2 or more matches is allowed.  */
	    if (many_times_ok)
	    {
		/* If more than one repetition is allowed,
		   put in a backward jump at the end.  */
		storeJump (b, maybe_finalize_jump, laststart - 3);
		b += 3;
	    }
	    insertJump (on_failure_jump, laststart, b + 3, b);
	    pending_exact = 0;
	    b += 3;
	    if (!zero_times_ok)
	    {
		/* At least one repetition required: insert before the loop
		   a skip over the initial on-failure-jump instruction */
		insertJump (dummy_failure_jump, laststart, laststart + 6, b);
		b += 3;
	    }
	    break;

	  case '.':
	    laststart = b;
	    PATPUSH (anychar);
	    break;
	    
	  case '[':
	    if (b - cmpexp.buffer
		> cmpexp.allocated - 3 - (1 << BYTEWIDTH) / BYTEWIDTH) 
		/* Note that EXTEND_BUFFER clobbers c */
		EXTEND_BUFFER;
	  laststart = b;
	  if (*p == '^') 
	  {
	      PATPUSH (charset_not);
	      p++;
	  }
	  else
	    PATPUSH (charset);
	  p1 = p;

	  PATPUSH ((1 << BYTEWIDTH) / BYTEWIDTH);
	  /* Clear the whole map */
	  bzero (b, (1 << BYTEWIDTH) / BYTEWIDTH);
	  /* Read in characters and ranges, setting map bits */
	  while (1)
	    {
	      PATFETCH (c);
	      if (c == ']' && p != p1 + 1) break;
	      if (*p == '-')
		{
		  PATFETCH (c1);
		  PATFETCH (c1);
		  while (c <= c1)
		    b[c / BYTEWIDTH] |= 1 << (c % BYTEWIDTH), c++;
		}
	      else
		{
		  b[c / BYTEWIDTH] |= 1 << (c % BYTEWIDTH);
		}
	    }
	  /* Discard any bitmap bytes that are all 0 at the end of the map.
	     Decrement the map-length byte too. */
	  while (b[-1] > 0 && b[b[-1] - 1] == 0)
	    b[-1]--;
	  b += b[-1];
	  break;

	  case '\\':
	    if (p == pend)  errRegex("Invalid regular expression");
	    PATFETCH (c);
	    switch (c)
	    {
	      case '(':
		if (stackp == stacke) errRegex("Nesting too deep");
		if (regnum < RE_NREGS)
	        {
		    PATPUSH (start_memory);
		    PATPUSH (regnum);
	        }
		*stackp++ = b - cmpexp.buffer;
		*stackp++ = fixup_jump ? fixup_jump - cmpexp.buffer + 1 : 0;
		*stackp++ = regnum++;
		*stackp++ = begalt - cmpexp.buffer;
		fixup_jump = 0;
		laststart = 0;
		begalt = b;
		break;
		
	      case ')':
		if (stackp == stackb)  errRegex("Unmatched \\)");
		begalt = *--stackp + cmpexp.buffer;
		if (fixup_jump)
		storeJump (fixup_jump, jump, b);
		if (stackp[-1] < RE_NREGS)
		{
		    PATPUSH (stop_memory);
		    PATPUSH (stackp[-1]);
		}
		stackp -= 2;
		fixup_jump = 0;
		if (*stackp)
		fixup_jump = *stackp + cmpexp.buffer - 1;
		laststart = *--stackp + cmpexp.buffer;
		break;
		
	      case '|':
		insertJump (on_failure_jump, begalt, b + 6, b);
		pending_exact = 0;
		b += 3;
		if (fixup_jump)
		storeJump (fixup_jump, jump, b);
		fixup_jump = b;
		b += 3;
		laststart = 0;
		begalt = b;
		break;

	      case '1':
	      case '2':
	      case '3':
	      case '4':
	      case '5':
	      case '6':
	      case '7':
	      case '8':
	      case '9':
		c1 = c - '0';
		if (c1 >= regnum)
		goto normal_char;
		for (stackt = stackp - 2;  stackt > stackb;  stackt -= 4)
 		if (*stackt == c1)
		goto normal_char;
		laststart = b;
		PATPUSH (duplicate);
		PATPUSH (c1);
	      break;
	      default:
		/* You might think it wuld be useful for \ to mean
		   not to translate; but if we don't translate it
		   it will never match anything.  */
		goto normal_char;
	    }
	    break;
	    
	  default:
	  normal_char:
	    if (!pending_exact || pending_exact + *pending_exact + 1 != b
		|| *pending_exact == 0177 || *p == '*' || *p == '^'
		|| *p == '+' || *p == '?')
	    {
		laststart = b;
		PATPUSH (exactn);
		pending_exact = b;
		PATPUSH (0);
	    }
	    PATPUSH (c);
	    (*pending_exact)++;
	}
    }

    if (fixup_jump)
    storeJump (fixup_jump, jump, b);

    if (stackp != stackb) errRegex("Unmatched \\(");

    cmpexp.used = b - cmpexp.buffer;
    PATPUSH('\0');
}

void Regex::re_compile_fastmap ()
/* Compute a fastmap for this Regex.
 The fastmap records which of the (1 << BYTEWIDTH) possible characters
 can start a string that matches the pattern.
 This fastmap is used by re_search to skip quickly over totally implausible 
text. */
{
  unsigned char *pattern = (unsigned char *) cmpexp.buffer;
  int size = cmpexp.used;
  register char *fastmap = cmpexp.fastmap;
  register unsigned char *p = pattern;
  register unsigned char *pend = pattern + size;
  register int j;

  unsigned char *stackb[NFAILURES];
  unsigned char **stackp = stackb;

  bzero (fastmap, (1<<BYTEWIDTH));

  cmpexp.fastmap_accurate = 1;
  cmpexp.can_be_null = 0;

  while (p)
    {
      if (p == pend)
	{
	  cmpexp.can_be_null = 1;
	  break;
	}

      switch ((enum regexpcode) *p++)
	{
	case exactn:
	  fastmap[p[1]] = 1;
	  break;

        case begline:
	  continue;

	case endline:
	  fastmap['\n'] = 1;
	  if (cmpexp.can_be_null != 1)
	    cmpexp.can_be_null = 2;
	  break;

	case finalize_jump:
	case maybe_finalize_jump:
	case jump:
	case dummy_failure_jump:
	  cmpexp.can_be_null = 1;
	  j = *p++ & 0377;
	  j += SIGN_EXTEND_CHAR (*(char *)p) << 8;
	  p += j + 1;		/* The 1 compensates for missing ++ above */
	  if (j > 0)
	    continue;
	  /* Jump backward reached implies we just went through
	     the body of a loop and matched nothing.
	     Opcode jumped to should be an on_failure_jump.
	     Just treat it like an ordinary jump.
	     For a * loop, it has pushed its failure point already;
	     if so, discard that as redundant.  */
	  if ((enum regexpcode) *p != on_failure_jump)
	    continue;
	  p++;
	  j = *p++ & 0377;
	  j += SIGN_EXTEND_CHAR (*(char *)p) << 8;
	  p += j + 1;		/* The 1 compensates for missing ++ above */
	  if (stackp != stackb && *stackp == p)
	    stackp--;
	  continue;
	  
	case on_failure_jump:
	  j = *p++ & 0377;
	  j += SIGN_EXTEND_CHAR (*(char *)p) << 8;
	  p++;
	  *++stackp = p + j;
	  continue;

	case start_memory:

	case stop_memory:
	  p++;
	  continue;

	case duplicate:
	  cmpexp.can_be_null = 1;
	  fastmap['\n'] = 1;

	case anychar:
	  for (j = 0; j < (1 << BYTEWIDTH); j++)
	    if (j != '\n')
	      fastmap[j] = 1;
	  if (cmpexp.can_be_null)
	    return;
	  /* Don't return; check the alternative paths
	     so we can set can_be_null if appropriate.  */
	  break;

	case charset:
	  for (j = *p++ * BYTEWIDTH - 1; j >= 0; j--)
	    if (p[j / BYTEWIDTH] & (1 << (j % BYTEWIDTH)))
	      {
		  fastmap[j] = 1;
	      }
	  break;

	case charset_not:
	  /* Chars beyond end of map must be allowed */
	  for (j = *p * BYTEWIDTH; j < (1 << BYTEWIDTH); j++)
	      fastmap[j] = 1;

	  for (j = *p++ * BYTEWIDTH - 1; j >= 0; j--)
	    if (!(p[j / BYTEWIDTH] & (1 << (j % BYTEWIDTH))))
	      {
		  fastmap[j] = 1;
	      }
	  break;
	}

      /* Get here means we have successfully found the possible starting characters
	 of one path of the pattern.  We need not follow this path any farther.
	 Instead, look at the next alternative remembered in the stack. */
      if (stackp != stackb)
	p = *stackp--;
      else
	break;
    }
}



int Regex::re_match(const String& str, int pos, regPtTy regs, int mstop)
/* Match the pattern described by *this against data string. 
  Start the match at position `pos'.
  Do not consider matching past the position `mstop'.  (This does not work
  as advertised.)

  -1 is returned if there is no match.  Otherwise the value is the length
  of the substring which was matched.
*/
{
    char *string = &str[0];
    register char *p = cmpexp.buffer;
    register char *pend = p + cmpexp.used;
    /* Pointer to end of data string. */
    char *dend = string + str.length();
    /* Pointer just past last char to consider matching */
    char *end_match;
    register char *d;
    register int mcnt;

    /* Failure point stack.  Each place that can handle a failure further down the line
       pushes a failure point on this stack.  It consists of two char *'s.
       The first one pushed is where to resume scanning the pattern;
       the second pushed is where to resume scanning the strings.
       If the latter is zero, the failure point is a "dummy".
       If a failure happens and the innermost failure point is dormant,
       it discards that failure point and tries the next one. */

    char **stackb = (char **) malloc (2 * NFAILURES * sizeof (char *));
    char **stackp = stackb, **stacke = &stackb[2 * NFAILURES];

    /* Information on the "contents" of registers.
       These are pointers into the input strings; they record
       just what was matched (on this attempt) by some part of the pattern.
       The start_memory command stores the start of a register's contents
       and the stop_memory command stores the end.

       At that point, regstart[regnum] points to the first character in the register,
       regend[regnum] points to the first character beyond the end of the register.*/

    char *regstart[RE_NREGS];
    char *regend[RE_NREGS];

    /* Compute where to stop matching.*/
    if (mstop < str.length())  end_match = string + mstop;
    else    end_match = dend;

    /* Initialize \( and \) text positions to -1
       to mark ones that no \( or \) has been seen for.  */

    for (mcnt = 0; mcnt < sizeof (regstart) / sizeof (*regstart); mcnt++)
    {
	regstart[mcnt] = (char *) -1;
	regend[mcnt] = (char *) -1;
    }

    /* `p' scans through the pattern as `d' scans through the data.*/

    if (pos < str.length())
      d = string + pos;
    else errArgs("re_match()", "Starting position is out of string range.");

    /* Write PREFETCH; just before fetching a character with *d.  */
#define PREFETCH if (d==end_match)  goto fail; /*end of string => failure*/

    /* This loop loops over pattern commands.
       It exits by returning from the function if match is complete,
       or it drops through if match fails at this starting point in the input data. */

    while (1)
    {
	if (p == pend)
	/* End of pattern means we have succeeded! */
	{
	    /* If caller wants register contents data back, convert it to indices */
	    if (regs)
	    {
		regend[0] = d;
		regstart[0] = string;
		for (mcnt = 0; mcnt < RE_NREGS; mcnt++)
		{
		    if ((mcnt != 0) && regstart[mcnt] == (char *) -1)
		    {
			regs->start[mcnt] = -1;
			regs->end[mcnt] = -1;
			continue;
		    }
		    regs->start[mcnt] = regstart[mcnt] - string;
		    if (regend[mcnt] != (char *) -1)  
		        regs->end[mcnt] = regend[mcnt] - string;
		    else regs->end[mcnt] = regs->start[mcnt];
		}
		regs->start[0] = pos;
	    }
//	    free((char*)stackb);
	    return d - string - pos;
	}

      /* Otherwise match next pattern command */
      switch ((enum regexpcode) *p++)
      {

	/* \( is represented by a start_memory, \) by a stop_memory.
	    Both of those commands contain a "register number" argument.
	    The text matched within the \( and \) is recorded under that number.
	    Then, \<digit> turns into a `duplicate' command which
	    is followed by the numeric value of <digit> as the register number. */

	case start_memory:
	  regstart[*p++] = d;
	  break;

	case stop_memory:
	  regend[*p++] = d;
	  break;

	case duplicate:
	{
	    int regno = *p++;   /* Get which register to match against */

	    /* mcnt gets # consecutive chars to compare */

	    /* Make sure there are enough characters left in input string.*/
	    if ((mcnt = regend[regno] - regstart[regno]) > end_match - d) goto fail;

	    /* Compare that many; failure if mismatch, else skip them. */
	    if (bcmp (d, regstart[regno], mcnt))  goto fail;
	    d += mcnt;
	}
	    break;

	case anychar:
	  /* fetch a data character */
	  PREFETCH;
	  /* Match anything but a newline.  */
	  if ((*d++) == '\n')
	    goto fail;
	  break;

	case charset:
	case charset_not:
	  {
	    /* Nonzero for charset_not */
	    int not = 0;
	    register int c;
	    if (*(p - 1) == (char) charset_not)
	      not = 1;

	    /* fetch a data character */
	    PREFETCH;
	    c = *(unsigned char *)d;

	    if (c < *p * BYTEWIDTH
		&& p[1 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
	      not = !not;

	    p += 1 + *p;

	    if (!not) goto fail;
	    d++;
	    break;
	  }

	case begline:
	  if (d == string || d[-1] == '\n')
	    break;
	  goto fail;

	case endline:
	  if ((d == dend) || (*d == '\n'))
	    break;
	  goto fail;

	/* "or" constructs ("|") are handled by starting each alternative
	    with an on_failure_jump that points to the start of the next alternative.
	    Each alternative except the last ends with a jump to the joining point.
	    (Actually, each jump except for the last one really jumps
	     to the following jump, because tensioning the jumps is a hassle.) */

	/* The start of a stupid repeat has an on_failure_jump that points
	   past the end of the repeat text.
	   This makes a failure point so that, on failure to match a repetition,
	   matching restarts past as many repetitions have been found
	   with no way to fail and look for another one.  */

	/* A smart repeat is similar but loops back to the on_failure_jump
	   so that each repetition makes another failure point. */

	case on_failure_jump:
	  if (stackp == stacke)
	  {
	      long stack_size = stacke - stackb;
	      stackb = (char **) realloc ((char*)stackb, 2 * stack_size * sizeof (char *));
	      stackp = stackb + stack_size;
	      stacke = stackb + (2 * stack_size);
	    }
	  mcnt = *p++ & 0377;
	  mcnt += SIGN_EXTEND_CHAR (*p) << 8;
	  p++;
	  *stackp++ = mcnt + p;
	  *stackp++ = d;
	  break;

	/* The end of a smart repeat has an maybe_finalize_jump back.
	   Change it either to a finalize_jump or an ordinary jump. */

	case maybe_finalize_jump:
	  mcnt = *p++ & 0377;
	  mcnt += SIGN_EXTEND_CHAR (*p) << 8;
	  p++;
	  /* Compare what follows with the begining of the repeat.
	     If we can establish that there is nothing that they would
	     both match, we can change to finalize_jump */
	  if (p == pend)
	    p[-3] = (char) finalize_jump;
	  else if (*p == (char) exactn || *p == (char) endline)
	    {
	      register int c = *p == (char) endline ? '\n' : p[2];
	      register char *p1 = p + mcnt;
	      /* p1[0] ... p1[2] are an on_failure_jump.
		 Examine what follows that */
	      if (p1[3] == (char) exactn && p1[5] != c)
		p[-3] = (char) finalize_jump;
	      else if (p1[3] == (char) charset || p1[3] == (char) charset_not)
		{
		  int not = p1[3] == (char) charset_not;
		  if (c < p1[4] * BYTEWIDTH
		      && p1[5 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
		    not = !not;
		  /* not is 1 if c would match */
		  /* That means it is not safe to finalize */
		  if (!not)
		    p[-3] = (char) finalize_jump;
		}
	    }
	  p -= 2;
	  if (p[-1] != (char) finalize_jump)
	    {
	      p[-1] = (char) jump;
	      goto nofinalize;
	    }

	/* The end of a stupid repeat has a finalize-jump
	   back to the start, where another failure point will be made
	   which will point after all the repetitions found so far. */

	case finalize_jump:
	  stackp -= 2;

	case jump:
	nofinalize:
	  mcnt = *p++ & 0377;
	  mcnt += SIGN_EXTEND_CHAR (*p) << 8;
	  p += mcnt + 1;	/* The 1 compensates for missing ++ above */
	  break;

	case dummy_failure_jump:
	  if (stackp == stacke)
	    {
	      long stack_size = stacke - stackb;
	      stackb = (char **) realloc ((char*)stackb, 2 * stack_size * sizeof (char *));
	      stackp = stackb + stack_size;
	      stacke = stackb + (2 * stack_size);
	    }
	  *stackp++ = 0;
	  *stackp++ = 0;
	  goto nofinalize;

	case exactn:
	  /* Match the next few pattern characters exactly.
	     mcnt is how many characters to match. */
	  mcnt = *p++;
	  do
	  {
	      PREFETCH;
	      if (*d++ != *p++) goto fail;
	  }
	  while (--mcnt);

	  break;
	}
      continue;    /* Successfully matched one pattern command; keep matching */

      /* Jump here if any matching operation fails. */
    fail:
      if (stackp != stackb)
	/* A restart point is known.  Restart there and pop it. */
	{
	  if (!stackp[-2])
	    {   /* If innermost failure point is dormant, flush it and keep looking */
	      stackp -= 2;
	      goto fail;
	    }
	  d = *--stackp;
	  p = *--stackp;
	}
      else break;   /* Matching at this starting point really fails! */
    }
//    free((char*)stackb);
    return -1;         /* Failure to match */
}

Range Regex::re_search(const String& str, int range, regPtTy regs, int mstop,
 int startpos)
/* Like re_match but tries first a match starting at index `startpos',
 then at startpos + 1, and so on.
 `range' is the number of places to try before giving up.
 If `range' is negative, the starting positions tried are
  startpos, startpos - 1, etc.
 It is up to the caller to make sure that range is not so large
  as to take the starting position outside of the input strings.
  Do not consider matching past the position 'mstop'.  

The value returned is the range for which the match was found,
 or (0,-1) if no match was found. */
{
    register char *fastmap = cmpexp.fastmap;
    int size = str.size(); 
    int match_length;
    Range r;
    /* Update the fastmap now if not correct already */
    if (!cmpexp.fastmap_accurate)
    re_compile_fastmap ();
    while (1)
    {
	/* Skip quickly over characters that cannot possibly be the start 
	   of a match.
	   Note, however, that if the pattern can possibly match
	   the null string, we must test it at each starting point
	   so that we take the first null string we get.  */

	if (startpos < size && cmpexp.can_be_null != 1)
	{
	    register char *p = &(str)[startpos];
	    if (range > 0)
	    {
		while (range > 0 && !fastmap[*p++])
		{
		    range--;
		    startpos++;
		}
	    }
	    else
	    {
		while (range < 0 && (!fastmap[*p--]))
		{
		    range++;
		    startpos--;
		}
		if (!range) 
		{
		    r(0,-1);
		    return r;
		}
	    }
	}

	if (range >= 0 && startpos == size && cmpexp.can_be_null == 0)
	{
	    r(0,-1);
	    return r;
	}

	if (0 <= (match_length = re_match (str, startpos, regs, mstop)))
	{
	    r(startpos,match_length);
	    return r;
	}

	if (!range) //failure
	{
	    r(0,-1);
	    return r;
	}

	/* else advance */
	if (range > 0)
	{
	    range--;
	    startpos++;
	}
	else  // r < 0
	{
	    range++;
	    startpos--;
	}
    }
}

void Regex::errRegex(char* s)
{
    const char* a = *(String*)this;
    setOOPSerror(OOPS_BADREGEX,DEFAULT,a,s,this);
}

void Regex::errArgs(char* fname,char* desc)
{
    setOOPSerror(OOPS_REGEXERR,DEFAULT,this,fname,desc);
}


//======= constructors:
Regex::Regex(const char* rexp,unsigned oflow) : (rexp,oflow)
{
    init_cmpexp();
    re_compile_pattern();
}

Regex::Regex(char& c, unsigned l, unsigned oflow):   (c,l,oflow)
{
    init_cmpexp();
    re_compile_pattern();
}

Regex::Regex(unsigned oflow):    (oflow)
{
    init_cmpexp();
}

Regex::Regex(const String& rexp,unsigned oflow) : (rexp,oflow)
{
    init_cmpexp();
    re_compile_pattern();
}

Regex::Regex(const SubString& rexp, unsigned oflow) : (rexp,oflow)
{
    init_cmpexp();
    re_compile_pattern();
}

Regex::Regex(const Regex& rexp) : (&rexp[0])
{
    // A copy is more efficient that re_compile_pattern()
    bcopy(&rexp.cmpexp,&cmpexp,sizeof(cmpexp));
    cmpexp_deepenShallowCopy();
}

Regex::Regex(const Regex& rexp, unsigned oflow) : (&rexp[0],oflow)
{
    // A copy is more efficient that re_compile_pattern()
    bcopy(&rexp.cmpexp,&cmpexp,sizeof(cmpexp));
    cmpexp_deepenShallowCopy();
}

//========== public member functions:

void Regex::debug()
{
    register int i;
    printf("Compiled expression:");
    for (i = 0; i<cmpexp.used; i++)
    {
	char c = cmpexp.buffer[i];
	if (c < 041 || c >= 0177)
	{
	    putchar ('\\');
	    putchar (((c >> 6) & 3) + '0');
	    putchar (((c >> 3) & 7) + '0');
	    putchar ((c & 7) + '0');
	}
	else
	  putchar (c);
    }
    putchar('\n');
    printf ("\n%d allocated, %d used.\n", cmpexp.allocated, cmpexp.used);
    printf("Allowed by fastmap: ");
    for (i = 0; i < (1 << BYTEWIDTH); i++)
      if (cmpexp.fastmap[i]) putchar (i);
    putchar ('\n');
}

void Regex::operator=(const char* cs)
{
    *(String*)this=cs;
    re_compile_pattern();
}

void Regex::operator=(const Regex& rexp)
{
    this->String::operator=(rexp);
    free (cmpexp.buffer);
    bcopy(&rexp.cmpexp,&cmpexp,sizeof(cmpexp));
    cmpexp_deepenShallowCopy();
}

void Regex::operator=(const String& str)
{
    *(String*)this=str;
    re_compile_pattern();
}

void Regex::operator=(const SubString& substr)
{
    *(String*)this=substr;
    re_compile_pattern();
}

void Regex::deepenShallowCopy()
// Called by deepCopy() to convert a shallow copy to a deep copy.
{
    String::deepenShallowCopy();
    cmpexp_deepenShallowCopy();
}

void Regex::scanFrom(istream& strm)
{
    String::scanFrom(strm);
    re_compile_pattern();
}

void Regex::toAscii()
{
    String::toAscii();
    re_compile_pattern();
}

void Regex::toLower()
{
    String::toLower();
    re_compile_pattern();
}

void Regex::toUpper()
{
    String::toUpper();
    re_compile_pattern();
}

Regex::Regex(istream& strm, Regex& where) : (strm,where)	
// Construct an object from istream "strm" at address "where".
{
    this = &where;
    init_cmpexp();
    re_compile_pattern();
}

Regex::Regex(fileDescTy& fd, Regex& where) : (fd,where)	
// Construct an object from file descriptor "fd" at address "where".
{
    this = &where;
    init_cmpexp();
    re_compile_pattern();
}


//======== Pattern-matching public member functions:

bool Regex::exactMatch(const String& s)
// Check for exact match between Regex and entire String s.
{
    return re_match(s,0,0,s.length()) == s.length();
}

Range Regex::match(const String& s, int pos)
// Check for match between Regex and index pos of String s.
// Return range of string matched.
{
    Range r(pos,re_match(s,pos,0,s.length()));
    return r;
}	    

bool Regex::match(const String& s, Range& r, int pos)
// Check for match between Regex and index pos of String s.
// r is returned as range of string matched with length of -1 if no match. 
{
    r = match(s,pos);
    return (r.length()!=-1);
}

OrderedCltn* Regex::match(const String& s, OrderedCltn& oc, int pos)
// Check for match between Regex and index pos of String s.
// Return OrderedCltn of Ranges.  First Range will contain string
// matched and subsequent ranges will contain strings matched by each
// regex contained in parens.
{
    regTy regs;
    Range *r;
    if (re_match(s,pos,&regs,s.length()) != -1)
    {
	for (register int i=0;i<RE_NREGS;i++)
	{
	    if ((i==0)||(regs.start[i] != -1))
	    {
		r = new Range(regs.start[i],regs.end[i]-regs.start[i]);
		oc.add(*r);
	    }
	    else  break;
	}
    }
    else
    {
	r = new Range(0,-1);
	oc.add(*r);
    }
    return &oc;
}

bool Regex::search(const String& str, Range& r, int startpos)
{
    r = re_search(str, str.length()-startpos, 0, str.length(), startpos);
    return (r.length() != -1);
}
    
Range Regex::search(const String& str, int startpos)
{
    return re_search(str, str.length()-startpos, 0, str.length(), startpos);
}

OrderedCltn* Regex::search(const String& s, OrderedCltn& oc, int startpos)
// Search String s for Regex starting at startpos.
// Return OrderedCltn of Ranges.  First Range will contain string
// matched and subsequent ranges will contain strings matched by each
// regex contained in parens.
{
    regTy regs;
    Range *r;
    Range t = re_search(s,s.length(),&regs,s.length(),startpos);
    if (t.found())
    {
	for (register int i=0;i<RE_NREGS;i++)
	{
	    if ((i==0)||(regs.start[i] != -1))
	    {
		r = new Range(regs.start[i],regs.end[i]-regs.start[i]);
		oc.add(*r);
	    }
	    else  break;
	}
    }
    return &oc;
}
    
Range Regex::search(const String& str, int range, int mstop, int startpos)
/* This allows more flexibility in the search.  
   'range' is the number of places to try before giving up; a negative value
   will provide a backward search.
   Do not consider matching past the position 'mstop'. */
{
    return (re_search(str, range, 0, mstop, startpos));
}

OrderedCltn* Regex::search(const String& s, OrderedCltn& oc, int range, int mstop, int startpos)
/* Search String s for Regex starting at startpos.
   'range' gives number of positions to try, with a negative value providing
    a backward search.
    Do not consider matching past 'mstop'.
    Return OrderedCltn of Ranges.  First Range will contain string
    matched and subsequent ranges will contain strings matched by each
    regex contained in parens.*/
{
    regTy regs;
    Range *r;
    if (re_search(s,range,&regs,mstop,startpos) != Range(0,-1))
    {
	for (register int i=0;i<RE_NREGS;i++)
	{
	    if ((i==0)||(regs.start[i] != -1))
	    {
		r = new Range(regs.start[i],regs.end[i]-regs.start[i]);
		oc.add(*r);
	    }
	    else  break;
	}
    }
    return &oc;
}

OrderedCltn* Regex::repeatSearch(const String& s, OrderedCltn& oc, int range, int stop, int startpos)
// Search String for successive, nonoverlapping matches with Regex.
// Return OrderedCltn of Ranges where match occurred.
{
    int final_start = range + startpos - 1;  
    Range *r;
    r = new Range(re_search(s, range, 0, stop, startpos));
    while (r->found())
    {
	oc.add(*r);
	startpos = r->firstIndex() + r->length();
	if (startpos > final_start)  break;
	range = final_start - startpos;
	r = new Range(re_search(s,range,0,stop,startpos));
    }
    return &oc;
}


OrderedCltn* Regex::overlapSearch(const String& s, OrderedCltn& oc, int range,
 int stop, int startpos)
// Search String for successive, overlapping matches with Regex.
// Return OrderedCltn of Ranges where match occurred.
{
    int final_start = range + startpos -1;  
    Range *r;
    r = new Range(re_search(s, range, 0, stop, startpos));
    while (r->found())
    {
	oc.add(*r);
	startpos = r->firstIndex() + 1;
	if (startpos > final_start)  break;
	range = final_start - startpos;
	r = new Range(re_search(s,range,0,stop,startpos));
    }
    return &oc;
}
