/*
 * EXECOM.C
 *
 * Matthew Dillon, 10 August 1986
 *    Finally re-written.
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 *
 */

#include "shell.h"

#define F_EXACT 0
#define F_ABBR	1

#define ST_COND	  0x03
#define ST_NAME	  0x02
#define ST_NOEXP  0x04

int has_wild = 0;		  /* set if any arg has wild card */

struct COMMAND {
   int (*func)();
   short minargs;
   short stat;
   int	 val;
   char *name;
};

extern char *format_insert_string();
extern char *mpush(), *exarg();

extern int do_run(), do_number();
extern int do_quit(), do_set_var(), do_unset_var();
extern int do_echo(), do_source(), do_mv();
extern int do_cd(), do_pwd(), do_rm(), do_mkdir(), do_history();
extern int do_mem(), do_cat(), do_dir(), do_devinfo(), do_inc();
extern int do_foreach(), do_return(), do_if(), do_label(), do_goto();
extern int do_input(), do_ver(), do_sleep(), do_help();
extern int do_strhead(), do_strtail();
extern int do_copy(), date(),  do_ps();
extern int do_forever(), do_abortline();

static struct COMMAND Command[] = {
   do_run      , 0,  0,		 0 ,   "\001",
   do_number   , 0,  0,		 0 ,   "\001",
   do_set_var  , 0,  0, LEVEL_ALIAS,   "alias",
   do_abortline, 0,  0,		 0,    "abortline",
   do_cd       , 0,  0,		 0 ,   "cd",
   do_cat      , 0,  0,		 0 ,   "cat",
   do_copy     , 1,  0,		 0 ,   "copy",
   date	       , 0,  0,		 0 ,   "date",
   do_dir      , 0,  ST_NOEXP,	 0 ,   "dir",
   do_inc      , 1,  0,		-1 ,   "dec",
   do_devinfo  , 0,  0,		 0 ,   "devinfo",
   do_echo     , 0,  0,		 0 ,   "echo",
   do_if       , 0,  ST_COND,	 1 ,   "else",
   do_if       , 0,  ST_COND,	 2 ,   "endif",
   do_foreach  , 3,  0,		 0 ,   "foreach",
   do_forever  , 1,  0,		 0 ,   "forever",
   do_goto     , 1,  0,		 0 ,   "goto",
   do_help     , 0,  0,		 0 ,   "help",
   do_history  , 0,  0,		 0 ,   "history",
   do_if       , 1,  ST_COND,	 0 ,   "if",
   do_inc      , 1,  0,		 1 ,   "inc",
   do_input    , 1,  0,		 0 ,   "input",
   do_label    , 1,  ST_COND,	 0 ,   "label",
   do_mem      , 0,  0,		 0 ,   "mem",
   do_mkdir    , 0,  0,		 0 ,   "mkdir",
   do_mv       , 2,  0,		 0 ,   "mv",
   do_ps       , 0,  0,		 0,    "ps",
   do_pwd      , 0,  0,		 0 ,   "pwd",
   do_quit     , 0,  0,		 0 ,   "quit",
   do_return   , 0,  0,		 0 ,   "return",
   do_rm       , 0,  0,		 0 ,   "rm",
   do_run      , 1,  ST_NAME,	 0 ,   "run",
   do_set_var  , 0,  0, LEVEL_SET  ,   "set",
   do_sleep    , 0,  0,		 0,    "sleep",
   do_source   , 0,  0,		 0 ,   "source",
   do_strhead  , 3,  0,		 0 ,   "strhead",
   do_strtail  , 3,  0,		 0 ,   "strtail",
   do_unset_var, 0,  0, LEVEL_ALIAS,   "unalias",
   do_unset_var, 0,  0, LEVEL_SET  ,   "unset",
   do_ver      , 0,  0,		 0 ,   "version",
   '\0'	       , 0,  0,		 0 ,   NULL
};


static unsigned char elast;	     /* last end delimeter */
static char Cin_ispipe, Cout_ispipe;

exec_command(base)
char *base;
{
   register char *scr;
   register int i;
   char buf[32];

   if (!H_stack) {
      add_history(base);
      sprintf(buf, "%d", H_tail_base + H_len);
      set_var(LEVEL_SET, V_HISTNUM, buf);
   }
   scr = malloc((strlen(base) << 2) + 2);    /* 4X */
   preformat(base, scr);
   i = fcomm(scr, 1);
   return ((i) ? -1 : 1);
}

isalphanum(c)
char c;
{
   if (c >= '0' && c <= '9')
      return (1);
   if (c >= 'a' && c <= 'z')
      return (1);
   if (c >= 'A' && c <= 'Z')
      return (1);
   if (c == '_')
      return (1);
   return (0);
}

preformat(s, d)
register char *s, *d;
{
   register int si, di, qm;

   si = di = qm = 0;
   while (s[si] == ' ' || s[si] == 9)
      ++si;
   while (s[si]) {
      if (qm && s[si] != '\"' && s[si] != '\\') {
	 d[di++] = s[si++] | 0x80;
	 continue;
      }
      switch (s[si]) {
      case ' ':
      case 9:
	 d[di++] = ' ';
	 while (s[si] == ' ' || s[si] == 9)
	    ++si;
	 if (s[si] == 0 || s[si] == '|' || s[si] == ';')
	    --di;
	 break;
      case '*':
      case '?':
	 d[di++] = 0x80;
      case '!':
	 d[di++] = s[si++];
	 break;
      case '#':
	 d[di++] = '\0';
	 while (s[si])
	    ++si;
	 break;
      case ';':
      case '|':
	 d[di++] = s[si++];
	 while (s[si] == ' ' || s[si] == 9)
	    ++si;
	 break;
      case '\\':
	 d[di++] = s[++si] | 0x80;
	 if (s[si]) ++si;
	 break;
      case '\"':
	 qm = 1 - qm;
	 ++si;
	 break;
      case '^':
	 d[di++] = s[++si] & 0x1F;
	 if (s[si]) ++si;
	 break;
      case '$':		/* search end of var name and place false space */
	 d[di++] = 0x80;
	 d[di++] = s[si++];
	 while (isalphanum(s[si]))
	    d[di++] = s[si++];
	 d[di++] = 0x80;
	 break;
      default:
	 d[di++] = s[si++];
	 break;
      }
   }
   d[di++] = 0;
   d[di]   = 0;
   if (debug & 0x01) {
      fprintf (stderr,"PREFORMAT: %d :%s:\n", strlen(d), d);
   }
}

/*
 * process formatted string.  ' ' is the delimeter.
 *
 *    0: check '\0': no more, stop, done.
 *    1: check $.     if so, extract, format, insert
 *    2: check alias. if so, extract, format, insert. goto 1
 *    3: check history or substitution, extract, format, insert. goto 1
 *
 *    4: assume first element now internal or disk based command.
 *
 *    5: extract each ' ' or 0x80 delimited argument and process, placing
 *	 in av[] list (except 0x80 args appended).  check in order:
 *
 *	       '$'	   insert string straight
 *	       '>'	   setup stdout
 *	       '>>'	   setup stdout flag for append
 *	       '<'	   setup stdin
 *	       '*' or '?'  do directory search and insert as separate args.
 *
 *	       ';' 0 '|'   end of command.  if '|' setup stdout
 *			    -execute command, fix stdin and out (|) sets
 *			     up stdin for next guy.
 */


fcomm(str, freeok)
register char *str;
{
   static int alias_count;
   int p_alias_count = 0;
   char *istr;
   char *nextstr;
   char *command;
   char *pend_alias = NULL;
   char err = 0;
   has_wild = 0;
   ++alias_count;

   mpush_base();
   if (*str == 0)
      goto done1;
step1:
   if (alias_count == MAXALIAS || ++p_alias_count == MAXALIAS) {
      fprintf(stderr,"Alias Loop\n");
      err = 20;
      goto done1;
   }
   if (*str == '$') {
      if (istr = get_var (LEVEL_SET, str + 1))
	 str = format_insert_string(str, istr, &freeok);
   }
   istr = NULL;
   if (*(unsigned char *)str < 0x80)
      istr = get_var (LEVEL_ALIAS, str);  /* only if not \command */
   *str &= 0x7F;			  /* remove \ teltail	  */
   if (istr) {
      if (*istr == '%') {
	 pend_alias = istr;
      } else {
	 str = format_insert_string(str, istr, &freeok);
	 goto step1;
      }
   }
   if (*str == '!') {
      char *p, c;		      /* fix to allow !cmd1;!cmd2 */
      for(p = str; *p && *p != ';' ; ++p);
      c = *p;
      *p = '\0';
      istr = get_history(str);
      *p = c;
      replace_head(istr);
      str = format_insert_string(str, istr, &freeok);
      goto step1;
   }
   nextstr = str;
   command = exarg(&nextstr);
   if (*command == 0)
      goto done0;
   if (pend_alias == 0) {
      register int ccno;
      ccno = find_command(command);
      if (Command[ccno].stat & ST_COND)
	 goto skipgood;
   }
   if (disable) {
      while (elast && elast != ';' && elast != '|')
	 exarg(&nextstr);
      goto done0;
   }
skipgood:
   {
      register char *arg, *ptr, *scr;
      short redir;
      short doexpand;
      short cont;
      short inc;

      ac = 1;
      av[0] = command;
step5:						/* ac = nextac */
      if (!elast || elast == ';' || elast == '|')
	 goto stepdone;

      av[ac] = '\0';
      cont = 1;
      doexpand = redir = inc = 0;

      while (cont && elast) {
	 int ccno = find_command(command);
	 ptr = exarg(&nextstr);
	 inc = 1;
	 arg = "";
	 cont = (elast == 0x80);
	 switch (*ptr) {
	 case '<':
	    redir = -2;
	 case '>':
	    if (Command[ccno].stat & ST_NAME) {
							/* don't extract   */
		redir = 0;				/* <> stuff if its */
		arg = ptr;				/* external cmd.   */
		break;
	    }
	    ++redir;
	    arg = ptr + 1;
	    if (*arg == '>') {
	       redir = 2;	 /* append >> (not impl yet) */
	       ++arg;
	    }
	    cont = 1;
	    break;
	 case '$':
	    if ((arg = get_var(LEVEL_SET, ptr + 1)) == NULL)
	       arg = ptr;
	    break;
	 case '*':
	 case '?':
	    if ((Command[ccno].stat & ST_NOEXP) == 0)
	       doexpand = 1;
	    arg = ptr;
	    break;
	 default:
	    arg = ptr;
	    break;
	 }

	 /* Append arg to av[ac] */

	 for (scr = arg; *scr; ++scr)
	    *scr &= 0x7F;
	 if (av[ac]) {
	    register char *old = av[ac];
	    av[ac] = mpush(strlen(arg)+1+strlen(av[ac]));
	    strcpy(av[ac], old);
	    strcat(av[ac], arg);
	 } else {
	    av[ac] = mpush(strlen(arg)+1);
	    strcpy(av[ac], arg);
	 }
	 if (elast != 0x80)
	    break;
      }

      /* process expansion */

      if (doexpand) {
	 char **eav, **ebase;
	 int eac;
	 has_wild = 1;
	 eav = ebase = expand(av[ac], &eac);
	 inc = 0;
	 if (eav) {
	    if (ac + eac + 2 > MAXAV) {
	       ierror (NULL, 506);
	       err = 1;
	    } else {
	       QuickSort(eav, eac);
	       for (; eac; --eac, ++eav)
		  av[ac++] = strcpy(mpush(strlen(*eav)+1), *eav);
	    }
	    free_expand (ebase);
	 }
      }

      /* process redirection  */

      if (redir && !err) {
	 register char *file = (doexpand) ? av[--ac] : av[ac];

	 if (redir < 0)
	    Cin_name = file;
	 else {
	    Cout_name = file;
	    Cout_append = (redir == 2);
	 }
	 inc = 0;
      }

      /* check elast for space */

      if (inc) {
	 ++ac;
	 if (ac + 2 > MAXAV) {
	    ierror (NULL, 506);
	    err = 1;		    /* error condition */
	    elast = 0;		    /* don't process any more arguemnts */
	 }
      }
      if (elast == ' ')
	 goto step5;
   }
stepdone:
   av[ac] = '\0';

   /* process pipes via files */

   if (elast == '|' && !err) {
      static int which;		    /* 0 or 1 in case of multiple pipes */
      which = 1 - which;
      Cout_name = (which) ? Pipe1 : Pipe2;
      Cout_ispipe = 1;
   }


   if (err)
      goto done0;

   {
      register int i, len;
      char save_elast;
      register char *avline;

      save_elast = elast;
      for (i = len = 0; i < ac; ++i)
	 len += strlen(av[i]) + 1;
      avline = malloc(len+1);
      for (len = 0, i = ((pend_alias) ? 1 : 0); i < ac; ++i) {
	 if (debug & 0x02) {
	     fprintf (stderr, "AV[%2d] %d :%s:\n", i, strlen(av[i]), av[i]);
	 }
	 strcpy(avline + len, av[i]);
	 len += strlen(av[i]);
	 if (i + 1 < ac)
	    avline[len++] = ' ';
      }
      avline[len] = 0;
      if (pend_alias) {				      /* special % alias */
	 register char *ptr, *scr;
	 for (ptr = pend_alias; *ptr && *ptr != ' '; ++ptr);
	 set_var (LEVEL_SET, pend_alias + 1, avline);
	 free (avline);

	 scr = malloc((strlen(ptr) << 2) + 2);
	 preformat (ptr, scr);
	 fcomm (scr, 1);
	 unset_var (LEVEL_SET, pend_alias + 1);
      } else {					      /* normal command	 */
	 register int ccno;
	 long  oldcin = (long)Input();
	 long  oldcout = (long)Output();

#ifndef AZTEC_C
	 extern struct _dev _devtab[];
#endif
	 struct _dev *stdfp;

	 fflush(stdout);
	 ccno = find_command (command);
	 if ((Command[ccno].stat & ST_NAME) == 0) {
	    if (Cin_name) {
	       if ((Cin = (long)Open(Cin_name,1005L)) == 0L) {
		  ierror (NULL, 504);
		  err = 1;
		  Cin_name = '\0';
	       } else {
		  Myprocess->pr_CIS = Cin;
		  _devtab[stdin->_unit].fd = Cin;
	       }
	    }
	    if (Cout_name) {
	       if (Cout_append) {
		  if ((Cout = (long)Open(Cout_name, 1005L)) != 0L)
		     Seek(Cout, 0L, 1L);
	       } else {
		  Cout = (long)Open(Cout_name,1006L);
	       }
	       if (Cout == NULL) {
		  err = 1;
		  ierror (NULL, 504);
		  Cout_name = '\0';
		  Cout_append = 0;
	       } else {
		  Myprocess->pr_COS = Cout;
		  _devtab[stdout->_unit].fd = Cout;
	       }
	    }
	 }
	 if (ac < Command[ccno].minargs + 1) {
	    ierror (NULL, 500);
	    err = -1;
	 } else if (!err) {
	    i = (*Command[ccno].func)(avline, Command[ccno].val);
	    if (i < 0)
	       i = 20;
	    err = i;
	 }
	 free (avline);
	 if (Exec_ignoreresult == 0 && Lastresult != err) {
	    Lastresult = err;
	    seterr();
	 }
	 if ((Command[ccno].stat & ST_NAME) == 0) {
	    if (Cin_name) {
	       fflush(stdin);
	       clearerr(stdin);
	       Close(Cin);
	    }
	    if (Cout_name) {
	       fflush(stdout);
	       clearerr(stdout);
	       stdout->_flags &= ~_DIRTY;    /* because of nil: device */
	       Close(Cout);
	       Cout_append = 0;
	    }
	 }

	 /* the next few lines solve a bug with fexecv and bcpl programs */
	 /* that muck up the input/output streams  which causes GURUs	*/

	 Myprocess->pr_CIS =  _devtab[stdin->_unit].fd = oldcin;
	 Myprocess->pr_COS =  _devtab[stdout->_unit].fd = oldcout;
      }
      if (Cin_ispipe && Cin_name)
	 DeleteFile(Cin_name);
      if (Cout_ispipe) {
	 Cin_name = Cout_name;	       /* ok to assign.. static name */
	 Cin_ispipe = 1;
      } else {
	 Cin_name = '\0';
      }
      Cout_name = '\0';
      Cout_ispipe = 0;
      elast = save_elast;
   }
   mpop_tobase();		       /* free arguments   */
   mpush_base();		       /* push dummy base  */

done0:
   {
      char *str;
      if (err && E_stack == 0) {
	 str = get_var(LEVEL_SET, V_EXCEPT);
	 if (err >= ((str)?atoi(str):1)) {
	    if (str) {
	       ++H_stack;
	       ++E_stack;
	       exec_command(str);
	       --E_stack;
	       --H_stack;
	    } else {
	       Exec_abortline = 1;
	    }
	 }
      }
      if (elast != 0 && Exec_abortline == 0)
	 err = fcomm(nextstr, 0);
      Exec_abortline = 0;
      if (Cin_name)
	 DeleteFile(Cin_name);
      Cin_name = NULL;
      Cin_ispipe = 0;
   }
done1:
   mpop_tobase();
   if (freeok)
      free(str);
   --alias_count;
   return ((int)err);		       /* TRUE = error occured	  */
}


char *
exarg(ptr)
unsigned char **ptr;
{
   register unsigned char *end;
   register unsigned char *start;

   start = end = *ptr;
   while (*end && *end != 0x80 && *end != ';' && *end != '|' && *end != ' ')
      ++end;
   elast = *end;
   *end = '\0';
   *ptr = end + 1;
   return ((char *)start);
}

static char **Mlist;

mpush_base()
{
   char *str;

   str = malloc(5);
   *(char ***)str = Mlist;
   str[4] = 0;
   Mlist = (char **)str;
}

char *
mpush(bytes)
{
   char *str;

   str = malloc(5 + bytes + 2);   /* may need 2 extra bytes in do_run() */
   *(char ***)str = Mlist;
   str[4] = 1;
   Mlist = (char **)str;
   return (str + 5);
}

mpop_tobase()
{
   register char *next;
   while (Mlist) {
      next = *Mlist;
      if (((char *)Mlist)[4] == 0) {
	 free (Mlist);
	 Mlist = (char **)next;
	 break;
      }
      free (Mlist);
      Mlist = (char **)next;
   }
}


/*
 * Insert 'from' string in front of 'str' while deleting the
 * first entry in 'str'.  if freeok is set, then 'str' will be
 * free'd
 */



char *
format_insert_string(str, from, freeok)
char *str;
char *from;
int *freeok;
{
   register char *new1, *new2;
   register unsigned char *strskip;
   int len;

   for (strskip = (unsigned char *)str; *strskip && *strskip != ' ' && *strskip != ';' && *strskip != '|' && *strskip != 0x80; ++strskip);
   len = strlen(from);
   new1 = malloc((len << 2) + 2);
   preformat(from, new1);
   len = strlen(new1) + strlen(strskip);
   new2 = malloc(len+2);
   strcpy(new2, new1);
   strcat(new2, strskip);
   new2[len+1] = 0;
   free (new1);
   if (*freeok)
      free (str);
   *freeok = 1;
   return (new2);
}

find_command(str)
char *str;
{
   int i;
   int len = strlen(str);

   if (*str >= '0'  &&	*str <= '9')
      return (1);
   for (i = 0; Command[i].func; ++i) {
      if (strncmp (str, Command[i].name, len) == 0)
	 return (i);
   }
   return (0);
}

do_help()
{
   register struct COMMAND *com;
   int i= 0;


   for (com = &Command[2]; com->func; ++com) {
      printf ("%-12s", com->name);
      if (++i  % 6 == 0) printf("\n");
   }
   printf("\n");
   return(0);
}

