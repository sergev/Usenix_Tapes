
/*
 *  SET.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  Variable set/unset/get/reset routines
 *
 */


#include <stdio.h>
#include "dmail.h"
#define MAXLEVELS 3

struct MASTER {
    struct MASTER *next;
    struct MASTER *last;
    char *name;
    char *text;
};

struct MASTER *Mbase[MAXLEVELS];

set_var (level, name, str)
register char *name, *str;
{
    register struct MASTER *base = Mbase[level];
    register struct MASTER *last;

    push_break();
    while (base != NULL) {
        if (strcmp (name, base->name) == 0) {
            free (base->text);
            goto gotit;
        }
        last = base;
        base = base->next;
    }
    if (base == Mbase[level]) {
        base = Mbase[level] = (struct MASTER *)malloc (sizeof (struct MASTER));
        base->last = NULL;
    } else {
        base = (struct MASTER *)malloc (sizeof (struct MASTER));
        base->last = last;
        last->next = base;
    }
    base->name = malloc (strlen (name) + 1);
    strcpy (base->name, name);
    base->next = NULL;
gotit:
    base->text  = malloc (strlen (str) + 1);
    strcpy (base->text, str);
    pop_break();
}


unset_var(level, name, str)
register char *name, *str;
{
    register struct MASTER *base = Mbase[level];
    register struct MASTER *last;

    push_break();
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
            free (base->name);
            free (base->text);
            free (base);
            pop_break();
            return (1);
        }
        last = base;
        base = base->next;
    }
    pop_break();
    return (-1);
}


char *
get_var(level, name)
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


do_unset_var(str, level)
char *str;
{
    int i;

    push_break();
    for (i = 1; i < ac; ++i)
        unset_var (level, av[i]);
    fix_globals();
    pop_break();
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
    }
    if (ac == 2) {
        str = get_var (level, av[1]);
        if (str) {
            printf ("%-10s %s\n", av[1], str);
        } else {
            push_break();
            set_var (level, av[1], "");
            fix_globals();
            pop_break();
        }
    }
    if (ac > 2) {
        push_break();
        set_var (level, av[1], next_word (next_word (command)));
        fix_globals();
        pop_break();
    }
}



