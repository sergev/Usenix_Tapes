
/*
 * COMMANDS.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *  DMAIL  (C) 1985  Matthew Dillon
 *
 *  Global Routines:    DO_QUIT()
 *                      DO_EXIT() 
 *                      DO_CD()
 *                      DO_ECHO()
 *                      DO_GO()
 *                      DO_SOURCE()
 *                      DO_SHELL()
 *                      DO_WRITE()
 *                      DO_DELNEXT()
 *                      DO_NUMBER()
 *                      DO_NEXT()
 *                      DO_HEADER()
 *                      DO_TYPE()
 *                      DO_DELETE()
 *                      DO_UNDELETE()
 *                      DO_MARK()
 *
 *  Static Routines:    None.
 *
 */

#include <stdio.h>
#include <sys/file.h>
#include "dmail.h"

#define LAST_TYPE   0
#define LAST_HEADER 1

static int Last_operation;
static int Last_deleted = -1;

do_quit(garbage, com)
char *garbage;
{
    int fd, r, back;
    char *str;
    char *nulav[3];

    nulav[0] = "";
    nulav[1] = "";
    nulav[2] = NULL;
    push_break();
    if (get_inode (mail_file) == get_inode (output_file)) {
        back = save_file (0, 0, ST_DELETED | ST_STORED);
    } else {
        r = write_file (output_file, O_CREAT, ST_READ, ST_DELETED | ST_STORED);
        if (r < 0) {
            printf ("Unable to write to %s\n", output_file);
            back = save_file (0, 0, ST_DELETED | ST_STORED);
        } else {
            back = save_file (0, 0, ST_READ | ST_DELETED | ST_STORED);
        }
    }
    if (back) 
        printf ("%d  kept in %s\n", back, mail_file);
    sleep (1);
    if ((fd = open (mail_file, O_RDONLY, 0)) >= 0) {
        read (fd, Buf, 1);
        close (fd);
    }
    if (!com) 
        done (0);
    free_entry();
    if (av[1] == 0) {
        if (!Silence)
            puts ("NO FROM FILE SPECIFIED");
        av[1] = mail_file;
        av[2] = NULL;
    }
    mail_file = realloc (mail_file, strlen(av[1]) + 1);
    strcpy (mail_file, av[1]);
    str = (av[2]) ? av[2] : mail_file;
    output_file = realloc (output_file, strlen(str) + 1);
    strcpy (output_file, str);
    initial_load_mail();
    m_select (nulav, M_RESET);
    pop_break();
    if (!Silence)
        printf ("\nRF %-20s   WF %-20s\n", mail_file, output_file);
}


do_exit(garbage, com)
char *garbage;
{
    char *str;
    char *nulav[3];

    nulav[0] = "";
    nulav[1] = "";
    nulav[2] = NULL;
    if (!com)
        done (0);
    push_break();
    free_entry();
    if (av[1] == 0) {
        if (!Silence)
            puts ("NO FROM FILE SPECIFIED");
        av[1] = mail_file;
        av[2] = NULL;
    }
    mail_file = realloc (mail_file, strlen(av[1]) + 1);
    strcpy (mail_file, av[1]);
    str = (av[2]) ? av[2] : mail_file;
    output_file = realloc (output_file, strlen(str) + 1);
    strcpy (output_file, str);
    initial_load_mail();
    m_select (nulav, M_RESET);
    pop_break();
    if (!Silence)
        printf ("\nRF %-20s   WF %-20s\n", mail_file, output_file);
}


do_cd()
{
    char *dir = (ac < 2) ? home_dir : av[1];

    if (chdir(dir) < 0) {
        printf ("Cannot CD to %s\n", dir);
        return (-1);
    }
    return (1);
}


do_echo(str)
char *str;
{
    puts (next_word(str));
    return (1);
}


do_go()
{
    int i;

    if (ac < 2) {
        puts ("go to which article?");
    }
    rewind_range (1);
    i = get_range();
    if (i < 0) {
        if (!Silence)
            printf ("Message #%d does not exist\n", i);
        return (-1);
    }
    if (i == 0) {
        if (!Silence)
            puts ("No Message");
        return (-1);
    }
    Current = indexof(i);
    return (1);
}


do_source()
{
    char comline[1024];
    FILE *fi = NULL;

    if (ac < 2) {
        puts ("No file argument to source");
        return (-1);
    }
    if (push_base()) {
        push_break();
        pop_base();
        if (fi != NULL)
            fclose (fi);
        pop_break();
        return (-1);
    }
    push_break();
    fi = fopen (av[1], "r");
    pop_break();
    if (fi == NULL) {
        printf ("Cannot open %s\n", av[1]);
        return (-1);
    }
    while (fgets (comline, 1024, fi) != NULL) {
        comline[strlen(comline) - 1] = '\0';
        exec_command (comline);
    }
    push_break();
    fclose (fi);
    fi = NULL;
    pop_break();
    pop_base();
    return (1);
}


do_shell(str)
char *str;
{
    int pid, ret;
    char *shell;
    char *args;

    shell = getenv("SHELL");
    if (shell == NULL)
        shell = "/bin/sh";
    args = "-fc";
    push_break();
    str = next_word (str);
    if (strlen (str)) {
        if ((pid = vfork()) == 0) {
            execl (shell, shell, args, str, NULL);
            _exit (1);
        }
    } else {
        if ((pid = vfork()) == 0) {
            execl (shell, shell, NULL);
            _exit (1);
        }
    }
    while ((ret = wait(0)) > 0) {
        if (ret == pid)
            break;
    }
    pop_break();
    return (1);
}


do_write()
{
    char *file;
    int r, count = 0;
    register int i, j;

    if (ac < 2) {
        puts ("You must specify at least a file-name");
        return (-1);
    }
    file = av[1];
    rewind_range (2);
    push_break();
    while (i = get_range()) {
        j = indexof (i);
        if (j >= 0  &&  !(Entry[j].status & ST_DELETED)) {
            Entry[j].status |= ST_STORED | ST_SCR;
            ++count;
        }
    }
    r = write_file (file, O_CREAT, ST_SCR, 0);
    rewind_range (2);
    if (r > 0) {
        while (i = get_range()) {
            j = indexof (i);
            if (j >= 0)
                Entry[j].status &= ~ST_SCR;
        }
        if (!Silence)
            printf ("%d Items written\n", count);
    } else {
        while (i = get_range()) {
            j = indexof (i);
            if (j >= 0)
                Entry[j].status &= ~(ST_SCR | ST_STORED);
        }
        printf ("Could not write to file %s\n", file);
    }
    pop_break();
    return (1);
}


do_delnext()
{
    static int warning;

    if (!warning  &&  Last_operation == LAST_HEADER) {
        ++warning;
        puts ("Note that the next command is displaying headers only at");
        puts ("this point.  (one-time warning, NOTHING deleted");
        return (-1);
    }
    if (do_mark("", ST_DELETED) > 0)
        return (do_next("", 1));
    return (-1);
}


do_number(str, com)
char *str;
int com;
{
    int x;

    x = indexof (atoi(str));
    if (x < 0) {
        puts ("Non existant message");
        return (-1);
    }
    Current = x;
    switch (Last_operation) {
    case LAST_TYPE:
        return (do_type());
    case LAST_HEADER:
        return (do_header());
    default:
        puts ("Internal Error NEXT");
        return (-1);
    }
}


do_next(str, com)
char *str;
{
    int ok;

    push_break();
    if (com > 0) {
        if (++Current > Entries)
            Current = Entries;
        if (fix() < 0) {
            puts ("End of file");
            pop_break();
            return (-1);
        }
        --com;
    }
    if (com < 0) {
        ++com;
        ok = 0;
        while (--Current >= 0) {
            if (Entry[Current].no  &&  !(Entry[Current].status & ST_DELETED)) {
                ok = 1;
                break;
            }
        }
        if (!ok) {
            puts ("Start of file");
            Current = 0;
            fix();
            pop_break();
            return (-1);
        }
    }
    pop_break();
    if (!com) {
        switch (Last_operation) {
        case LAST_TYPE:
            return (do_type());
        case LAST_HEADER:
            return (do_header());
        }
    }
    return (1);
}


do_header()
{
    char buf[1024];

    Last_operation = LAST_HEADER;
    if (push_base()) {
        push_break();
        pop_base();
        PAGER (-1);
        fseek (m_fi, 0, 0);
        fflush (stdout);
        pop_break();
        return (-1);
    }
    if (single_position() < 0)
        return (-1);
    PAGER (0);
    sprintf (Puf, "MESSAGE HEADER #%d (%d) %s\n",
            Entry[Current].no, 
            Current + 1,
            (Entry[Current].status & ST_DELETED) ? "  DELETED" : "");
    PAGER (Puf);
    sprintf (Puf, "From %s\n", Entry[Current].from);
    PAGER (Puf);
    while (fgets (buf, 1024, m_fi) != NULL) {
        FPAGER (buf);
        if (*buf == '\n') {
            PAGER (-1);
            pop_base();
            return (1);
        }
    }
    PAGER ("END OF FILE ENCOUNTERED");
    PAGER (-1);
    pop_base();
    return (-1);
}


do_type()
{
    char buf[1024];
    int i;

    Last_operation = LAST_TYPE;
    if (push_base()) {
        push_break();
        pop_base();
        PAGER (-1);
        fseek (m_fi, 0, 0);
        fflush (stdout);
        pop_break();
        return (-1);
    }
    if (single_position() < 0)
        return (-1);
    if (skip_to_data (m_fi) < 0) {
        printf ("Cannot find data for message %d\n", Entry[Current].no);
        return (-1);
    }
    PAGER (0);
    sprintf (Puf, "MESSAGE TEXT #%d (%d) %s\n", 
            Entry[Current].no, 
            Current + 1,
            (Entry[Current].status & ST_DELETED) ? "  DELETED" : "");
    PAGER (Puf);
    for (i = 0; i < Listsize; ++i) {
        if (*Entry[Current].fields[header[i]]) {
            sprintf (Puf, "%-10s %s", 
                    Find[header[i]].search,
                    Entry[Current].fields[header[i]]);
            PAGER (Puf);
        }
    }
    PAGER ("");
    while ((fgets (buf, 1024, m_fi) != NULL)  &&  strncmp (buf, "From ", 5)) 
        FPAGER (buf);
    Entry[Current].status |= ST_READ;
    PAGER (-1);
    pop_base();
    return (1);
}


do_mark(garbage, mask)
char *garbage;
{
    int count = 0;
    register int i, j;

    rewind_range (1);
    push_break();
    while (i = get_range()) {
        j = indexof (i);
        if (j >= 0) {
            if (mask & ST_DELETED)
                Last_deleted = j;
            if ((Entry[j].status & mask) != mask) {
                Entry[j].status |= mask;
                if (Entry[j].status & ST_DELETED)
                    Entry[j].status &= ~(ST_STORED | ST_READ | ST_TAG);
                ++count;
            }
        }
    }
    if (!Silence)
        printf ("%d  Items\n", count);
    pop_break();
    return (1);
}


do_unmark(garbage, mask)
char *garbage;
{
    int count = 0;
    register int i, j;
    register struct ENTRY *en;

    push_break();
    if (ac == 1 && (mask & ST_DELETED) && Last_deleted != -1)  {
        en = &Entry[Last_deleted];
        if (en->no) {
            en->status &= ~mask;
            printf ("Undeleted last deleted message (# %d)\n", en->no);
            Current = Last_deleted;
            Last_deleted = -1;
        } else {
            puts ("Last deleted message not within current select bounds");
            pop_break();
            return (-1);
        }
        pop_break();
        return (1);
    }
    rewind_range (1);
    while (i = get_range()) {
        j = indexof (i);
        if (j >= 0) {
            if (Entry[j].status & mask) {
                Entry[j].status &= ~mask;
                ++count;
            }
        }
    }
    if (!Silence)
        printf ("%d  Items\n", count);
    pop_break();
    return ((count) ? 1 : -1);
}


