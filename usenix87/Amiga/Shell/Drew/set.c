
/*
 * SET.C
 *
 * (c)1986 Matthew Dillon     9 October 1986
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 */

#include "shell.h"
#define MAXLEVELS (3 + MAXSRC)

struct MASTER {
   struct MASTER *next;
   struct MASTER *last;
   char *name;
   char *text;
};

static struct MASTER *Mbase[MAXLEVELS];

char *
set_var(level, name, str)
register char *name, *str;
{
   register struct MASTER *base = Mbase[level];
   register struct MASTER *last;
   register int len;

   for (len = 0; isalphanum(name[len]); ++len);
   while (base != NULL) {
      if (strlen(base->name) == len && strncmp (name, base->name, len) == 0) {
	 Free (base->text);
	 goto gotit;
      }
      last = base;
      base = base->next;
   }
   if (base == Mbase[level]) {
      base = Mbase[level] = (struct MASTER *)malloc (sizeof(struct MASTER));
      base->last = NULL;
   } else {
      base = (struct MASTER *)malloc (sizeof(struct MASTER));
      base->last = last;
      last->next = base;
   }
   base->name = malloc (len + 1);
   bmov (name, base->name, len);
   base->name[len] = 0;
   base->next = NULL;
gotit:
   base->text = malloc (strlen(str) + 1);
   strcpy (base->text, str);
   return (base->text);
}

char *
get_var (level, name)
register char *name;
{
   register struct MASTER *base = Mbase[level];
   register unsigned char *scr;
   register int len;

   for (scr = (unsigned char *)name; *scr && *scr != 0x80 && *scr != ' ' && *scr != ';' && *scr != '|'; ++scr);
   len = scr - name;

   while (base != NULL) {
      if (strlen(base->name) == len && strncmp (name, base->name, len) == 0)
	 return (base->text);
      base = base->next;
   }
   return (NULL);
}

unset_level(level)
{
   register struct MASTER *base = Mbase[level];

   while (base) {
      Free (base->name);
      Free (base->text);
      Free (base);
      base = base->next;
   }
   Mbase[level] = NULL;
}

unset_var(level, name)
char *name;
{
   register struct MASTER *base = Mbase[level];
   register struct MASTER *last = NULL;
   register int len;

   for (len = 0; isalphanum(name[len]); ++len);
   while (base) {
      if (strlen(base->name) == len && strncmp (name, base->name, len) == 0) {
	 if (base != Mbase[level])
	    last->next = base->next;
	 else
	    Mbase[level] = base->next;
	 if (base->next != NULL)
	    base->next->last = last;
	 if (base == Mbase[level])
	    Mbase[level] = base->next;
	 Free (base->name);
	 Free (base->text);
	 Free (base);
	 return (1);
      }
      last = base;
      base = base->next;
   }
   return (-1);
}


do_unset_var(str, level)
char *str;
{
   register int i;

   for (i = 1; i < ac; ++i)
      unset_var (level, av[i]);
   return (0);
}

do_set_var(command, level)
char *command;
{
   register struct MASTER *base = Mbase[level];
   register char *str;

   if (ac == 1) {
      while (base) {
	 if (CHECKBREAK())
	     return(0);
	 printf ("%-10s ", base->name);
	 puts (base->text);
	 base = base->next;
      }
      return (0);
   }
   if (ac == 2) {
      str = get_var (level, av[1]);
      if (str) {
	 printf ("%-10s ", av[1]);
	 puts(str);
      } else if (level == LEVEL_SET) { /* only create var if set command */
	 set_var (level, av[1], "");
      }
   }
   if (ac > 2)
      set_var (level, av[1], next_word (next_word (command)));
   if (*av[1] == '_') {
      S_histlen = (str = get_var(LEVEL_SET, V_HIST))   ? atoi(str) : 0;
      debug	= (str = get_var(LEVEL_SET, V_DEBUG))  ? atoi(str) : 0;
      Verbose	= (get_var(LEVEL_SET, V_VERBOSE)) ? 1 : 0;
      if (S_histlen < 2)   S_histlen = 2;
   }
   return (0);
}
