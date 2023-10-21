
/*
 *  SENDMAIL.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  Global Routines:    DO_REPLY()
 *                      DO_MAIL()
 *
 *  Static Routines:    WORD_SIZE()
 *                      FOPEN_SCRATCH()
 *                      FREOPEN_SCRATCH()
 *                      FCLOSE_SCRATCH()
 *                      FTERMINATE_SCRATCH()
 *                      DELETE_SCRATCH()
 *                      RUN_VI()
 *                      SEND_MAIL() 
 *
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sysexits.h>
#include <signal.h>
#include "dmail.h"

extern FILE *popen();

FILE *fi;
char file[64];

do_reply(garbage, itext)
char *garbage;
{
    int i, j, notfirst;
    int anyargs  = 0;
    int to_field, cc_field;
    register int len;
    char *ptr;
    char buf[1024];
    char scr;
    FILE *fs;

    if (push_base()) {
        push_break();
        pop_base();
        fclose_scratch();
        puts ("ABORTED, no mail sent");
        pop_break();
        return (-1);
    }
    fopen_scratch();
    strcpy (buf, "To: ");
    for (i = 1; i < ac; ++i) {
        if (*av[i] >= '0'  &&  *av[i] <= '9') {
            if ((j = indexof(atoi(av[i]))) < 0) {
                puts ("No such message");
                fclose_scratch();
                pop_break();
                return (-1);
            }
            Current = j;
        } else {
            if (anyargs)
                strcat (buf, ", ");
            anyargs = 1;
            strcat (buf, av[i]);
        }
    }
    len = strlen(buf);
    switch (itext) {
    case R_FORWARD:
        strcat (buf, "\n");
        fputs (buf, fi);
        fputs ("Subject: \n", fi);
        break;
    case R_INCLUDE:
    case R_REPLY:
        if (anyargs) {
            strcat (buf, ", ");
            len = strlen(buf);
        }
        sprintf (buf + len, "%.*s\n",
                word_size(Entry[Current].from), Entry[Current].from);
        fputs (buf, fi);
        fputs ("Cc: ", fi);
        ptr = get_field ("To:");
        to_field = (*ptr) ? 1 : 0;
        fputs (ptr, fi);
        scr = *(ptr + strlen(ptr) - 1);
        ptr = get_field ("Cc:");
        cc_field = (*ptr) ? 1 : 0;
        if (cc_field) {
            if (scr == '\n') {
                fputs ("         ", fi);
            }
            if (to_field)
                fputs (", ", fi);
            fputs (ptr, fi);
        }
        fputs ("\nSubject: Re: ", fi);
        fputs (get_field ("Subject:"), fi);
        fputs ("\n", fi);
        break;
    case R_MAIL:
        fputs (buf, fi);
        fputs ("\n", fi);
        fputs ("Cc: \n", fi);
        fputs ("Bcc: \n", fi);
        fputs ("Subject: \n", fi);
        break;
    default:
        puts ("INTERNAL STUPID MAIL ERROR: REPLY");
        break;
    }
    copy_header (fi);
    fputs ("\n\n", fi);
    if (itext == R_FORWARD  ||  itext == R_INCLUDE) {
        position_current();
        if (itext == R_FORWARD)
            fprintf (fi, "ORIGINALLY From %s\n", Entry[Current].from);
        else
            skip_to_data (m_fi);
        while ((fgets (Buf, MAXFIELDSIZE, m_fi) != NULL) && !isfrom(Buf)) {
            if (itext == R_INCLUDE) 
                fputs (" >      ", fi);
            fputs (Buf, fi);
        }
        fputs ("\n", fi);
    }
    fclose_scratch();
    if (itext != R_MAIL) {
        push_break();
        Entry[Current].status |= ST_SCR;
        write_file ("#", O_CREAT | O_TRUNC, ST_SCR, 0);
        Entry[Current].status &= ~ST_SCR;
        pop_break();
    }
    j = -1;
loop:
    ++j;
    if (run_vi() || j) {
        push_break();
        switch (do_ask()) {
        case 1:
            puts ("SENDING.. wait");
            send_mail();
            break;
        case 2:
            pop_break();
            goto loop;
        default:
            break;
        }
        pop_base();
        pop_break();
    } else {
        puts ("File not modified or ABORTED, no mail sent");
        pop_base();
    }
    unlink ("#");
}

do_ask()
{
    char in[256];

    if (!S_ask)
        return (1);
    fputs ("\n(Send, Vi, Quit) ?", stdout);
    fflush(stdout);
    gets (in);
    switch (in[0]) {
    case 's':
    case 'S':
        return (1);
    case 'v':
    case 'V':
        return (2);
    }
    puts ("ABORT, no mail sent");
    return (0);
}



static
copy_header(fi)
FILE *fi;
{
    FILE *fs;
    char *ptr;

    if (ptr = get_var (LEVEL_SET, "header")) {
        push_break();
        if ((fs = fopen (ptr, "r")) != NULL) {
            while (fgets (Buf, MAXFIELDSIZE, fs) != NULL)
                fputs (Buf, fi);
            fclose (fs);
        } else {
            printf ("Cannot open header file %d %s\n", strlen(ptr), ptr);
            perror ("fopen");
        }
        pop_break();
    }
}


static
fopen_scratch()
{
    sprintf (file, "/tmp/dmt%d", getpid());
    fi = fopen (file, "w+");
    if (fi == NULL) {
        perror ("Dmail, cannot open scratch file");
        done (1);
    }
}



static
fclose_scratch()
{
    if (fi != NULL) {
        fflush (fi);
        fclose (fi);
        fi = NULL;
    }
}


static
fterminate_scratch()
{
    if (fi != NULL) {
        fclose (fi);
        fi = NULL;
        unlink (file);
    }
}


static
delete_scratch()
{
    sprintf (file, "/tmp/dmt%d", getpid());
    unlink (file);
}


static
word_size(str)
register char *str;
{
    register int size = 0;

    while (*str) {
        if (*str == ' ')
            return (size);
        ++str;
        ++size;
    }
    return (size);
}


static
run_vi()
{
    char buf[64];
    int ret, pid = 0;
    struct stat stat1, stat2;
    char *argv[3];

    argv[0] = visual;
    argv[1] = file;
    argv[2] = NULL;
    if (push_base()) {
        push_break();
        pop_base();
        if (pid) {
            kill (pid, SIGKILL);
            sprintf (buf, "/tmp/Ex%d", pid); unlink (buf);
            sprintf (buf, "/tmp/Rx%d", pid); unlink (buf);
            wait(0);
            system ("clear; reset ; clear");
            pid = 0;
        }
        pop_break();
        return (0);
    }
    stat1.st_mtime = stat2.st_mtime = stat1.st_ctime = stat2.st_ctime = 0;
    stat (file, &stat1);
    if (S_novibreak)
        push_break();

    pid = vfork();
    if (!pid) {
        execv (visual, argv);
        _exit (1);
    }
    while ((ret = wait(0)) > 0) {
        if (ret == pid)
            break;
    }
    if (S_novibreak)
        pop_break();
    stat (file, &stat2);
    pop_base();
    return (!(stat1.st_mtime==stat2.st_mtime));
}


static
send_mail()
{
    char buf[1024];
    int fildes[2];
    int fd, stdin_fd;
    int i;
    char *argv[6];

    push_break();
    argv[0] = S_sendmail;
    argv[1] = "-t";
    argv[2] = "-oo";
    argv[3] = "-oi";
    if (S_verbose) {
        argv[4] = "-v";
        argv[5] = NULL;
    } else {
        argv[4] = NULL;
    }
    pipe (fildes);
    stdin_fd = dup (0);
    dup2 (fildes[0], 0);
    if (!vfork()) 
        execv (S_sendmail, argv);
    dup2 (stdin_fd, 0);
    fd = open (file, O_RDONLY, 0);
    if (fd < 0) {
        perror ("Dmail, Cannot open scratch file");
        done (1);
    }
    while ((i = read (fd, buf, 1024)) > 0)
        write (fildes[1], buf, i);
    close (fd);
    close (fildes[1]);
    if (S_verbose)
        wait (0);
    pop_break();
}


