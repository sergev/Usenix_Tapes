#include <stdio.h>
#include <ctype.h>
#include "argument.h"

/*
 * argv_file.c: argv_file
 * Set up to read actual arguments from a file, separated by white space.
 * Backslash and single and double quotes are supported, 
 * filename wild cards are not.
 * Argv_eof, argv_get, and argv_unget in argv_get.c should be used to 
 * handle the actuals.  Argv_unget is done completely in argv_get.c.
 *
 * history:     created 21 September 1981       John S. Quarterman.
 *      29 October 1981 added prompt    John S. Quarterman
 */

int argv_file (file, prompt)
FILE *file;
int (* prompt) ();
{
  extern int _argv_eof ();
  extern char *_argv_get ();
  extern FILE *_fpointer;
  extern int (* _fprompt) (); 

  argv_switch.argv_eof = _argv_eof; 
  argv_switch.argv_get = _argv_get;
  if (file != NULL)
  {
    if (!isatty (fileno (file)))
      prompt = NULL;    /* prompt only if input is a tty */
    _fpointer = NULL;   /* NULL forces _argv_get to reset */
    _fprompt = prompt;  /* and set up for initial prompt */
    _argv_get ();       /* do the reset */
  }
  _fpointer = file;     /* put the real file in */
  return (1);
}

/*
 * check for eof.
 */
static int _argv_eof () 
{
  extern FILE *_fpointer;

  if (_fpointer == NULL)
    return (1);
  return (feof (_fpointer));
}

/**/
/*
 * _argv_get:  return next actual;  eof has already been checked.
 */
static char *_argv_get ()
{
  extern int isquote ();
  extern FILE *_fpointer;
  extern int (*_fprompt) ();
  static char gotten[128];
  static int doprompt;
  register char *put = &gotten[0];
  register int this;
  register int quoted;
  int started = 0;

  if (_fpointer == NULL)
  {                             /* reset */
    doprompt = (_fprompt != NULL);
    isquote (-1);
    return (NULL);
  }
  while
  (
    (!doprompt ? 1 : (((*_fprompt) ()), doprompt = 0, 1))  /* prompt? */
    && (put < &gotten[sizeof (gotten) - 1])     /* end of string buffer? */ 
    && ((this = getc (_fpointer)) != EOF)       /* end of file? */ 
  )
  {
    quoted = isquote (this);
    if (quoted < 0)
    {                   /* quote character:  throw it away */
      continue;
    }
    if (quoted > 0)
    {                   /* quoted character:  use it */
      started = 1;      /* it is part of the argument */
      *put++ = this;
      continue;
    }
    if (isspace (this) || (this == '\n')) 
    {                   /* white space:  argument separator */
      if (this == '\n') /* set prompt when newline seen */
	doprompt = (_fprompt != NULL);
      if (started)
	break;          /* end of argument */ 
      continue;
    }
    started = 1;        /* any non-white space char starts an argument */
    *put++ = this;      /* ordinary character */
  }
  *put = '\0';          /* null terminate the argument */
  if (!started)
    return (NULL);      /* EOF or error before any argument was read */
  return (gotten);              /* return it */
}

/*
 * isquote:  check to see if a character is a quote.  
 * returns:  -1 (quote char); 1 (quoted char); 0 (ordinary char).
 * -1 argument resets.
 */
static int isquote (this)
register int this;
{
  struct fs_oc          /* the quote characters */
  {
    int fq_open;        /* open quote character */
    int fq_close;       /* close quote character */
  };
  static struct fs_oc *fs_quoted = NULL; /* no quote is open */
  static struct fs_oc fs_oc[] =
  {
    { '\'', '\'' },       /* character quote */
    { '\`', '\'' },       /* other character quote */
    { '\"', '\"' },       /* string quote */
    { '\0', '\0' },       /* end of the array */
  };
  register struct fs_oc *comp = fs_quoted;
  register int esc;

  if (this == -1)
  {                             /* reset */
    isescaped (-1);
    fs_quoted = NULL;
    return (0);
  };
  esc = isescaped (this);
  if (esc != 0)
    return (esc);
  if (comp != NULL)
  {                           /* a quote is open, try to close it */
    if (this == comp -> fq_close)
    {                         /* close quote character */
      fs_quoted = NULL;
      return (-1);
    }
    return (1);                 /* quoted character */
  }
  for (comp = &fs_oc[0]; comp -> fq_open != '\0'; comp++)
  {                             /* try to open a quote */
    if (this == comp -> fq_open)
    {                           /* open quote character */
      fs_quoted = comp;
      return (-1);
    }
  }
  return (0);                   /* ordinary character */
}

/*
 * isescape:  check to see if a character is escaped.
 * returns:  -1 (escape char); 1 (escaped char); 0 (ordinary char).
 * -1 argument resets.
 */
static int isescaped (this)
register int this;
{
  static int fs_escaped  = 0;           /* escape is not open */

  if (this == -1)
  {
    fs_escaped = 0;
    return (0);
  }
  if (fs_escaped)       
  {                             /* note escaped character */
    fs_escaped = 0;
    if (this == '\n')           /* backslash at the end of a line */
      return (-1);              /* turns the newline into nothing */
    return (1);
  }
  if (this == '\\')
  {                             /* note next character is escaped */
    fs_escaped = 1;
    return (-1);
  }
  return (0);                   /* ordinary character */
}

static FILE *_fpointer = NULL;  /* the file to read */
static int (*_fprompt) () = NULL;   /* the prompt routine to call */
