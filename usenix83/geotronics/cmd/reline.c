/*
 * reline.c
 * John Quarterman
 * 27 April 1982
 * 26 May 1982
 */

#include <stdio.h>
#include <ctype.h>
#include <argument.h>
#include <ttypes.h>
#include <signal.h>

#define MAXSTRIKE 8             /* maximum number of overstrikes buffered */
#define MAXLINE 132             /* maximum overstrike linelength */
#define TABSIZE 8               /* standard tab stop interval */

static char *help_main[] =
{
  arg_replace, " makes underscores and boldface visible on terminal screens,\n",
  "and can handle various text conversions necessary for most printers,\n",
  "including underlining and multiple overstriking (for boldface).\n",
  "Tabs are converted to spaces at 8 character stops.\n",
  "Line feed and form feed are passed unchanged.\n",
  "Overlong (132 characters maximum) lines are truncated.\n",
  "\n",
  "The terminal output mode is the default.  Some terminals have\n",
  "several attributes supported, e.g., boldface and underline on VT100s.\n",
  "On other terminals and in printer output (-f -r -b), one sees:\n",
  "Non-ascii characters are shown as ~ followed by the ascii equivalent.\n",
  "Non-space control characters are shown in printing form, e.g., ^K.\n",
  "Rubout is shown as ^/.\n",
  "\n",
  NULL
};

static int useatt = 1;          /* use attributes */
static int versatec = 0;        /* versatec-style attributes */
static int forward = 0;         /* opposite of reverse */
static int reverse = 1;         /* output lines in reverse order */
static int useback = 0;         /* backspaces instead of carriage return */
static int usenewline = 0;      /* newline instead of carriage return */
static int strip = 0;           /* flag to strip overstrikes */
static char *termname = NULL;

int main (argc, argv)
int argc;
char **argv;
{
  extern char *termtype ();
  extern void reline (), setsigs (), setterm ();
  extern int offit (), getterm (), flagatt ();
  extern int useback, usenewline, reverse, strip, useatt, versatec;
  static char outbuf[BUFSIZ];   /* stdout buffer for setbuf */
  int unbuffer = 0;             /* don't buffer flag */
  char *inname = NULL;          /* input file name */
  int ret;

  setterm (termtype (NULL));     /* what are we? */

  arg_init (&argc, &argv, help_main);

  arg_spec (0, "-u", NULL, "%d", &unbuffer
    , "%d", "unbuffered output", NULL, NULL);
  arg_spec (0, "-a", NULL, "%d", &useatt
    , "%d", "special terminal character attributes", flagatt, NULL);
  arg_spec (0, "-v", NULL, "%d", &versatec
    , "%d", "Versatec:  underscores marked by high bit", flagatt, NULL);
  arg_spec (0, "-f", NULL, "%d", &forward
    , "%d", "forward printer striking order", offit, &reverse);
  arg_spec (0, "-r", NULL, "%d", &reverse
    , "%d", "reverse printer striking order", offit, &forward);
  arg_spec (0, "-b", NULL, "%d", &useback
    , "%d", "backspaces instead of carriage return", offit, &useatt);
  arg_spec (0, "-n", NULL, "%d", &usenewline
    , "%d", "newline instead of carriage return", offit, &useatt);
  arg_spec (0, "-s", NULL, "%d", &strip
    , "%d", "strip attributes or overstrikes", NULL, NULL);
  arg_spec (0, "-t", "name", "", &termname
    , "", "terminal type", getterm, &termname);
  arg_spec (0, "[", "name", "", &inname
    , "", "input file name", NULL, NULL);

  while ((ret = arg_process ()) > 0)
    ;
  if (ret < 0)
    exit (-1);

  if ((inname != NULL) && (strcmp (inname, "-") != 0))
    if (NULL == freopen (inname, "r", stdin))
      arg_error (&inname, 0, "can't open");
  if (!unbuffer)
    setbuf (stdout, outbuf);
  if (versatec)
    setterm ("versatec");

  setsigs ();
  reline ();
  exit (0);
}

static int offit (what)
char *what;
{
  *((int *) what) = 0;
  useatt = 0;
  versatec = 0;
  return (1);
}

static int flagatt ()
{
  useatt = 1;
  forward = 0;
  reverse = 1;
  useback = 0;
  usenewline = 0;
  return (1);
}
/**/
struct strike
{                               /* overstrike control structure */
  int s_full;                   /* next column to fill in the buffer */
  int s_col;                    /* highest (space) column */
  char s_buf[MAXLINE];          /* the character buffer */
};
static struct strike linebuf[MAXSTRIKE + 1];

#define botline (&linebuf[0])  /* first, last, and current lines */
#define topline (&linebuf[MAXSTRIKE])
static struct strike *line = botline;

#define basic (&linebuf[0])     /* the basic line */
static int levels = 0;          /* highest used line index */

static struct strike att;       /* attribute stuff */

/*
 * addatt:  add another attribute for a character position.
 * areatt:  there are attributes in this line to output.
 * getatt:  get the attributes for this character position.
 */
#define addatt(a, col) (\
  (att.s_buf[col] |= (a)),\
  ((col >= att.s_full) ? (att.s_full = (col) + 1 ): 0)\
)
#define areatt (useatt && (att.s_full != 0))
#define getatt(col) (att.s_buf[col])

/* character attributes */
#define A_UNDER         0001    /* underlined */
#define A_BOLD          0002    /* boldface */
#define A_CONTROL       0004    /* control */
#define A_META          0010    /* meta */

/**/
/*
 * reline:  does the work.
 */
static void reline ()
{
  extern void outchar (), backchar (), contchar ();
  extern void outflush (), setatt ();
  register int this;            /* the input character */

  while ((this = getchar ()) != EOF)
  {
    if (!isascii (this))
    {                           /* metacharacter */
      if (useatt)
	setatt (A_META);
      else
	outchar ('~');
      this = toascii (this);
      if (iscntrl (this) || (isspace (this) && (this != '\ ')))
	contchar (this);        /* metacontrol */
      else
	outchar (this);         /* metanormal */
      continue;
    }
    if (isprint (this) || (this == '\ '))
    {                           /* 90% of the characters go here */
      outchar (this);
      continue;
    }
    if (this == '\t')
    {                           /* tabs */
      do
	outchar ('\ ');         /* output spaces until at a tab stop */
      while ((line -> s_col % TABSIZE) != 0);
      continue;
    }
    if ((this == '\b') || (this == '\r'))
    {                           /* backspace or carriage return */
      backchar (this);
      continue;
    }
    if (isspace (this) && (this != '\v'))
    {                           /* newlines and the like */
      outflush (this);
      continue;
    }
    contchar (this);            /* control character */
  }
  outflush (EOF);               /* output anything in the strike buffer */
  fflush (stdout);              /* make sure it's output */
}
/**/
/*
 * contchar:  output a control character in printable form.
 */
static void contchar (this)
register int this;
{
  this = (this == '\0177') ? '/' : (this + '@');
  if (useatt)
    setatt (A_CONTROL);
  else
    outchar ('^');
  outchar (this);
}

/*
 * outchar:  buffer a character in a line buffer.
 */
static void outchar (this)
int this;
{
  register int col = line -> s_col;
  register int full = line -> s_full;

  if (col >= MAXLINE)
    line -> s_col++;
  else
  if (this == '\ ')
    line -> s_col++;            /* just advance buffer column for space */
  else
  {
    while (full < col)          /* fill in the blanks with spaces */
      line -> s_buf[full++] = '\ ';
    line -> s_buf[full++] = this;
    line -> s_full = full;
    line -> s_col = full;
  }
  while ((line > botline) && (line -> s_col >= ((line - 1) -> s_full)))
    line--;                    /* back up a level of overstriking */
}
/**/
/*
 * backchar:  handle backspace and carriage return.
 */
static void backchar (this)
int this;
{
  register int col;

  if (this == '\b')
  {
    col = line -> s_col;
    if (col > 0)
      col--;
  }
  else                          /* carriage return */
    col = 0;
  if (col >= line -> s_full)
  {                             /* room to back up one in the buffer */
    line -> s_col = col;
    return;
  }
  while ((line < topline) && (col < line -> s_full))
    line++;                     /* go up a level of overstriking */
  if ((line - botline) > levels)
    levels = line - botline;    /* bump overstrike line count */
  line -> s_col = col;          /* preserve overstrike column */
}
/**/
/*
 * outflush:  output a line terminator, flushing the overstrike buffer. 
 */
static void outflush (this)
int this;
{
  extern void doatt (), offatt (), outatt ();
  register struct strike *strike;
  struct strike *first;
  register struct strike *last;
  register int inc;

#define dumpline(s,l) (\
  ((s) -> s_buf[(s) -> s_full] = '\0'),\
  fputs (&((s) -> s_buf[(l)]), stdout)\
)

  if (useatt)                   /* attributes for terminals */
  {
    for (strike = botline + 1; strike <= topline; strike++) 
      doatt (strike);
    if (!strip && areatt)
      outatt ();
    else
      dumpline (basic, 0);
    if (areatt)
      offatt ();
  }
  else
  if (strip)                    /* strip all overstruck lines */
    dumpline (basic, 0);
  else
  {
    if (!reverse)
    {   /* forward */
      inc = 1;
      first = botline;
      last = &linebuf[levels];
    }
    else
    {   /* reverse */
      inc = -1;
      first = &linebuf[levels];
      last = botline;
    }
    for (strike = first; ; strike += inc)
    {
      int low;

      low = 0;
      if (strike != first)
      {
	if (useback)
	  low = backcol (strike -> s_buf, (strike - inc) -> s_full);
	else
	if (usenewline)
	  putchar ('\n');
	else
	  putchar ('\r');
      }
      if (low < strike -> s_full)
	dumpline (strike, low);
      if (strike == last)
	break;                  /* normal loop termination */
    }
  }
  if (this != EOF)
    putchar (this);
  for (strike = &linebuf[levels]; strike >= botline; strike--)
    strike -> s_col = strike -> s_full = 0;
  levels = 0;
  line = botline;
}

static int backcol (buffer, top)
register char *buffer;          /* what to output */
register int top;               /* where to back up from */
{
  register int col;
  int low;

  for (col = 0; (buffer[col] == '\ ') && (col++ < top); )
    ;                           /* don't bother with leading spaces */
  if (col > top)
    for (low = top; top++ <= col; putchar ('\ '))
      ;
  else
    for (low = col; col++ < top; putchar ('\b'))
      ;                           /* use many backspaces */
  return (low);
}
/**/
/*
 * setatt:  add in an attribute for a character.
 * Used for control and meta characters.
 */
static void setatt (what)
int what;
{
  register int col = line -> s_full;

  if (col >= MAXLINE)
    return;                     /* no room in the buffer */
  addatt (what, col);           /* do it */
}

/*
 * doatt:  deduce attributes from overstruck characters.
 * You can get both bold face and underline, but not by the same overstrike.
 */
static void doatt (strike)
struct strike *strike;
{
  register char *toadd = strike -> s_buf;
  register char *totop = &(toadd[strike -> s_full]);
  register char *addto = basic -> s_buf;
  char *buffer = toadd;

  for (; toadd < totop; toadd++, addto++)
  {
    if (*toadd == *addto)
    {                           /* boldface */
      if (*toadd == '\ ')
	continue;
      addatt (A_BOLD, toadd - buffer);
      continue;
    }
    if ((*toadd == '_') || (*addto == '_'))
    {                           /* underlined */
      addatt (A_UNDER, toadd - buffer);
      if ((*toadd == '_') && (*addto != '\ '))
	continue;
    }
    if (*toadd == '\ ')
      continue;
    *addto = *toadd;            /* replace one with the other */
  }
}

static void offatt ()
{
  register int col;

  for (col = att.s_full; col > 0; att.s_buf[--col] = 0)
    ;
  att.s_full = 0;
}
/**/
/*
 * Versatec thingies.
 */
static int v_char (col, this, thing)
int col, this, thing;
{
  if (this & A_META)
    putchar ('~');
  if (this & A_CONTROL)
    putchar ('^');
  if (this & A_UNDER)
    thing |= 0200;
  putchar (thing);
}

static int v_off ()
{
}
/*
 * Random terminal stuff:  underscores shown by hyphens in following line.
 */
static struct strike hyphens;
static int t_char (col, this, thing)
register int col;
register int this;
int thing;
{
  if (this & A_META)
    putchar ('~');
  else
  if (this & A_CONTROL)
    putchar ('^');
  else
  if (this & (A_UNDER | A_BOLD))
  {
    register int hchar;

    while (hyphens.s_full < col)
      hyphens.s_buf[hyphens.s_full++] = '\ ';
    switch ((int) (this & (A_UNDER | A_BOLD)))
    {
      case A_UNDER:
	hchar = '-';            /* just underlined */
	break;
      case A_BOLD:
	hchar = thing;          /* just bold */
	break;
      case (A_BOLD | A_UNDER):
	hchar = '=';            /* bold and underlined */
	break;
    }
    hyphens.s_buf[hyphens.s_full++] = hchar;
  }
  putchar (thing);
}

static int t_off ()
{
  register char *low = &hyphens.s_buf[0]; 
  register char *high = &hyphens.s_buf[hyphens.s_full];

  if (high > low)
  {
    putchar ('\n');
    while (low < high)
      putchar (*low++);
  }
  hyphens.s_full = 0;
}
/**/
/*
 * VT100 thingies.
 */
static int vt_char (col, theatt, thechar)
register int col;
register int theatt;
int thechar;
{
  char modes[32];
  register char *point = modes;

  if ((col <= 0) || (getatt (col - 1) != theatt))
  {
    *point++ = '\033';
    *point++ = '[';
    if (theatt & A_BOLD)
    {
      *point++ = ';';
      *point++ = '1';           /* boldface */
    }
    if (theatt & A_UNDER)
    {
      *point++ = ';';
      *point++ = '4';           /* underscore */
    }
    if (theatt & A_META)
    {
      *point++ = ';';
      *point++ = '5';           /* blinking */
    }
    if (theatt & A_CONTROL)
    {
      *point++ = ';';
      *point++ = '7';           /* reverse video */
    }
    *point++ = 'm';
  }
  *point++ = thechar;
  if ((col >= MAXLINE - 1) || (getatt (col + 1) != theatt))
  {
    *point++ = '\033';          /* all char attributes off */
    *point++ = '[';
    *point++ = 'm';
  }
  *point = '\0';
  fputs (modes, stdout);
}

static int vt_off ()
{
}

static int vt_sig (sig)
int sig;
{
  fputs ("\033[m", stdout);     /* all char attributes off */
  exit (-sig);
}
/**/

struct termatt
{
  char *t_name;                 /* terminal type name */
  int (*t_achar) ();           /* used to output attributed characters */
  int (*t_aoff) ();            /* flush attributed line */
  int (*t_asig) ();            /* signal catching routine */
  char *t_asetup;               /* terminal mode setup string */
};

static struct termatt termatt[] =
{
  { "dumb", t_char, t_off, },
  { "vt100", vt_char, vt_off, vt_sig,
    "\033<\033[m"               /* ANSI (VT100) mode, all char attributes off */
  },
  { "versatec", v_char, v_off, },
  { NULL, },
};
static struct termatt *termel = &termatt[0];

static void setsigs ()
{
  register struct termatt *term = termel;
  register int (*func) ();

  if (!useatt)
    return;

  if (term -> t_asetup != NULL)
    fputs (term -> t_asetup, stdout);
  if ((func = term -> t_asig) == NULL)
    return;
  if (signal (SIGINT, SIG_IGN) != SIG_IGN)
    signal (SIGINT, func); 
  if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
    signal (SIGTERM, func);
}

static int getterm (what)
char *what;
{
  extern char *termname;
  register char *name = *((char **) what);

  setterm (termtype (name));
  if (strcmp (termname, name) != 0)
  {
    *((char **) what) = name; 
    arg_error (NULL, 0, "unknown terminal type name");
  }
  return (1);
}

static void setterm (name)
register char *name;
{
  extern struct termatt termatt[];
  extern struct termatt *termel;
  extern char *termname;
  register struct termatt *term;

  for (term = termatt; term -> t_name != NULL; term++)
    if (strcmp (term -> t_name, name) == 0)
      break;
  if (term -> t_name == NULL)
    term = &termatt[0];
  termname = term -> t_name;
  termel = term;
}

static void outatt ()
{
  register int col;
  register int top = basic -> s_full;
  register char *buffer = basic -> s_buf;
  int attributes;
  struct termatt *term = termel;

  for (col = 0; col < top; col++)
  {
    if ((attributes = getatt (col)) == 0)
      putchar (buffer[col]);
    else
      (*(term -> t_achar)) (col, attributes, buffer[col]);
  }
  (*(term -> t_aoff)) (attributes);
}
/**/
static char *termtype (what)
char *what;
{
  extern char *getenv ();
  extern int ttytype ();
  register char *name;

  if (what == NULL)
    if ((name = getenv ("TERM")) != NULL)
      return (name);
  switch (ttytype (what))
  {
    case VT100:
      return ("vt100");
    case ADM3A:
      return ("adm3a");
    case VK170:
      return ("vk170");
  }
  return ("dumb");
}

#define   TTYFILE   "/etc/ttytype"

static  char    *typename =     NULL;

static int
ttytype( select )
char *select;
{
  register  FILE      *fp;

  auto      char      line[5];
  register  char      *linep = &line[0];

  extern    char      *ttyname();
  auto      char      *tname;
  register  char      name;

  extern    char      *typename;
  extern    int       strcmp();

  extern    int       exit();


  if( select != NULL )
    typename = select;
  else
  {
    if ((tname = ttyname (fileno (stdout))) == NULL)
      tname = ttyname (fileno (stderr));
    if (tname == NULL)
      return (-1);
    name = tname[8];
    if( (fp = fopen( TTYFILE, "r" )) == NULL )
      return (-1);
    while( fgets( linep, 5, fp ) != NULL )
      if( *linep == name )
      {
	*(linep+3) = '\0';
	typename = ++linep;
	break;
      }
  }
  if( typename == NULL )
    return (-1);

  if((strcmp( typename, "d1" ) == 0 ) || (strcmp (ttyname, "vt100") == 0))
    return VT100;

  if((strcmp( typename, "la" ) == 0 ) || (strcmp (ttyname, "adm3a") == 0))
    return ADM3A;

  if((strcmp( typename, "dv" ) == 0 ) || (strcmp (ttyname, "vk170") == 0))
    return VK170;

  return (-1);
}
