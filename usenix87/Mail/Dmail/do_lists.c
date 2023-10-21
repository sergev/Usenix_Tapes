
/*
 *  DO_LISTS.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  (C) 1985  Matthew Dillon
 *
 *  Global Routines:    DO_SETLIST()
 *                      DO_LIST()
 *                      DO_SELECT()
 *                      DO_DEFER()
 *
 *  Static Routines:    None.
 *
 *      LIST associated commands.
 *
 */

#include <stdio.h>
#include "dmail.h"


do_setlist(str)
char *str;
{
    int i, fw, idx, localecho = 1;
    int sac = 1;

    push_break();
    if (strcmp (av[sac], "-s") == 0) {
        ++sac;
        localecho = 0;
    }
    hold_load();
    if (ac > sac) {
        Listsize = 0;
        for (i = sac; i < ac; ++i) {
            fw = atoi(av[i]);
            if (fw > 4096)
                fw = 4096;
            if (fw < 0  ||  (*av[i] < '0'  ||  *av[i] > '9'))
                fw = 20;
            else
                ++i;
            if (i >= ac)
                continue;
            idx = get_extra_ovr (av[i]);
            if (idx < 0) {
                printf ("To many entries, cannot load: %s\n", av[i]);
                fflush (stdout);
                continue;
            }
            header[Listsize] = idx;
            width[Listsize] = fw;
            ++Listsize;
        }
    }
    nohold_load();
    pop_break();
    if (localecho) {
        puts ("");
        printf ("Entry   Width   Field\n\n");
        for (i = 0; i < Listsize; ++i) 
            printf ("%-6d   %-5d   %s\n", i, width[i], Find[header[i]].search);
        puts ("");
    }
    return (1);
}


/*
 * Pre-position #   0 >     Current article
 *                  1 -     Read already
 *
 */


do_list()
{
    int i, j;
    int start = 0;
    int end   = Entries;
    static char curr[10] = { "    " };

    if (ac == 1) {
        av[1] = "all";
        ++ac;
    }
    if (push_base()) {
        push_break();
        pop_base();
        PAGER (-1);
        pop_break();
        return (-1);
    }
    PAGER (0);
    FPAGER ("\n         ");
    for (j = 0; j < Listsize; ++j) {
        if (width[j]) {
            sprintf (Puf, "%-*.*s",
                    2 + width[j],
                    2 + width[j],
                    Find[header[j]].search);
            FPAGER (Puf);
        }
    }
    FPAGER ("\n");
    rewind_range (1);
    while (i = get_range()) {
        i = indexof(i);
        if (Entry[i].no  &&  ((Entry[i].status & ST_DELETED) == 0)) {
            curr[0] = (Entry[i].status & ST_TAG) ? 'T' : ' ';
            curr[1] = (i == Current) ? '>' : ' ';
            curr[2] = (Entry[i].status & ST_READ)    ? 'r' : ' ';
            curr[3] = (Entry[i].status & ST_STORED)  ? 'w' : ' ';
            sprintf (Puf, "%s%-3d", curr, Entry[i].no);
            FPAGER (Puf);
            for (j = 0; j < Listsize; ++j) {
                if (width[j]) {
                    sprintf(Puf, "  %-*.*s",
                            width[j], 
                            width[j],
                            Entry[i].fields[header[j]]);
                    FPAGER (Puf);
                }
            }
            FPAGER ("\n");
        }
    }
    FPAGER ("\n");
    PAGER (-1);
    pop_base();
    return (1);
}



do_select(str, mode)
char *str;
{
    int ret = 1;
    int localecho = 1;
    int avi = 1;
    int scr;
    char *nulav[3];

    nulav[0] = "";
    nulav[1] = "";
    nulav[2] = NULL;
    if (strcmp (av[avi], "-s") == 0) {
        localecho = 0;
        ++avi;
        --ac;
    }
    switch (ac) {
    case 2:
        if (localecho)
            puts ("SELECT ALL");
        ret = m_select (nulav, mode);
        break;
    case 1:
        break;
    default:
        ret = m_select (av + avi, mode);
        scr = indexof(0);
        if (scr > 0  &&  localecho)
            printf ("%d  Entries selected\n", Entry[scr].no);
        break;
    }
    if (ret < 0  &&  localecho) {
        puts ("Null field");
        return (-1);
    }
    return (1);
}


do_defer()
{
    register int i, j;

    push_break();
    j = 1;
    for (i = 0; i < Entries; ++i) {
        if (Entry[i].no)
            Entry[i].no = (Entry[i].status & ST_READ) ? 0 : j++;
    }
    pop_break();
    return (1);
}

