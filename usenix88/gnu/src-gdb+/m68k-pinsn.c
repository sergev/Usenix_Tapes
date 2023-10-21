/* Print m68k instructions for GDB, the GNU debugger.
   Copyright (C) 1986, 1987 Free Software Foundation, Inc.

GDB is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY.  No author or distributor accepts responsibility to anyone
for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.
Refer to the GDB General Public License for full details.

Everyone is granted permission to copy, modify and redistribute GDB,
but only under the conditions described in the GDB General Public
License.  A copy of this license is supposed to have been given to you
along with GDB so you can know your rights and responsibilities.  It
should be in a file named COPYING.  Among other things, the copyright
notice and this notice must be preserved on all copies.

In other words, go ahead and share GDB, but don't try to stop
anyone else from sharing it farther.  Help stamp out software hoarding!
*/

#include <stdio.h>

#include "defs.h"
#include "param.h"
#include "symtab.h"
#include "m68k-opcode.h"

/* 68k instructions are never longer than this many bytes.  */
#define MAXLEN 22

/* Number of elements in the opcode table.  */
#define NOPCODES (sizeof m68k_opcodes / sizeof m68k_opcodes[0])

extern char *reg_names[];
char *fpcr_names[] = { "", "fpiar", "fpsr", "fpiar/fpsr", "fpcr",
		     "fpiar/fpcr", "fpsr/fpcr", "fpiar-fpcr"};

static unsigned char *print_insn_arg ();
static unsigned char *print_indexed ();
static void print_base ();
static int fetch_arg ();

#define NEXTBYTE(p)  (p += 2, ((char *)p)[-1])

#define NEXTWORD(p)  \
  (p += 2, ((((char *)p)[-2]) << 8) + p[-1])

#define NEXTLONG(p)  \
  (p += 4, (((((p[-4] << 8) + p[-3]) << 8) + p[-2]) << 8) + p[-1])

#define NEXTSINGLE(p) \
  (p += 4, *((float *)(p - 4)))

#define NEXTDOUBLE(p) \
  (p += 8, *((double *)(p - 8)))

#define NEXTEXTEND(p) \
  (p += 12, 0.0)	/* Need a function to convert from extended to double
			   precision... */

#define NEXTPACKED(p) \
  (p += 12, 0.0)	/* Need a function to convert from packed to double
			   precision.   Actually, it's easier to print a
			   packed number than a double anyway, so maybe
			   there should be a special case to handle this... */

/* Print the m68k instruction at address MEMADDR in debugged memory,
   on STREAM.  Returns length of the instruction, in bytes.  */

int
print_insn (memaddr, stream)
     CORE_ADDR memaddr;
     FILE *stream;
{
  unsigned char buffer[MAXLEN];
  register int i;
  register unsigned char *p;
  register char *d;
  register int bestmask;
  int best;

  read_memory (memaddr, buffer, MAXLEN);

  bestmask = 0;
  best = -1;
  for (i = 0; i < NOPCODES; i++)
    {
      register unsigned int opcode = m68k_opcodes[i].opcode;
      register unsigned int match = m68k_opcodes[i].match;
      if (((0xff & buffer[0] & (match >> 24)) == (0xff & (opcode >> 24)))
	  && ((0xff & buffer[1] & (match >> 16)) == (0xff & (opcode >> 16)))
	  && ((0xff & buffer[2] & (match >> 8)) == (0xff & (opcode >> 8)))
	  && ((0xff & buffer[3] & match) == (0xff & opcode)))
	{
	  /* Don't use for printout the variants of divul and divsl
	     that have the same register number in two places.
	     The more general variants will match instead.  */
	  for (d = m68k_opcodes[i].args; *d; d += 2)
	    if (d[1] == 'D')
	      break;

	  /* Don't use for printout the variants of most floating
	     point coprocessor instructions which use the same
	     register number in two places, as above. */
	  if (*d == 0)
	    for (d = m68k_opcodes[i].args; *d; d += 2)
	      if (d[1] == 't')
		break;

	  if (*d == 0 && match > bestmask)
	    {
	      best = i;
	      bestmask = match;
	    }
	}
    }

  /* Handle undefined instructions.  */
  if (best < 0)
    {
      fprintf (stream, "0%o", (buffer[0] << 8) + buffer[1]);
      return 2;
    }

  fprintf (stream, "%s", m68k_opcodes[best].name);

  /* Point at first word of argument data,
     and at descriptor for first argument.  */
  p = buffer + 2;
  
  /* Why do this this way? -MelloN */
  for (d = m68k_opcodes[best].args; *d; d += 2)
    {
      if (d[0] == '#')
	{
	  if (d[1] == 'l' && p - buffer < 6)
	    p = buffer + 6;
	  else if (p - buffer < 4 && d[1] != 'C' && d[1] != '8' )
	    p = buffer + 4;
	}
      if (d[1] >= '1' && d[1] <= '3' && p - buffer < 4)
	p = buffer + 4;
      if (d[1] >= '4' && d[1] <= '6' && p - buffer < 6)
	p = buffer + 6;
    }

  d = m68k_opcodes[best].args;

  if (*d)
    fputc (' ', stream);

  while (*d)
    {
      p = print_insn_arg (d, buffer, p, memaddr + p - buffer, stream);
      d += 2;
      if (*d && *(d - 2) != 'I' && *d != 'k')
	fprintf (stream, ",");
    }
  return p - buffer;
}

static unsigned char *
print_insn_arg (d, buffer, p, addr, stream)
     char *d;
     unsigned char *buffer;
     register unsigned char *p;
     CORE_ADDR addr;		/* PC for this arg to be relative to */
     FILE *stream;
{
  register int val;
  register int place = d[1];
  int regno;
  register char *regname;
  register unsigned char *p1;
  register double flval;
  int flt_p;

  switch (*d)
    {
    case 'C':
      fprintf (stream, "ccr");
      break;

    case 'S':
      fprintf (stream, "sr");
      break;

    case 'U':
      fprintf (stream, "usp");
      break;

    case 'J':
      {
	static struct { char *name; int value; } names[]
	  = {{"sfc", 0x000}, {"dfc", 0x001}, {"cacr", 0x002},
	     {"usp", 0x800}, {"vbr", 0x801}, {"caar", 0x802},
	     {"msp", 0x803}, {"isp", 0x804}};

	val = fetch_arg (buffer, place, 12);
	for (regno = sizeof names / sizeof names[0] - 1; regno >= 0; regno--)
	  if (names[regno].value == val)
	    {
	      fprintf (stream, names[regno].name);
	      break;
	    }
	if (regno < 0)
	  fprintf (stream, "%d", val);
      }
      break;

    case 'Q':
      val = fetch_arg (buffer, place, 3);
      if (val == 0) val = 8;
      fprintf (stream, "#%d", val);
      break;

    case 'M':
      val = fetch_arg (buffer, place, 8);
      if (val & 0x80)
	val = val - 0x100;
      fprintf (stream, "#%d", val);
      break;

    case 'T':
      val = fetch_arg (buffer, place, 4);
      fprintf (stream, "#%d", val);
      break;

    case 'D':
      fprintf (stream, "%s", reg_names[fetch_arg (buffer, place, 3)]);
      break;

    case 'A':
      fprintf (stream, "%s", reg_names[fetch_arg (buffer, place, 3) + 010]);
      break;

    case 'R':
      fprintf (stream, "%s", reg_names[fetch_arg (buffer, place, 4)]);
      break;

    case 'F':
      fprintf (stream, "fp%d", fetch_arg (buffer, place, 3));
      break;

    case 'O':
      val = fetch_arg (buffer, place, 6);
      if (val & 0x20)
	fprintf (stream, "%s", reg_names [val & 7]);
      else
	fprintf (stream, "%d", val);
      break;

    case '+':
      fprintf (stream, "(%s)+", reg_names[fetch_arg (buffer, place, 3) + 8]);
      break;

    case '-':
      fprintf (stream, "-(%s)", reg_names[fetch_arg (buffer, place, 3) + 8]);
      break;

    case 'k':
      if (place == 'k')
	fprintf (stream, "{%s}", reg_names[fetch_arg (buffer, place, 3)]);
      else if (place == 'C')
	{
	  val = fetch_arg (buffer, place, 7);
	  if ( val > 63 )		/* This is a signed constant. */
	    val -= 128;
	  fprintf (stream, "{#%d}", val);
	}
      else
	error ("Invalid arg format in opcode table: \"%c%c\".",
	       *d, place);
      break;

    case '#':
      p1 = buffer + 2;
      if (place == 's')
	val = fetch_arg (buffer, place, 4);
      else if (place == 'C')
	val = fetch_arg (buffer, place, 7);
      else if (place == '8')
	val = fetch_arg (buffer, place, 3);
      else if (place == '3')
	val = fetch_arg (buffer, place, 8);
      else if (place == 'b')
	val = NEXTBYTE (p1);
      else if (place == 'w')
	val = NEXTWORD (p1);
      else if (place == 'l')
	val = NEXTLONG (p1);
      else
	error ("Invalid arg format in opcode table: \"%c%c\".",
	       *d, place);
      fprintf (stream, "#%d", val);
      break;

    case '^':
      if (place == 's')
	val = fetch_arg (buffer, place, 4);
      else if (place == 'C')
	val = fetch_arg (buffer, place, 7);
      else if (place == '8')
	val = fetch_arg (buffer, place, 3);
      else if (place == 'b')
	val = NEXTBYTE (p);
      else if (place == 'w')
	val = NEXTWORD (p);
      else if (place == 'l')
	val = NEXTLONG (p);
      else
	error ("Invalid arg format in opcode table: \"%c%c\".",
	       *d, place);
      fprintf (stream, "#%d", val);
      break;

    case 'B':
      if (place == 'b')
	val = NEXTBYTE (p);
      else if (place == 'w')
	val = NEXTWORD (p);
      else if (place == 'l')
	val = NEXTLONG (p);
      else if (place == 'g')
	{
	  val = ((char *)buffer)[1];
	  if (val == 0)
	    val = NEXTWORD (p);
	  else if (val == -1)
	    val = NEXTLONG (p);
	}
      else if (place == 'c')
	{
	  if (buffer[1] & 0x40)		/* If bit six is one, long offset */
	    val = NEXTLONG (p);
	  else
	    val = NEXTWORD (p);
	}
      else
	error ("Invalid arg format in opcode table: \"%c%c\".",
	       *d, place);

      print_address (addr + val, stream);
      break;

    case 'd':
      val = NEXTWORD (p);
      fprintf (stream, "%d(%s)", val, reg_names[fetch_arg (buffer, place, 3)]);
      break;

    case 's':
      fprintf (stream, "%s", fpcr_names[fetch_arg (buffer, place, 3)]);
      break;

    case 'I':
      val = fetch_arg (buffer, 'd', 3);		  /* Get coprocessor ID... */
      if (val != 1)				/* Unusual coprocessor ID? */
	fprintf (stream, "(cpid=%d) ", val);
      if (place == 'i')
	p += 2;			     /* Skip coprocessor extended operands */
      break;

    case '*':
    case '~':
    case '%':
    case ';':
    case '@':
    case '!':
    case '$':
    case '?':
    case '/':
    case '&':

      if (place == 'd')
	{
	  val = fetch_arg (buffer, 'x', 6);
	  val = ((val & 7) << 3) + ((val >> 3) & 7);
	}
      else
	val = fetch_arg (buffer, 's', 6);

      /* Get register number assuming address register.  */
      regno = (val & 7) + 8;
      regname = reg_names[regno];
      switch (val >> 3)
	{
	case 0:
	  fprintf (stream, "%s", reg_names[val]);
	  break;

	case 1:
	  fprintf (stream, "%s", regname);
	  break;

	case 2:
	  fprintf (stream, "(%s)", regname);
	  break;

	case 3:
	  fprintf (stream, "(%s)+", regname);
	  break;

	case 4:
	  fprintf (stream, "-(%s)", regname);
	  break;

	case 5:
	  val = NEXTWORD (p);
	  fprintf (stream, "%d(%s)", val, regname);
	  break;

	case 6:
	  p = print_indexed (regno, p, addr, stream);
	  break;

	case 7:
	  switch (val & 7)
	    {
	    case 0:
	      val = NEXTWORD (p);
	      fprintf (stream, "@#");
	      print_address (val, stream);
	      break;

	    case 1:
	      val = NEXTLONG (p);
	      fprintf (stream, "@#");
	      print_address (val, stream);
	      break;

	    case 2:
	      val = NEXTWORD (p);
	      print_address (addr + val, stream);
	      break;

	    case 3:
	      p = print_indexed (-1, p, addr, stream);
	      break;

	    case 4:
	      flt_p = 1;	/* Assume it's a float... */
	      switch( place )
	      {
		case 'b':
		  val = NEXTBYTE (p);
		  flt_p = 0;
		  break;

		case 'w':
		  val = NEXTWORD (p);
		  flt_p = 0;
		  break;

		case 'l':
		  val = NEXTLONG (p);
		  flt_p = 0;
		  break;

		case 'f':
		  flval = NEXTSINGLE(p);
		  break;

		case 'F':
		  flval = NEXTDOUBLE(p);
		  break;

		case 'x':
		  flval = NEXTEXTEND(p);
		  break;

		case 'p':
		  flval = NEXTPACKED(p);
		  break;

		default:
		  error ("Invalid arg format in opcode table: \"%c%c\".",
		       *d, place);
	      }
	      if ( flt_p )	/* Print a float? */
		fprintf (stream, "#%g", flval);
	      else
		fprintf (stream, "#%d", val);
	      break;

	    default:
	      fprintf (stream, "<invalid address mode 0%o>", val);
	    }
	}
      break;

    default:
      error ("Invalid arg format in opcode table: \"%c\".", *d);
    }

  return (unsigned char *) p;
}

/* Fetch BITS bits from a position in the instruction specified by CODE.
   CODE is a "place to put an argument", or 'x' for a destination
   that is a general address (mode and register).
   BUFFER contains the instruction.  */

static int
fetch_arg (buffer, code, bits)
     unsigned char *buffer;
     char code;
     int bits;
{
  register int val;
  switch (code)
    {
    case 's':
      val = buffer[1];
      break;

    case 'd':			/* Destination, for register or quick.  */
      val = (buffer[0] << 8) + buffer[1];
      val >>= 9;
      break;

    case 'x':			/* Destination, for general arg */
      val = (buffer[0] << 8) + buffer[1];
      val >>= 6;
      break;

    case 'k':
      val = (buffer[3] >> 4);
      break;

    case 'C':
      val = buffer[3];
      break;

    case '1':
      val = (buffer[2] << 8) + buffer[3];
      val >>= 12;
      break;

    case '2':
      val = (buffer[2] << 8) + buffer[3];
      val >>= 6;
      break;

    case '3':
    case 'j':
      val = (buffer[2] << 8) + buffer[3];
      break;

    case '4':
      val = (buffer[4] << 8) + buffer[5];
      val >>= 12;
      break;

    case '5':
      val = (buffer[4] << 8) + buffer[5];
      val >>= 6;
      break;

    case '6':
      val = (buffer[4] << 8) + buffer[5];
      break;

    case '7':
      val = (buffer[2] << 8) + buffer[3];
      val >>= 7;
      break;
      
    case '8':
      val = (buffer[2] << 8) + buffer[3];
      val >>= 10;
      break;

    default:
      abort ();
    }

  switch (bits)
    {
    case 3:
      return val & 7;
    case 4:
      return val & 017;
    case 5:
      return val & 037;
    case 6:
      return val & 077;
    case 7:
      return val & 0177;
    case 8:
      return val & 0377;
    case 12:
      return val & 07777;
    default:
      abort ();
    }
}

/* Print an indexed argument.  The base register is BASEREG (-1 for pc).
   P points to extension word, in buffer.
   ADDR is the nominal core address of that extension word.  */

static unsigned char *
print_indexed (basereg, p, addr, stream)
     int basereg;
     unsigned char *p;
     FILE *stream;
     CORE_ADDR addr;
{
  register int word;
  static char *scales[] = {"", "*2", "*4", "*8"};
  register int base_disp;
  register int outer_disp;
  char buf[40];

  word = NEXTWORD (p);

  /* Generate the text for the index register.
     Where this will be output is not yet determined.  */
  sprintf (buf, "[%s.%c%s]",
	   reg_names[(word >> 12) & 0xf],
	   (word & 0x800) ? 'l' : 'w',
	   scales[(word >> 9) & 3]);

  /* Handle the 68000 style of indexing.  */

  if ((word & 0x100) == 0)
    {
      print_base (basereg,
		  ((word & 0x80) ? word | 0xff00 : word & 0xff)
		  + ((basereg == -1) ? addr : 0),
		  stream);
      fprintf (stream, "%s", buf);
      return p;
    }

  /* Handle the generalized kind.  */
  /* First, compute the displacement to add to the base register.  */

  if (word & 0200)
    basereg = -2;
  if (word & 0100)
    buf[0] = 0;
  base_disp = 0;
  switch ((word >> 4) & 3)
    {
    case 2:
      base_disp = NEXTWORD (p);
      break;
    case 3:
      base_disp = NEXTLONG (p);
    }
  if (basereg == -1)
    base_disp += addr;

  /* Handle single-level case (not indirect) */

  if ((word & 7) == 0)
    {
      print_base (basereg, base_disp, stream);
      fprintf (stream, "%s", buf);
      return p;
    }

  /* Two level.  Compute displacement to add after indirection.  */

  outer_disp = 0;
  switch (word & 3)
    {
    case 2:
      outer_disp = NEXTWORD (p);
      break;
    case 3:
      outer_disp = NEXTLONG (p);
    }

  fprintf (stream, "%d(", outer_disp);
  print_base (basereg, base_disp, stream);

  /* If postindexed, print the closeparen before the index.  */
  if (word & 4)
    fprintf (stream, ")%s", buf);
  /* If preindexed, print the closeparen after the index.  */
  else
    fprintf (stream, "%s)", buf);

  return p;
}

/* Print a base register REGNO and displacement DISP, on STREAM.
   REGNO = -1 for pc, -2 for none (suppressed).  */

static void
print_base (regno, disp, stream)
     int regno;
     int disp;
     FILE *stream;
{
  if (regno == -2)
    fprintf (stream, "%d", disp);
  else if (regno == -1)
    fprintf (stream, "0x%x", disp);
  else
    fprintf (stream, "%d(%s)", disp, reg_names[regno]);
}

/* This is not part of insn printing, but it is machine-specific,
   so this is a convenient place to put it.

   Convert a 68881 extended float to a double.
   FROM is the address of the extended float.
   Store the double in *TO.  */

convert_from_68881 (from, to)
     char *from;
     double *to;
{
#if 0
  asm ("movl a6@(8),a0");
  asm ("movl a6@(12),a1");
  asm ("fmovex a0@,fp0");
  asm ("fmoved fp0,a1@");
#else
  /* Hand-assemble those insns since some assemblers lose
     and some have different syntax.  */
  asm (".word 020156");
  asm (".word 8");
  asm (".word 021156");
  asm (".word 12");
  asm (".long 0xf2104800");
  asm (".long 0xf2117400");
#endif
}

/* The converse: convert the double *FROM to an extended float
   and store where TO points.  */

convert_to_68881 (from, to)
     double *from;
     char *to;
{
#if 0
  asm ("movl a6@(8),a0");
  asm ("movl a6@(12),a1");
  asm ("fmoved a0@,fp0");
  asm ("fmovex fp0,a1@");
#else
  /* Hand-assemble those insns since some assemblers lose.  */
  asm (".word 020156");
  asm (".word 8");
  asm (".word 021156");
  asm (".word 12");
  asm (".long 0xf2105400");
  asm (".long 0xf2116800");
#endif
}
