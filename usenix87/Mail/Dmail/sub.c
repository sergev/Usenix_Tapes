
/*
 * SUB.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  Global Routines:    INDEXOF()
 *                      SIG()
 *                      POSITION_CURRENT()
 *                      SKIP_TO_DATE()
 *                      GET_FIELD()
 *                      COMPILE_FIELD()
 *                      ISFROM()
 *                      XSTRNCMP()
 *                      NEXT_WORD()
 *                      DONE()
 *
 */

#include <signal.h>
#include <stdio.h>
#include "dmail.h"

#define SENDMAIL "/usr/lib/sendmail"


indexof(num)
register int num;
{
    register int i, last;

    if (num < 1)
        num = -1;
    for (last = -1, i = 0; i < Entries; ++i) {
        if (Entry[i].no) {
            last = i;
            if (Entry[i].no == num)
                return (i);
        }
    }
    if (num == -1  &&  last >= 0)
        return (last);
    return (-1);
}


null()
{
}


position_current()
{
    int pos;

    pos = Entry[Current].fpos;
    if (fseek (m_fi, pos, 0) != pos)
        puts ("ERROR: Cannot position file to message");
}


skip_to_data(fi)
FILE *fi;
{
    char buf[MAXFIELDSIZE];

    while (fgets (buf, MAXFIELDSIZE, fi) != NULL) {
        if (*buf == '\n')
            return (1);
    }
    return (-1);
}


char *
get_field(str)
char *str;
{
    int i, entry = Current;
    int len = strlen(str);

    i = get_extra (str);
    if (i >= 0)
        return (Entry[entry].fields[i]);
    if (m_fi == NULL)
        return ("");
    fseek (m_fi, Entry[entry].fpos, 0);
    while (fgets (Buf, MAXFIELDSIZE, m_fi) != NULL) {
        if (isfrom (Buf))
            break;
        if (strncmp (Buf, str, len) == 0) {
            Buf[strlen(Buf) - 1] = '\0';
            compile_field(Buf, m_fi);
            return (next_word (Buf));
        }
    }
    return ("");
}


compile_field(buf, fi)
char *buf;
FILE *fi;
{
    int len, acc, pos;

    acc = 0;
    buf += strlen (buf) + 1;
    pos = ftell (fi);
    while (fgets (buf, MAXFIELDSIZE - acc, fi) != NULL) {
        if (*buf == ' ' || *buf == 9) {
            *(buf - 1) = '\n';
            len = strlen (buf) - 1;
            *(buf + len) = '\0';
            buf += len;
            acc += len + 2;
            if (acc > MAXFIELDSIZE - 10) {
                printf ("Warning: Field size beyond %d bytes\n", MAXFIELDSIZE);
                sleep (2);
                return (1);
            }
        } else {
            *buf = '\0';
            fseek (fi, pos, 0);
            return (1);
        }
        pos = ftell (fi);
    }
    fseek (fi, pos, 0);
}


isfrom(str)
register char *str;
{
    static char from[] = {"From "};
    register int i = 0;

    while (i < 5) {
        if (*str++ != from[i++])
            return (0);
    }
    return (1);
}


xstrncmp (src, dest, len)
register char *src, *dest;
register int len;
{
    while (--len >= 0) {
        if ((*src & 0x1f) != (*dest & 0x1f)) {
            if ((*src & 0x1f) < (*dest & 0x1f))
                return (-1);
            return (1);
        }
        ++src; ++dest;
    }
    return (0);
}



char *
next_word(str)
register char *str;
{
    while (*str  &&  *str != ' '  && *str != 9)
        ++str;
    while (*str  &&  (*str == ' ' || *str == 9))
        ++str;
    return (str);
}

done(n)
{
    char scr[64];

    push_break();
    sprintf (scr, "/tmp/dmail%d", getpid());
    unlink (scr);
    sprintf (scr, "/tmp/dmt%d", getpid());
    unlink (scr);
    unlink ("#");
    exit (n);
}


fix_globals()
{
    char *ptr;

    push_break();
    S_page = (ptr = get_var (LEVEL_SET, "page")) ? 
            ((*ptr) ? atoi (ptr) : 24) : -1;
    if (S_page > 0  && (S_page -= 4) < 0)
        S_page = 1;

    S_sendmail = (ptr = get_var (LEVEL_SET, "sendmail")) ? ptr : SENDMAIL;
    S_novibreak= (ptr = get_var (LEVEL_SET, "vibreak")) ? 0 : 1;
    S_verbose  = (ptr = get_var (LEVEL_SET, "verbose")) ? 1 : 0;
    S_ask      = (ptr = get_var (LEVEL_SET, "ask")) ? 1 : 0;
    pop_break();
}


_pager(str, nl)
char *str;
int nl;
{
    static int count;
    static FILE *fi;
    char buf[1024];
    char *ptr;

    if (str == 0) {
        switch (S_page) {
        case -1:
            count = 0;
            return (1);
        case 0:
            ptr = get_var (LEVEL_SET, "page");
            fi = popen (ptr, "w");
            if (fi == NULL) {
                count = 0;
                printf ("CANNOT RUN PAGER PROGRAM: %s\n", ptr);
            } else {
                count = -1;
            }
            return (1);
        default:
            count = 0;
            return (1);
        }
    }
    if ((long)str == -1) {
        if (fi != NULL) {
            pclose (fi);
            fi = NULL;
        }
        return (1);
    }
    if (count < 0) {
        fputs (str, fi);
        while (nl--)
            fputs ("\n", fi);
    } else {
        fputs (str, stdout);
        while (nl--) {
            fputs ("\n", stdout);
            ++count;
        }
        while (*str) {
            if (*str++ == '\n')
                ++count;
        }
        if (S_page > 0  &&  S_page <= count) {
            count = 0;
            puts ("\n-- more --");
            gets(buf);
        }
    }
}




