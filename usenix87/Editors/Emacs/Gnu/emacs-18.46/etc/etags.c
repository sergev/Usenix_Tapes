/* Tags file maker to go with GNUmacs
   Copyright (C) 1984, 1987 Free Software Foundation, Inc. and Ken Arnold

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
as you receive it, in any medium, provided that you conspicuously
and appropriately publish on each copy a valid copyright notice
"Copyright (C) 1986 Free Software Foundation"; and include
following the copyright notice a verbatim copy of the above disclaimer
of warranty and of this License.

  2. You may modify your copy or copies of this source file or
any portion of it, and copy and distribute such modifications under
the terms of Paragraph 1 above, provided that you also do the following:

    a) cause the modified files to carry prominent notices stating
    that you changed the files and the date of any change; and

    b) cause the whole of any work that you distribute or publish,
    that in whole or in part contains or is a derivative of this
    program or any part thereof, to be freely distributed
    and licensed to all third parties on terms identical to those
    contained in this License Agreement (except that you may choose
    to grant more extensive warranty protection to third parties,
    at your option).

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

In other words, you are welcome to use, share and improve this program.
You are forbidden to forbid anyone else to use, share and improve
what you give them.   Help stamp out software-hoarding!  */

#include <stdio.h>
#include <ctype.h>

/* Define the symbol ETAGS to make the program "etags",
 which makes emacs-style tag tables by default.
 Define CTAGS to make the program "ctags" compatible with the usual one.
 Default is ETAGS.  */

#ifndef CTAGS
#ifndef ETAGS
#define ETAGS 1
#endif
#endif

#define	reg	register
#define	logical	char

#define	TRUE	(1)
#define	FALSE	(0)

#define	iswhite(arg)	(_wht[arg])	/* T if char is white		*/
#define	begtoken(arg)	(_btk[arg])	/* T if char can start token	*/
#define	intoken(arg)	(_itk[arg])	/* T if char can be in token	*/
#define	endtoken(arg)	(_etk[arg])	/* T if char ends tokens	*/
#define	isgood(arg)	(_gd[arg])	/* T if char can be after ')'	*/

#define	max(I1,I2)	(I1 > I2 ? I1 : I2)

struct	nd_st {			/* sorting structure			*/
	char	*name;			/* function or type name	*/
	char	*file;			/* file name			*/
	logical f;			/* use pattern or line no	*/
	int	lno;			/* line number tag is on	*/
	long    cno;			/* character number line starts on */
	char	*pat;			/* search pattern		*/
	logical	been_warned;		/* set if noticed dup		*/
	struct	nd_st	*left,*right;	/* left and right sons		*/
};

long	ftell();
typedef	struct	nd_st	NODE;

int number; /* tokens found so far on line starting with # (including #) */
logical gotone,				/* found a func already on line	*/
					/* boolean "func" (see init)	*/
	_wht[0177],_etk[0177],_itk[0177],_btk[0177],_gd[0177];

	/* typedefs are recognized using a simple finite automata,
	 * tydef is its state variable.
	 */
typedef enum {none, begin, middle, end } TYST;

TYST tydef = none;

char	searchar = '/';			/* use /.../ searches 		*/

int	lineno;			/* line number of current line */
long	charno;			/* current character number */
long	linecharno;		/* character number of start of line */

char    *curfile,		/* current input file name		*/
	*outfile= "tags",	/* output file				*/
	*white	= " \f\t\n",	/* white chars				*/
	*endtk	= " \t\n\"'#()[]{}=-+%*/&|^~!<>;,.:?",
				/* token ending chars			*/
	*begtk	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz",
				/* token starting chars			*/
	*intk	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz0123456789",
				/* valid in-token chars			*/
	*notgd	= ",;";		/* non-valid after-function chars	*/

int	file_num;		/* current file number			*/
int	aflag;			/* -a: append to tags */
int	tflag;			/* -t: create tags for typedefs */
int	uflag;			/* -u: update tags */
int	wflag;			/* -w: suppress warnings */
int	vflag;			/* -v: create vgrind style index output */
int	xflag;			/* -x: create cxref style output */
int	eflag;			/* -e: emacs style output */

FILE	*inf,			/* ioptr for current input file		*/
	*outf;			/* ioptr for tags file			*/

NODE	*head;			/* the head of the sorted binary tree	*/

char *savestr();
char *savenstr ();
char *rindex();
char *index();
char *concat ();
void initbuffer ();
long readline ();

/* A `struct linebuffer' is a structure which holds a line of text.
 `readline' reads a line from a stream into a linebuffer
 and works regardless of the length of the line.  */

struct linebuffer
  {
    long size;
    char *buffer;
  };

struct linebuffer lb, lb1;

#ifdef VMS

#include descrip

void
system (buf)
     char *buf;
{
  struct dsc$descriptor_s command =
    {
      strlen(buf), DSC$K_DTYPE_T, DSC$K_CLASS_S, buf
    };

  LIB$SPAWN(&command);
}
#endif /* VMS */


main(ac,av)
     int	ac;
     char	*av[];
{
  char cmd[100];
  int i;

#ifdef ETAGS
  eflag = 1;
#endif

  while (ac > 1 && av[1][0] == '-')
    {
      eflag = 0;
      for (i=1; av[1][i]; i++)
	{
	  switch(av[1][i])
	    {
	    case 'B':
	      searchar='?';
	      break;
	    case 'F':
	      searchar='/';
	      break;
	    case 'a':
	      aflag++;
	      break;
	    case 'e':
	      eflag++;
	      break;
	    case 't':
	      tflag++;
	      break;
	    case 'u':
	      uflag++;
	      break;
	    case 'w':
	      wflag++;
	      break;
	    case 'v':
	      vflag++;
	      xflag++;
	      break;
	    case 'x':
	      xflag++;
	      break;
	    default:
	      goto usage;
	    }
	}
      ac--; av++;
    }

  if (ac <= 1)
    {
    usage:
      printf("Usage: ctags [-BFaetuwvx] file ...\n");
      exit(1);
    }

  if (eflag)
    outfile = "TAGS";

  init();			/* set up boolean "functions"		*/

  initbuffer (&lb);
  initbuffer (&lb1);
  /*
   * loop through files finding functions
   */
  if (eflag)
    {
      outf = fopen (outfile, aflag ? "a" : "w");
      if (!outf)
	{
	  perror (outfile);
	  exit (1);
	}
    }

  for (file_num = 1; file_num < ac; file_num++)
    {
      find_entries(av[file_num]);
      if (eflag)
	{
	  fprintf (outf, "\f\n%s,%d\n",
		   av[file_num], total_size_of_entries (head));
	  put_entries (head);
	  free_tree (head);
	  head = NULL;
	}
    }

  if (eflag)
    {
      fclose (outf);
      exit (0);
    }

  if (xflag)
    {
      put_entries(head);
      exit(0);
    }
  if (uflag)
    {
      for (i=1; i<ac; i++)
	{
	  sprintf(cmd,
		  "mv %s OTAGS;fgrep -v '\t%s\t' OTAGS >%s;rm OTAGS",
		  outfile, av[i], outfile);
	  system(cmd);
	}
      aflag++;
    }
  outf = fopen(outfile, aflag ? "a" : "w");
  if (outf == NULL)
    {
      perror(outfile);
      exit(1);
    }
  put_entries(head);
  fclose(outf);
  if (uflag)
    {
      sprintf(cmd, "sort %s -o %s", outfile, outfile);
      system(cmd);
    }
  exit(0);
}

/*
 * This routine sets up the boolean psuedo-functions which work
 * by seting boolean flags dependent upon the corresponding character
 * Every char which is NOT in that string is not a white char.  Therefore,
 * all of the array "_wht" is set to FALSE, and then the elements
 * subscripted by the chars in "white" are set to TRUE.  Thus "_wht"
 * of a char is TRUE if it is the string "white", else FALSE.
 */
init()
{

  reg char *sp;
  reg int i;

  for (i = 0; i < 0177; i++)
    {
      _wht[i] = _etk[i] = _itk[i] = _btk[i] = FALSE;
      _gd[i] = TRUE;
    }
  for (sp = white; *sp; sp++)
    _wht[*sp] = TRUE;
  for (sp = endtk; *sp; sp++)
    _etk[*sp] = TRUE;
  for (sp = intk; *sp; sp++)
    _itk[*sp] = TRUE;
  for (sp = begtk; *sp; sp++)
    _btk[*sp] = TRUE;
  for (sp = notgd; *sp; sp++)
    _gd[*sp] = FALSE;
  _wht[0] = _wht['\n'];
  _etk[0] = _etk['\n'];
  _btk[0] = _btk['\n'];
  _itk[0] = _itk['\n'];
  _gd[0] = _gd['\n'];
}

/*
 * This routine opens the specified file and calls the function
 * which finds the function and type definitions.
 */
find_entries (file)
     char *file;
{
  char *cp;

  if ((inf=fopen(file,"r")) == NULL)
    {
      perror(file);
      return;
    }
  curfile = savestr(file);
  cp = rindex(file, '.');
  /* .tex, .aux or .bbl implies LaTeX source code */
  if (cp && (!strcmp (cp + 1, "tex") || !strcmp (cp + 1, "aux")
	     || !strcmp (cp + 1, "bbl")))
    {
      TEX_funcs(inf);
      fclose(inf);
      return;
    }
  /* .l or .el or .lisp (or .cl or .clisp or ...) implies lisp source code */
  if (cp && (!strcmp (cp + 1, "l") ||
	     !strcmp (cp + 1, "el") ||
	     !strcmp (cp + 1, "lsp") ||
	     !strcmp (cp + 1, "lisp") ||
	     !strcmp (cp + 1, "cl") ||
	     !strcmp (cp + 1, "clisp")))
    {
      L_funcs(inf);
      fclose(inf);
      return;
    }
  /* .scm or .sm or .scheme implies scheme source code */
  if (cp && (!strcmp (cp + 1, "sm") ||
	     !strcmp (cp + 1, "scm") ||
	     !strcmp (cp + 1, "scheme") ||
	     !strcmp (cp + 1, "SM") ||
	     !strcmp (cp + 1, "SCM") ||
             /* The `SCM' or `scm' prefix with a version number */
             (cp[-1] == 'm' && cp[-2] == 'c' && cp[-3] == 's') ||
             (cp[-1] == 'M' && cp[-2] == 'C' && cp[-3] == 'S')))
    {
      Scheme_funcs(inf);
      fclose(inf);
      return;
    }
  /* if not a .c or .h or .y file, try fortran */
  if (cp && (cp[1] != 'c' && cp[1] != 'h' && cp[1] != 'y')
      && cp[2] == '\0')
    {
      if (PF_funcs(inf) != 0)
	{
	  fclose(inf);
	  return;
	}
      rewind(inf);	/* no fortran tags found, try C */
    }
  C_entries();
  fclose(inf);
}

/* Record a tag on the current line.
  name is the tag name,
  f is nonzero to use a pattern, zero to use line number instead. */

pfnote (name, f, linestart, linelen, lno, cno)
     char *name;
     logical f;			/* f == TRUE when function */
     char *linestart;
     int linelen;
     int lno;
     long cno;
{
  register char *fp;
  register NODE *np;
  char *altname;
  char tem[51];

  if ((np = (NODE *) malloc (sizeof (NODE))) == NULL)
    {
      fprintf(stderr, "ctags: too many entries to sort\n");
      put_entries(head);
      free_tree(head);
      head = NULL;
      np = (NODE *) xmalloc(sizeof (NODE));
    }
  /* Change name "main" to M<thisfilename>. */
  if (!eflag && !xflag && !strcmp(name, "main"))
    {
      fp = rindex(curfile, '/');
      if (fp == 0)
	fp = curfile;
      else
	fp++;
      altname = concat ("M", fp, "");
      fp = rindex(altname, '.');
      if (fp && fp[2] == 0)
	*fp = 0;
      name = altname;
    }
  np->name = savestr(name);
  np->file = curfile;
  np->f = f;
  np->lno = lno;
  np->cno = cno;
  np->left = np->right = 0;
  if (eflag)
    {
      linestart[linelen] = 0;
    }
  else if (xflag == 0)
    {
      sprintf (tem, strlen (linestart) < 50 ? "%s$" : "%.50s", linestart);
      linestart = tem;
    }
  np->pat = savestr (linestart);
  if (head == NULL)
    head = np;
  else
    add_node(np, head);
}

free_tree(node)
     NODE *node;
{
  while (node)
    {
      free_tree(node->right);
      free(node);
      node = node->left;
    }
}

add_node(node, cur_node)
     NODE *node,*cur_node;
{
  register int dif;

  dif = strcmp(node->name, cur_node->name);

  /* If this tag name matches an existing one, then
     unless -e was given, do not add the node, but maybe print a warning */
  if (!eflag && !dif)
    {
      if (node->file == cur_node->file)
	{
	  if (!wflag)
	    {
	      fprintf(stderr,"Duplicate entry in file %s, line %d: %s\n",
		      node->file,lineno,node->name);
	      fprintf(stderr,"Second entry ignored\n");
	    }
	  return;
	}
      if (!cur_node->been_warned)
	if (!wflag)
	  fprintf(stderr,"Duplicate entry in files %s and %s: %s (Warning only)\n",
		  node->file, cur_node->file, node->name);
      cur_node->been_warned = TRUE;
      return;
    } 

  /* Actually add the node */
  if (dif < 0) 
    {
      if (cur_node->left != NULL)
	add_node(node,cur_node->left);
      else
	cur_node->left = node;
      return;
    }
  if (cur_node->right != NULL)
    add_node(node,cur_node->right);
  else
    cur_node->right = node;
}

put_entries(node)
     reg NODE *node;
{
  reg char *sp;

  if (node == NULL)
    return;

  /* Output subentries that precede this one */
  put_entries (node->left);

  /* Output this entry */

  if (eflag)
    {
      fprintf (outf, "%s%c%d,%d\n",
	       node->pat, 0177, node->lno, node->cno);
    }
  else if (!xflag)
    {
      fprintf (outf, "%s\t%s\t",
	       node->name, node->file);

      if (node->f)
	{		/* a function */
	  putc (searchar, outf);
	  putc ('^', outf);

	  for (sp = node->pat; *sp; sp++)
	    {
	      if (*sp == '\\' || *sp == searchar)
		putc ('\\', outf);
	      putc (*sp, outf);
	    }
	  putc (searchar, outf);
	}
      else
	{		/* a typedef; text pattern inadequate */
	  fprintf (outf, "%d", node->lno);
	}
      putc ('\n', outf);
    }
  else if (vflag)
    fprintf (stdout, "%s %s %d\n",
	     node->name, node->file, (node->lno+63)/64);
  else
    fprintf (stdout, "%-16s%4d %-16s %s\n",
	     node->name, node->lno, node->file, node->pat);

  /* Output subentries that follow this one */
  put_entries (node->right);
}

/* Return total number of characters that put_entries will output for
 the nodes in the subtree of the specified node.
 Works only if eflag is set, but called only in that case.  */

total_size_of_entries(node)
     reg NODE *node;
{
  reg int total = 0;
  reg long num;

  if (node == NULL)
    return 0;

  /* Count subentries that precede this one */
  total = total_size_of_entries (node->left);

  /* Count subentries that follow this one */
  total += total_size_of_entries (node->right);

  /* Count this entry */

  total += strlen (node->pat) + 3;

  num = node->lno;
  while (num)
    {
      total++;
      num /= 10;
    }

  num = node->cno;
  if (!num) total++;
  while (num)
    {
      total++;
      num /= 10;
    }
  return total;
}

/*
 * This routine finds functions and typedefs in C syntax and adds them
 * to the list.
 */
#define CNL_SAVE_NUMBER \
{ \
  linecharno = charno; lineno++; \
  charno += 1 + readline (&lb, inf); \
  lp = lb.buffer; \
}

#define CNL \
{ \
  CNL_SAVE_NUMBER; \
  number = 0; \
}

C_entries ()
{
  register int c;
  register char *token, *tp, *lp;
  logical incomm, inquote, inchar, midtoken;
  int level;
  char tok[BUFSIZ];

  lineno = 0;
  charno = 0;
  lp = lb.buffer;
  *lp = 0;

  number = 0;
  gotone = midtoken = inquote = inchar = incomm = FALSE;
  level = 0;

  while (!feof (inf))
    {
      c = *lp++;
      if (c == 0)
	{
	  CNL;
	  gotone = FALSE;
	}
      if (c == '\\')
	{
	  c = *lp++;
	  if (c == 0)
	    CNL_SAVE_NUMBER;
	  c = ' ';
	} 
      else if (incomm)
	{
	  if (c == '*')
	    {
	      while ((c = *lp++) == '*')
		continue;
	      if (c == 0)
		CNL;
	      if (c == '/')
		incomm = FALSE;
	    }
	}
      else if (inquote)
	{
	  /*
	  * Too dumb to know about \" not being magic, but
	  * they usually occur in pairs anyway.
	  */
	  if (c == '"')
	    inquote = FALSE;
	  continue;
	}
      else if (inchar)
	{
	  if (c == '\'')
	    inchar = FALSE;
	  continue;
	}
      else switch (c)
	{
	case '"':
	  inquote = TRUE;
	  continue;
	case '\'':
	  inchar = TRUE;
	  continue;
	case '/':
	  if (*lp == '*')
	    {
	      lp++;
	      incomm = TRUE;
	    }
	  continue;
	case '#':
	  if (lp == lb.buffer + 1)
	    number = 1;
	  continue;
	case '{':
	  if (tydef == begin)
	    {
	      tydef=middle;
	    }
	  level++;
	  continue;
	case '}':
	  if (lp == lb.buffer + 1)
	    level = 0;	/* reset */
	  else
	    level--;
	  if (!level && tydef==middle)
	    {
	      tydef=end;
	    }
	  continue;
	}
      if (!level && !inquote && !incomm && gotone == FALSE)
	{
	  if (midtoken)
	    {
	      if (endtoken(c))
		{
		  int f;
		  char *buf = lb.buffer;
		  int endpos = lp - lb.buffer;
		  char *lp1 = lp;
		  int line = lineno;
		  long linestart = linecharno;
		  int tem = consider_token (&lp1, token, &f);
		  lp = lp1;
		  if (tem)
		    {
		      if (linestart != linecharno)
			{
			  getline (linestart);
			  strncpy (tok, token + (lb1.buffer - buf),
				   tp-token+1);
			  tok[tp-token+1] = 0;
			  pfnote(tok, f, lb1.buffer, endpos, line, linestart);
			}
		      else
			{
			  strncpy (tok, token, tp-token+1);
			  tok[tp-token+1] = 0;
			  pfnote(tok, f, lb.buffer, endpos, line, linestart);
			}
		      gotone = f;	/* function */
		    }
		  midtoken = FALSE;
		  token = lp - 1;
		}
	      else if (intoken(c))
		tp++;
	    }
	  else if (begtoken(c))
	    {
	      token = tp = lp - 1;
	      midtoken = TRUE;
	    }
	}
      if (c == ';'  &&  tydef==end)	/* clean with typedefs */
	tydef=none;
    }
}

/*
 * This routine  checks to see if the current token is
 * at the start of a function, or corresponds to a typedef
 * It updates the input line * so that the '(' will be
 * in it when it returns.
 */
consider_token (lpp, token, f)
     char **lpp, *token;
     int *f;
{
  reg char *lp = *lpp;
  reg char c;
  static logical next_token_is_func;
  logical firsttok;	/* T if have seen first token in ()'s */
  int bad, win;

  *f = 1;			/* a function */
  c = lp[-1];
  bad = FALSE;
  if (!number)
    {		/* space is not allowed in macro defs	*/
      while (iswhite(c))
	{
	  c = *lp++;
	  if (c == 0)
	    {
	      if (feof (inf))
		break;
	      CNL;
	    }
	}
      /* the following tries to make it so that a #define a b(c)	*/
      /* doesn't count as a define of b.				*/
    }
  else
    {
      number++;
      if (number >= 4  || (number==2 && strncmp (token, "define", 6)))
	{
	  gotone = TRUE;
	badone:
	  bad = TRUE;
	  goto ret;
	}
    }
  /* check for the typedef cases		*/
  if (tflag && !strncmp(token, "typedef", 7))
    {
      tydef=begin;
      goto badone;
    }
  if (tydef==begin && (!strncmp(token, "struct", 6) ||
		       !strncmp(token, "union", 5) || !strncmp(token, "enum", 4)))
  {
    goto badone;
  }
  if (tydef==begin)
    {
      tydef=end;
      goto badone;
    }
  if (tydef==end)
    {
      *f = 0;
      win = 1;
      goto ret;
    }
  /* Detect GNUmacs's function-defining macros. */
  if (!number && !strncmp (token, "DEF", 3))
	 
    {
      next_token_is_func = 1;
      goto badone;
    }
  if (next_token_is_func)
    {
      next_token_is_func = 0;
      win = 1;
      goto ret;
    }
  if (c != '(')
    goto badone;
  firsttok = FALSE;
  while ((c = *lp++) != ')')
    {
      if (c == 0)
	{
	  if (feof (inf))
	    break;
	  CNL;
	}
      /*
	* This line used to confuse ctags:
	*	int	(*oldhup)();
	* This fixes it. A nonwhite char before the first
	* token, other than a / (in case of a comment in there)
	* makes this not a declaration.
	*/
      if (begtoken(c) || c=='/') firsttok++;
      else if (!iswhite(c) && !firsttok) goto badone;
    }
  while (iswhite (c = *lp++))
    {
      if (c == 0)
	{
	  if (feof (inf))
	    break;
	  CNL;
	}
    }
  win = isgood (c);
ret:
  *lpp = lp - 1;
  return !bad && win;
}

getline (atchar)
     long atchar;
{
  long saveftell = ftell (inf);

  fseek (inf, atchar, 0);
  readline (&lb1, inf);
  fseek (inf, saveftell, 0);
}

/* Fortran parsing */

char	*dbp;
int	pfcnt;

PF_funcs(fi)
     FILE *fi;
{
  lineno = 0;
  charno = 0;
  pfcnt = 0;

  while (!feof (fi))
    {
      lineno++;
      linecharno = charno;
      charno += readline (&lb, fi) + 1;
      dbp = lb.buffer;
      if (*dbp == '%') dbp++ ;	/* Ratfor escape to fortran */
      while (isspace(*dbp))
	dbp++;
      if (*dbp == 0)
	continue;
      switch (*dbp |' ')
	{
	case 'i':
	  if (tail("integer"))
	    takeprec();
	  break;
	case 'r':
	  if (tail("real"))
	    takeprec();
	  break;
	case 'l':
	  if (tail("logical"))
	    takeprec();
	  break;
	case 'c':
	  if (tail("complex") || tail("character"))
	    takeprec();
	  break;
	case 'd':
	  if (tail("double"))
	    {
	      while (isspace(*dbp))
		dbp++;
	      if (*dbp == 0)
		continue;
	      if (tail("precision"))
		break;
	      continue;
	    }
	  break;
	}
      while (isspace(*dbp))
	dbp++;
      if (*dbp == 0)
	continue;
      switch (*dbp|' ')
	{
	case 'f':
	  if (tail("function"))
	    getit();
	  continue;
	case 's':
	  if (tail("subroutine"))
	    getit();
	  continue;
	case 'p':
	  if (tail("program"))
	    {
	      getit();
	      continue;
	    }
	  if (tail("procedure"))
	    getit();
	  continue;
	}
    }
  return (pfcnt);
}

tail(cp)
     char *cp;
{
  register int len = 0;

  while (*cp && (*cp&~' ') == ((*(dbp+len))&~' '))
    cp++, len++;
  if (*cp == 0)
    {
      dbp += len;
      return (1);
    }
  return (0);
}

takeprec()
{
  while (isspace(*dbp))
    dbp++;
  if (*dbp != '*')
    return;
  dbp++;
  while (isspace(*dbp))
    dbp++;
  if (!isdigit(*dbp))
    {
      --dbp;		/* force failure */
      return;
    }
  do
    dbp++;
  while (isdigit(*dbp));
}

getit()
{
  register char *cp;
  char c;
  char nambuf[BUFSIZ];

  while (isspace(*dbp))
    dbp++;
  if (*dbp == 0 || !isalpha(*dbp))
    return;
  for (cp = dbp+1; *cp && (isalpha(*cp) || isdigit(*cp)); cp++)
    continue;
  c = cp[0];
  cp[0] = 0;
  strcpy(nambuf, dbp);
  cp[0] = c;
  pfnote(nambuf, TRUE, lb.buffer, cp - lb.buffer + 1, lineno, linecharno);
  pfcnt++;
}

/*
 * lisp tag functions
 * just look for (def or (DEF
 */

L_funcs (fi)
     FILE *fi;
{
  lineno = 0;
  charno = 0;
  pfcnt = 0;

  while (!feof (fi))
    {
      lineno++;
      linecharno = charno;
      charno += readline (&lb, fi) + 1;
      dbp = lb.buffer;
      if (dbp[0] == '(' && 
	  (dbp[1] == 'D' || dbp[1] == 'd') &&
	    (dbp[2] == 'E' || dbp[2] == 'e') &&
	      (dbp[3] == 'F' || dbp[3] == 'f'))
	{
	  while (!isspace(*dbp)) dbp++;
	  while (isspace(*dbp)) dbp++;
	  L_getit();
	}
    }
}

L_getit()
{
  register char *cp;
  char c;
  char nambuf[BUFSIZ];

  if (*dbp == 0) return;
  for (cp = dbp+1; *cp && *cp != '(' && *cp != ' '; cp++)
    continue;
  c = cp[0];
  cp[0] = 0;
  strcpy(nambuf, dbp);
  cp[0] = c;
  pfnote(nambuf, TRUE, lb.buffer, cp - lb.buffer + 1, lineno, linecharno);
  pfcnt++;
}

/*
 * Scheme tag functions
 * look for (def... xyzzy
 * look for (def... (xyzzy
 * look for (def ... ((...(xyzzy ....
 * look for (set! xyzzy
 */

static get_scheme ();
Scheme_funcs (fi)
     FILE *fi;
{
  lineno = 0;
  charno = 0;
  pfcnt = 0;

  while (!feof (fi))
    {
      lineno++;
      linecharno = charno;
      charno += readline (&lb, fi) + 1;
      dbp = lb.buffer;
      if (dbp[0] == '(' && 
	  (dbp[1] == 'D' || dbp[1] == 'd') &&
	    (dbp[2] == 'E' || dbp[2] == 'e') &&
	      (dbp[3] == 'F' || dbp[3] == 'f'))
	{
	  while (!isspace(*dbp)) dbp++;
          /* Skip over open parens and white space */
          while (*dbp && (isspace(*dbp) || *dbp == '(')) dbp++;
	  get_scheme ();
	}
      if (dbp[0] == '(' && 
	  (dbp[1] == 'S' || dbp[1] == 's') &&
	    (dbp[2] == 'E' || dbp[2] == 'e') &&
	      (dbp[3] == 'T' || dbp[3] == 't') &&
                (dbp[4] == '!' || dbp[4] == '!') &&
                  (isspace(dbp[5])))
	{
	  while (!isspace(*dbp)) dbp++;
          /* Skip over white space */
          while (isspace(*dbp)) dbp++;
	  get_scheme ();
	}
    }
}

static
get_scheme()
{
  register char *cp;
  char c;
  char nambuf[BUFSIZ];

  if (*dbp == 0) return;
  /* Go till you get to white space or a syntactic break */
  for (cp = dbp+1; *cp && *cp != '(' && *cp != ')' && !isspace(*cp); cp++)
    continue;
  /* Null terminate the string there. */
  c = cp[0];
  cp[0] = 0;
  /* Copy the string */
  strcpy(nambuf, dbp);
  /* Unterminate the string */
  cp[0] = c;
  /* Announce the change */
  pfnote(nambuf, TRUE, lb.buffer, cp - lb.buffer + 1, lineno, linecharno);
  pfcnt++;
}

/* Find tags in TeX and LaTeX input files.  */

/* TEX_toktab is a table of TeX control sequences that define tags.
   Each TEX_tabent records one such control sequence.  */

struct TEX_tabent
{
  char *name;
  int len;
};

struct TEX_tabent *TEX_toktab = NULL; /* Table with tag tokens */

/* Default set of control sequences to put into TEX_toktab.
   The value of environment var TEXTAGS is prepended to this.  */

static char *TEX_defenv =
  ":chapter:section:subsection:subsubsection:eqno:label:ref:cite:bibitem:typeout";

struct TEX_tabent *TEX_decode_env (); 

static char TEX_esc = '\\';
static char TEX_opgrp = '\{';
static char TEX_clgrp = '\}';

/*
 * TeX/LaTeX scanning loop.
 */

TEX_funcs (fi)
    FILE *fi;
{
  char *lasthit;

  lineno = 0;
  charno = 0;
  pfcnt = 0;

  /* Select either \ or ! as escape character.  */
  TEX_mode (fi);

  /* Initialize token table once from environment. */
  if (!TEX_toktab)
    TEX_toktab = TEX_decode_env ("TEXTAGS", TEX_defenv);

  while (!feof (fi))
    {
      lineno++;
      linecharno = charno;
      charno += readline (&lb, fi) + 1;
      dbp = lb.buffer;
      lasthit = dbp;

      while (!feof (fi))
	{	/* Scan each line in file */
	  lineno++;
	  linecharno = charno;
	  charno += readline (&lb, fi) + 1;
	  dbp = lb.buffer;
	  lasthit = dbp;
	  while (dbp = index (dbp, TEX_esc)) /* Look at each escape in line */
	    {
	      register int i;

	      if (! *(++dbp))
		break;
	      linecharno += dbp - lasthit;
	      lasthit = dbp;
	      i = TEX_Token (lasthit);
	      if (0 <= i)
		{
		  TEX_getit (lasthit, TEX_toktab[i].len);
		  break;		/* We only save a line once */
		}
	    }
	}
    }
}

#define TEX_LESC '\\'
#define TEX_SESC '\!'

/* Figure out whether TeX's escapechar is '\\' or '!' and set grouping */
/* chars accordingly. */

TEX_mode (f)
     FILE *f;
{
  int c;

  while ((c = getc (f)) != EOF)
    if (c == TEX_LESC || c == TEX_SESC)
      break;

  if (c == TEX_LESC)
    {
      TEX_esc = TEX_LESC;
      TEX_opgrp = '\{';
      TEX_clgrp = '\}';
    } 
  else
    {
      TEX_esc = TEX_SESC;
      TEX_opgrp = '\<';
      TEX_clgrp = '\>';
    }
  rewind (f);
}

/* Read environment and prepend it to the default string. */
/* Build token table. */

struct TEX_tabent *
TEX_decode_env (evarname, defenv)
     char *evarname;
     char *defenv;
{
  register char *env, *p;
  extern char *savenstr (), *index ();

  struct TEX_tabent *tab;
  int size, i;

  /* Append deafult string to environment. */
  env = (char *) getenv (evarname);
  if (!env)
    env = defenv;
  else
    env = concat (env, defenv, "");

  /* Allocate a token table */
  for (size = 1, p=env; p;)
    if ((p = index (p, ':')) && *(++p))
      size++;
  tab = (struct TEX_tabent *) xmalloc (size * sizeof (struct TEX_tabent));

  /* Unpack environment string into token table. Be careful about */
  /* zero-length strings (leading ':', "::" and trailing ':') */
  for (i = 0; *env;)
    {
      p = index (env, ':');
      if (!p)			/* End of environment string. */
	p = env + strlen (env);
      if (p - env > 0)
	{	/* Only non-zero strings. */
	  tab[i].name = savenstr (env, p - env);
	  tab[i].len = strlen (tab[i].name);
	  i++;
	}
      if (*p)
	env = p + 1;
      else
	{
	  tab[i].name = NULL;	/* Mark end of table. */
	  tab[i].len = 0;
	  break;
	}
    }
  return tab;
}

/* Record a tag defined by a TeX command of length LEN and starting at NAME.
   The name being defined actually starts at (NAME + LEN + 1).
   But we seem to include the TeX command in the tag name.  */

TEX_getit (name, len)
    char *name;
    int len;
{
  char *p = name + len;
  char nambuf[BUFSIZ];

  if (*name == 0) return;

  /* Let tag name extend to next group close (or end of line) */
  while (*p && *p != TEX_clgrp)
    p++;
  strncpy (nambuf, name, p - name);
  nambuf[p - name] = 0;

  pfnote (nambuf, TRUE, lb.buffer, strlen (lb.buffer), lineno, linecharno);
  pfcnt++;
}

/* If the text at CP matches one of the tag-defining TeX command names,
   return the index of that command in TEX_toktab.
   Otherwise return -1.  */

/* Keep the capital `T' in `Token' for dumb truncating compilers
   (this distinguishes it from `TEX_toktab' */
TEX_Token (cp)
    char *cp;
{
  int i;

  for (i = 0; TEX_toktab[i].len > 0; i++)
    if (strncmp (TEX_toktab[i].name, cp, TEX_toktab[i].len) == 0)
      return i;
  return -1;
}

/* Initialize a linebuffer for use */

void
initbuffer (linebuffer)
     struct linebuffer *linebuffer;
{
  linebuffer->size = 200;
  linebuffer->buffer = (char *) xmalloc (200);
}

/* Read a line of text from `stream' into `linebuffer'.
 Return the length of the line.  */

long
readline (linebuffer, stream)
     struct linebuffer *linebuffer;
     register FILE *stream;
{
  char *buffer = linebuffer->buffer;
  register char *p = linebuffer->buffer;
  register char *pend = p + linebuffer->size;

  while (1)
    {
      int c = getc (stream);
      if (p == pend)
	{
	  buffer = (char *) xrealloc (buffer, linebuffer->size *= 2);
	  p += buffer - linebuffer->buffer;
	  pend = buffer + linebuffer->size;
	  linebuffer->buffer = buffer;
	}
      if (c < 0 || c == '\n')
	{
	  *p = 0;
	  break;
	}
      *p++ = c;
    }

  return p - buffer;
}

char *
savestr(cp)
     char *cp;
{
  return savenstr (cp, strlen (cp));
}

char *
savenstr(cp, len)
    char *cp;
    int len;
{
  register char *dp;

  dp = (char *) xmalloc (len + 1);
  strncpy (dp, cp, len);
  dp[len] = '\0';
  return dp;
}

/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
 *
 * Identical to v7 rindex, included for portability.
 */

char *
rindex(sp, c)
     register char *sp, c;
{
  register char *r;

  r = NULL;
  do
    {
      if (*sp == c)
	r = sp;
    } while (*sp++);
  return(r);
}

/*
 * Return the ptr in sp at which the character c first
 * appears; NULL if not found
 *
 * Identical to v7 index, included for portability.
 */

char *
index(sp, c)
     register char *sp, c;
{
  do
    {
      if (*sp == c)
	return (sp);
    } while (*sp++);
  return (NULL);
}

/* Print error message and exit.  */

fatal (s1, s2)
     char *s1, *s2;
{
  error (s1, s2);
  exit (1);
}

/* Print error message.  `s1' is printf control string, `s2' is arg for it. */

error (s1, s2)
     char *s1, *s2;
{
#ifdef CTAGS
  printf ("ctags: ");
#else
  printf ("etags: ");
#endif
  printf (s1, s2);
  printf ("\n");
}

/* Return a newly-allocated string whose contents concatenate those of s1, s2, s3.  */

char *
concat (s1, s2, s3)
     char *s1, *s2, *s3;
{
  int len1 = strlen (s1), len2 = strlen (s2), len3 = strlen (s3);
  char *result = (char *) xmalloc (len1 + len2 + len3 + 1);

  strcpy (result, s1);
  strcpy (result + len1, s2);
  strcpy (result + len1 + len2, s3);
  *(result + len1 + len2 + len3) = 0;

  return result;
}

/* Like malloc but get fatal error if memory is exhausted.  */

int
xmalloc (size)
     int size;
{
  int result = malloc (size);
  if (!result)
    fatal ("virtual memory exhausted", 0);
  return result;
}

int
xrealloc (ptr, size)
     char *ptr;
     int size;
{
  int result = realloc (ptr, size);
  if (!result)
    fatal ("virtual memory exhausted");
  return result;
}
