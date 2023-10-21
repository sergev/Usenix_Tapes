
/*
 * RANGE.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  Global Routines:    REWIND_RANGE()
 *                      GET_RANGE()
 *                      SINGLE_POSITION()   
 *
 *  Static Routines:    None.
 *
 *
 */

#include <stdio.h>
#include "dmail.h"


static int range_ac;
static int in, start, end;

struct RANOP {
    char *name;
    int status, kstatus;
};

static struct RANOP Ranop[] = {
    "all",  0,              0,
    "tag",  ST_TAG,         ST_DELETED,
    "wri",  ST_STORED,      ST_DELETED,
    "del",  ST_DELETED,     0,
    "mar",  ST_READ,        ST_DELETED,
    "unt",  0,              ST_DELETED | ST_TAG,
    "unw",  0,              ST_DELETED | ST_STORED,
    "und",  0,              ST_DELETED,
    "unm",  0,              ST_DELETED | ST_READ,
    NULL ,  0,              0 };

rewind_range(beg)
{
    Silence = 0;
    range_ac = beg;
    if (range_ac >= ac) {
        start = Entry[Current].no;
        end   = start;
        in    = 1;
    } else {
        in    = 0;
    }
}


get_range()
{
    register char *ptr;
    register int i;
    static int status;      /* Status items required                */
    static int kstatus;     /* Status items which cannot be present */

again:
    if (in  &&  start <= end) {
        i = indexof(start++);
        if (i < 0  || (Entry[i].status & status) != status ||
                (Entry[i].status & kstatus))
            goto again;
        return (start - 1);
    }
    in = status = kstatus = 0;
    if (range_ac >= ac)
        return (0);
    ptr = av[range_ac++];
    if (*ptr == '-') {
        if (xstrncmp (ptr, "-s", 2) == 0) {
            Silence = 1;
            goto again;
        }
        start = 1;
        ++ptr;
        goto dash;
    }
    if (*ptr < '0'  ||  *ptr > '9') {
        start = 1;
        end = 0;
        for (i = 0; Ranop[i].name; ++i) {
            if (xstrncmp (ptr, Ranop[i].name, 3) == 0) {
                status = Ranop[i].status;
                kstatus = Ranop[i].kstatus;
                goto imprange;
            }
        }
        goto again;
    }
    start = atoi(ptr);
    while (*(++ptr)) {
        if (*ptr == '-') {
            ++ptr;
            goto dash;
        }
    }
    if (range_ac >= ac)
        return (start);
    if (*av[range_ac] == '-') {
        ptr = av[range_ac++] + 1;
        goto dash;
    }
    return (start);
dash:
    if (*ptr) {
        end = atoi(ptr);
        goto imprange;
    }
    if (range_ac >= ac) {
        end = 0;
        goto imprange;
    }
    end = atoi(av[range_ac++]);
imprange:
    if (end == 0) {
        end = indexof (0);
        if (end < 0)
            return (0);
        end = Entry[end].no;
    }
    if (start > end) {
        printf ("Bad Range: %s\n", av[range_ac - 1]);
        return (0);
    }
    in = 1;
    goto again;
}


single_position()
{
    long pos;
    int old = Current;

    switch (ac) {
    case 1:
        break;
    case 2:
        Current = indexof (atoi(av[1]));
        if (Current < 0) {
            Current = old;
            puts ("Out of Range, 0 will take you to the last entry");
            return (-1);
        }
        break;
    default:
        puts ("Range not implemented (yet?)");
        return (-1);
    }
    while (Current < Entries  &&  Entry[Current].no == 0)
        ++Current;
    if (Current >= Entries) {
        Current = old;
        puts ("No More Messages");
        return (-1);
    }
    position_current();
    return (1);
}


