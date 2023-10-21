
/*
 * HELP.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  (C) 1985  Matthew Dillon
 *
 *  Global Routines:    DO_HELP()
 *
 */

#include <stdio.h>
#include "dmail.h"
#include "execom.h"

#ifndef HELPFILE
static char *help[] = {
#include ".dmkout"
};

do_help()
{
    int i, j;
    char *ptr;

    if (push_base()) {
        push_break();
        pop_base();
        PAGER (-1);
        pop_break();
        return;
    }
    PAGER (0);
    if (ac == 1) {
        for (j = 0; help[j] && *help[j] != '.'; ++j)
            PAGER (help[j]);
        for (i = 0; Command[i].name != NULL; ++i) {
            if (*Command[i].name) {
                sprintf (Puf, "%-10s %s", Command[i].name, Desc[i]);
                PAGER (Puf);
            }
        }
    }
    PAGER ("");
    for (i = 1; i < ac; ++i) {
        j = 0;
again:
        while (help[j]  &&  *help[j] != '.')
            ++j;
        if (help[j]) {
            if (strncmp (av[i], help[j] + 1, strlen(av[i]))) {
                ++j;
                goto again;
            }
            while (help[j]  &&  *help[j] == '.')
                ++j;
            while (help[j]  &&  *help[j] != '.')
                PAGER (help[j++]);
            PAGER ("");
            goto again;
        }
    }
    PAGER (-1);
    pop_base();
}

#else

do_help()
{
    int i, j;
    FILE *fi = NULL;
    char *ptr;
    char *eof;

    if (push_base()) {
        push_break();
        pop_base();
        PAGER (-1);
        if (fi != NULL) {
            fclose (fi);
            fi = NULL;
        }
        pop_break();
        return (-1);
    }
    fi = fopen (HELPFILE, "r");
    if (fi == NULL) {
        printf ("Cannot open help file: %s\n", HELPFILE);
        PAGER (-1);
        pop_base();
        return (-1);
    }
    PAGER (0);
    if (ac == 1) {
        while (fgets (Puf, MAXFIELDSIZE, fi)  &&  *Puf != '.')
            FPAGER (Puf);
        fclose (fi);
        fi = NULL;
        for (i = 0; Command[i].name != NULL; ++i) {
            if (*Command[i].name) {
                sprintf (Puf, "%-10s %s", Command[i].name, Desc[i]);
                PAGER (Puf);
            }
        }
        PAGER (-1);
        pop_base();
        return (1);
    }
    PAGER ("");
    for (i = 1; i < ac; ++i) {
        fseek (fi, 0, 0);
again:
        while ((eof = fgets (Puf, MAXFIELDSIZE, fi))  &&  *Puf != '.');
        if (!eof)
            continue;
        if (strncmp (av[i], Puf + 1, strlen(av[i]))) 
            goto again;
        while ((eof = fgets (Puf, MAXFIELDSIZE, fi))  &&  *Puf == '.');
        if (!eof)
            continue;
        FPAGER (Puf);
        while ((eof = fgets (Puf, MAXFIELDSIZE, fi))  &&  *Puf != '.')
            FPAGER (Puf);
        PAGER ("");
        if (!eof)
            continue;
        goto again;
    }
    fclose (fi);
    fi = NULL;
    PAGER (-1);
    pop_base();
}

#endif

