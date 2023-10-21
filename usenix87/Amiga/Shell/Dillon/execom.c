
/*
 * EXECOM.C
 *
 * Matthew Dillon, 24 Feb 1986
 *
 *    This code is particular hacked up and needs re-writing.
 *
 */

#include "shell.h"


struct COMMAND {
   int (*func)();
   int stat;
   int val;
   char *name;
};

struct MLIST {
   struct MLIST *next;
};

#define F_EXACT   0
#define F_ABBR    1

extern char *breakout();

extern int do_run(), do_number();
extern int do_quit(), do_set_var(), do_unset_var();
extern int do_echo(), do_source(), do_mv();
extern int do_cd(), do_rm(), do_mkdir(), do_history();
extern int do_mem(), do_cat(), do_dir();
extern int do_foreach();

static struct MLIST *Mlist;

static struct COMMAND Command[] = {
   do_run      ,  0,          0 ,   "\001",
   do_number   ,  0,          0 ,   "\001",
   do_quit     ,  0,          0 ,   "quit",
   do_set_var  ,  0, LEVEL_SET  ,   "set",
   do_unset_var,  0, LEVEL_SET  ,   "unset",
   do_set_var  ,  0, LEVEL_ALIAS,   "alias",
   do_unset_var,  0, LEVEL_ALIAS,   "unalias",
   do_echo     ,  0,          0 ,   "echo",
   do_source   ,  0,          0 ,   "source",
   do_mv       ,  0,          0 ,   "mv",
   do_cd       ,  0,          0 ,   "cd",
   do_rm       ,  0,          0 ,   "rm",
   do_mkdir    ,  0,          0 ,   "mkdir",
   do_history  ,  0,          0 ,   "history",
   do_mem      ,  0,          0 ,   "mem",
   do_cat      ,  0,          0 ,   "cat",
   do_dir      ,  0,          0 ,   "dir",
   do_dir      ,  0,         -1 ,   "devinfo",
   do_foreach  ,  0,          0 ,   "foreach",
   NULL        ,  0,          0 ,   NULL };

static char *
mpush(amount)
{
   struct MLIST *ml;

   ml = (struct MLIST *)malloc (amount + sizeof(*Mlist));
   ml->next = Mlist;
   Mlist = ml;
   return ((char *)Mlist + sizeof(*Mlist));
}

static char *
mpop()
{
   char *old = NULL;

   if (Mlist == NULL) {
      puts ("Internal MLIST error");
      fflush (stdout);
   } else {
      old = (char *)Mlist + sizeof(*Mlist);
      Free (Mlist);
      Mlist = Mlist->next;
   }
   return (old);
}

mrm()
{
   while (Mlist) {
      Free (Mlist);
      Mlist = Mlist->next;
   }
}

exec_command(base)
char *base;
{
   char *str;
   int i;
   if (!H_stack)
      add_history(base);
   strcpy (str = mpush(strlen(base) + 1), base);
   i = e_command(str);
   if (mpop() != str) {
      puts ("POP ERROR");
      fflush (stdout);
   }
   return (1);
}

static
e_command(base)
char *base;
{
   char *com, *start, *avline, *alias;
   char *s1, *s2;
   int flag = 0;
   int i, pcount, len, ccno;
loop:
   com = breakout (&base, &flag);
   if (*com == '\0') {
      if (flag & FL_EOL)
         return (1);
      goto loop;
   }
   alias = NULL;
   if ((ccno = find_command(com, F_EXACT)) < 0) {
      switch (flag & FL_MASK) {
      case FL_BANG:
         alias = get_history (com);
         replace_head (alias, base, flag);
         break;
      case FL_DOLLAR:
         alias = get_var (LEVEL_SET, com + 1);
         break;
      default:
         alias = get_var (LEVEL_ALIAS, com);
         break;
      }
      if (alias == NULL) {
         if ((ccno = find_command (com, F_ABBR)) < 0) {
            printf ("%s Command Not Found\n", com);
            return (-1);
         } else {
            goto good_command;
         }
      }

      if (*alias == '%')
         goto good_command;

      start = (!(flag & FL_EOC)) ? base : "";
      while (!(flag & FL_EOC)) {
         flag = FL_OVERIDE;
         breakout (&base, &flag);
      }

      com = mpush (strlen(alias) + strlen(start) + 2);
      strcpy (com, alias);
      strcat (com, " ");
      strcat (com, start);
      i = e_command (com);
      if (mpop() != com)
         puts ("ME BAE ERROR");
      if (i < 0)
         return (-1);
      if (flag & FL_EOL)
         return (1);
      goto loop;
   }
good_command:
   pcount = 0;
   i = pcount = 0;
   av[i] = mpush (strlen(com) + 1);
   ++pcount;
   strcpy (av[i++], com);
   while (!(flag & FL_EOC)) {
      com = breakout (&base, &flag);
      if (*com == '\0')
         continue;
      switch (flag & FL_MASK) {
      case FL_DOLLAR:
         av[i] = get_var (LEVEL_SET, com + 1);
         if (av[i] == NULL)
            av[i] = com;
         av[i] = strcpy(mpush (strlen(av[i]) + 1), av[i]);
         ++pcount;
         break;
      case FL_QUOTE:
      default:
         av[i] = com;
         break;
      }
      if (flag & FL_IDOLLAR) {
         for (s1 = av[i]; *s1 && *s1 != '$'; ++s1);
         if (*s1) {
            *s1 = '\0';
            s1 = get_var (LEVEL_SET, s1 + 1);
            if (s1) {
               register char *scr_str = mpush(strlen(av[i])+strlen(s1)+1);
               ++pcount;
               strcpy (scr_str, av[i]);
               strcat (scr_str, s1);
               av[i] = scr_str;
            }
         }
      }
      if (flag & FL_WILD) {   /*  av[i] has a wild card, expand it */
         int eac;
         char **eav, **ebase;

         eav = ebase = expand (av[i], &eac);   /* returns malloc'd av list */
         if (eav == NULL) {
            puts ("Null expansion");
            goto fail;
         }
         if (i + eac > MAXAV - 2) {
            free_expand (ebase);
            goto avovr;
         }
         for (; eac; --eac, ++eav) {
            av[i++] = strcpy(mpush(strlen(*eav)+1), *eav);
            ++pcount;
         }
         --i;
         free_expand (ebase);
      }
      if (++i > MAXAV - 2) {
avovr:
         puts ("AV overflow");
         goto fail;
      }
   }
   av[i] = NULL;
   ac = i;
   for (len = 0, i = 0; i < ac; ++i)
      len += strlen(av[i]) + 1;
   avline = mpush (len + 1);
   *avline = '\0';
   for (i = 0; i < ac; ++i) {
      strcat (avline, av[i]);
      if (i + 1 < ac)
         strcat (avline, " ");
   }
   if (*alias) {
      for (s2 = alias; *s2 && *s2 != ' '; ++s2);
      if (*s2) {
         *s2 = '\0';
         set_var (LEVEL_SET, alias + 1, next_word(avline));
         *s2 = ' ';
         s1 = strcpy(mpush(strlen(s2)+1), s2);
         i = e_command (s1);
         if (mpop() != s1)
            puts ("AL-LINE ERROR");
      }
   } else {
      i = (*Command[ccno].func)(avline, Command[ccno].val);
   }
   if (mpop() != avline) {
      puts ("AVLINE ERROR");
fail:
      i = -1;
   }
   while (pcount--)
      mpop();
   if (i < 0)
      return (i);
   if (flag & FL_EOL)
      return (1);
   goto loop;
}

static char *
breakout(base, flag)
register int *flag;
char **base;
{
   register char *str, *scr;
   register int oflag = *flag;

loop:
   *flag = 0;
   str = *base;
   while (*str == ' ' || *str == 9)
      ++str;
   switch (*str) {
   case '\0':
      *flag = FL_EOC | FL_EOL;
      *base = str;
      return (str);
   case '*':
   case '?':
      *flag = FL_WILD;
      break;
   case ';':
      *flag = FL_EOC;
      *str = '\0';
      *base = str + 1;
      return (str);
   case '\"':
      *flag = FL_QUOTE;
      break;
   case '$':
      *flag = FL_DOLLAR;
      break;
   case '%':
      *flag = FL_PERCENT;
      break;
   case '!':
      *flag = FL_BANG;
      break;
   default:
      break;
   }
   scr = str;
   for (;;) {
      switch (*scr) {
      case '$':
         if (scr != str)
            *flag |= FL_IDOLLAR;
         ++scr;
         break;
      case '*':
      case '?':
         *flag |= FL_WILD;
         ++scr;
         break;
      case ' ':
      case 9:
         if (!(oflag & FL_OVERIDE))
            *scr = '\0';
         *base = scr + 1;
         return (str);
      case '\"':                    /* Quote */
         del_char(scr);
         while (*scr && *scr != '\"') {
            if (*scr == '\\') {
               del_char(scr);
               if (*scr)
                  ++scr;
            } else {
               ++scr;
            }
         }
         if (*scr == '\"')
            del_char(scr);
         break;
      case '\0':
         *flag |= FL_EOL | FL_EOC;
         *base = scr;
         return (str);
      case ';':
         *flag |= FL_EOC;
         *base = scr + 1;
         *scr = '\0';
         return (str);
      case '^':
         ++scr;
         if (*scr) {
            *(scr - 1) = *scr & 0x1f;
            del_char (scr);
         }
         break;
      case '\\':
         del_char(scr);
         if (*scr)
            ++scr;
         break;
      default:
         ++scr;
      }
   }
}

del_char(str)
register char *str;
{
   for (; *str; ++str)
      str[0] = str[1];
}

static
find_command(str, arg)
char *str;
{
   int i;
   int len = strlen(str);

   if (*str >= '0'  &&  *str <= '9')
      return (1);
   for (i = 0; Command[i].func; ++i) {
      if (strncmp (str, Command[i].name, len) == 0) {
         if (arg == F_ABBR)
            return (i);
         if (strcmp (str, Command[i].name) == 0)
            return (i);
         return (-1);
      }
   }
   if (arg == F_ABBR)
      return (0);
   return (-1);
}



