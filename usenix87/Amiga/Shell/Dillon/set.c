
/*
 * SET.C
 *
 * Matthew Dillon, 24 Feb 1986
 *
 */

#include "shell.h"
#define MAXLEVELS 3

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

   while (base != NULL) {
      if (strcmp (name, base->name) == 0) {
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
   base->name = malloc (strlen(name) + 1);
   strcpy (base->name, name);
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

   while (base != NULL) {
      if (strcmp (name, base->name) == 0)
         return (base->text);
      base = base->next;
   }
   return (NULL);
}


unset_var (level, name)
char *name;
{
   register struct MASTER *base = Mbase[level];
   register struct MASTER *last = NULL;

   while (base != NULL) {
      if (strcmp (name, base->name) == 0) {
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
   int i;

   for (i = 1; i < ac; ++i)
      unset_var (level, av[i]);
   return (1);
}

do_set_var(command, level)
char *command;
{
   register struct MASTER *base = Mbase[level];
   register char *str;

   if (ac == 1) {
      while (base) {
         printf ("%-10s %s\n", base->name, base->text);
         base = base->next;
      }
      return (1);
   }
   if (ac == 2) {
      str = get_var (level, av[1]);
      if (str)
         printf ("%-10s %s\n", av[1], str);
      else
         set_var (level, av[1], "");
   }
   if (ac > 2)
      set_var (level, av[1], next_word (next_word (command)));
   if (*av[1] == '_') {
      S_histlen = (str = get_var(LEVEL_SET, V_HIST)) ? atoi(str) : 0;

      if (S_histlen < 2)   S_histlen = 2;
   }
   return (1);
}


