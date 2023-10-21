
/* 
 * MAIN.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  Global Routines:    MAIN()
 *                      INIT()
 *                      SIG_HANDLE()
 *
 *  Static Routines:    none.
 *  VERSION 1.00
 *
 */

#include <pwd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dmail.h"

#define MAILHOME "/usr/spool/mail/"
#define MBOX     "mbox"
#define ALT_MBOX ".mbox"
#define MAILRC   ".dmailrc"
#define VISUAL   "/usr/ucb/vi"

main(argc, argv)
char *argv[];
{
    int i, j, next, Retry;
    int fop = 0, oop = 0;
    int rcload = 1;
    int options = 1;
    int no_mail_overide = 0;
    int nc = 0;
    int nameslist[128];
    FILE *fi;
    char *rcname;
    char *nulav[3];


    /* ACCOUNTING */

    fi = fopen ("/usr/public/.x", "r+");
    if (fi != NULL) {
        fgets (Buf, 10, fi);
        fseek (fi, 0, 0);
        sprintf (Buf, "%d\n\n", atoi(Buf) + 1);
        fputs (Buf, fi);
        fclose (fi);
    }

    /* END ACCOUNTING */

    if (push_base())
        done (1);

    nulav[0] = "";
    nulav[1] = "";
    nulav[2] = NULL;
    init();
    rcname = malloc (strlen(home_dir) + strlen(MAILRC) + 2);
    strcpy (rcname, home_dir);
    strcat (rcname, "/");
    strcat (rcname, MAILRC);
    for (i = 1; i < argc; ++i) {
        next = 0;
        if ((*argv[i] == '-') && options) {
            if (*(argv[i] + 1) == '\0') {
                options = 0;
                continue;
            }
            while (*++argv[i]) {
                switch (*argv[i]) {
                case 'O':
                    no_mail_overide = 1;
                    break;
                case 'l':
                    rcload  = 1;
                    if (i + 1 < argc  &&  *argv[i + 1] != '-') {
                        free (rcname);
                        oop = 1;
                        ++i;
                        ++next;
                        rcname = malloc (strlen (argv[i]) + 1);
                        strcpy (rcname, argv[i]);
                    }
                    break;
                case 'L':
                    rcload = 0;
                    break;
                case 'D':
                    Debug = 1;
                    break;
                case 'F':
                    if (++i < argc) {
                        add_extra (argv[i]);
                    } else {
                        puts (" -F Requires Field argument");
                        exit (1);
                    }
                    ++next;
                    break;
                case 'v':
                    set_var (LEVEL_SET, "verbose", "");
                    break;
                case 'o':
                    free (output_file);
                    if (i + 1 < argc  &&  *argv[i + 1] != '-') {
                        oop = 1;
                        ++i;
                        ++next;
                        output_file = malloc (strlen (argv[i]) + 1);
                        strcpy (output_file, argv[i]);
                    } else {
                        oop = -1;
                        output_file = malloc (strlen(home_dir) +
                                strlen(ALT_MBOX) + 2);
                        sprintf (output_file, "%s/%s", home_dir, ALT_MBOX);
                    }
                    break;
                case 'f':
                    if (i + 1 < argc  &&  *argv[i + 1] != '-') {
                        fop = 1;    
                        ++i;
                        ++next;
                        mail_file = realloc (mail_file, strlen (argv[i]) + 1);
                        strcpy (mail_file, argv[i]);
                    } else {
                        fop = -1;
                        mail_file = realloc (mail_file,
                                strlen(home_dir) + strlen(MBOX) + 2);
                        sprintf (mail_file, "%s/%s", home_dir, MBOX);
                    }
                    break;
                default:
                    puts ("dmail: Bad argument");
                    puts ("dmail -O      then 'help' for help.");
                    done (1);
                }
                if (next)
                    break;
            }
        } else {
            No_load_mail = 1;
            nameslist[nc++] = i;
        }
    }
    if (oop == -1  &&  fop == -1) {
        mail_file = realloc (mail_file, strlen(output_file) + 1);
        strcpy (mail_file, output_file);
    }
ends:
    initial_load_mail();
    m_select (nulav, M_RESET);
    Current = indexof (1);
    if (rcload) {
        ac = 2;
        av[1] = rcname;
        do_source(rcname, 0);
    }
    if (nc) {
        av[0] = "mail";
        for (i = 0; i < nc; ++i)
            av[i + 1] = argv[nameslist[i]];
        ac = nc + 1;
        do_reply ("", R_MAIL);
        done (0);
    }
    if (Entries + no_mail_overide == 0) {
        printf ("\nNO MAIL for %s\n\n", user_name);
        return (0);
    }
    printf ("\nRF %-20s   WF %-20s\n", mail_file, output_file);
    do {
        Retry = 20;
        pop_base();
loop:
        if (push_base()) {
            pop_base();
            if (Debug)
                printf ("TOP LEVEL INTR, Level: %d\n", Longstack);
            if (--Retry == 0)
                done (1);
            puts ("");
            goto loop;
        }
        check_new_mail();
    } while (do_command() > 0);
    return (0);
}



init()
{
    char *str;
    struct passwd *passwd;
    extern int sig_handle();

    Entry = (struct ENTRY *)malloc (sizeof(*Entry));
    Entry->status = Entry->no = Entry->fpos = 0;
    passwd = getpwuid(getuid());
    user_name   = malloc (strlen(passwd->pw_name) + 1);
    home_dir    = malloc (strlen(passwd->pw_dir) + 1);
    visual      = malloc (sizeof(VISUAL));
    strcpy  (visual     , VISUAL);
    strcpy  (user_name, passwd->pw_name);
    strcpy  (home_dir , passwd->pw_dir);
    if ((str = getenv ("HOME")) != NULL)
        strcpy ((home_dir = realloc (home_dir, strlen(str) + 1)), str);
    if ((str = getenv ("USER")) != NULL)
        strcpy ((user_name = realloc (user_name, strlen(str) + 1)), str);
    if ((str = getenv ("VISUAL")) != NULL)
        strcpy ((visual = realloc (visual, strlen(str) + 1)), str);
    mail_file   = malloc (strlen(MAILHOME) + strlen(user_name) + 1);
    sprintf (mail_file  , "%s%s", MAILHOME, user_name);
    output_file = malloc (strlen(home_dir) + 2 + sizeof(MBOX));
    sprintf (output_file, "%s/%s", home_dir, MBOX);
    fix_globals();
    signal (SIGHUP, sig_handle);
    signal (SIGINT, sig_handle);
    signal (SIGPIPE, SIG_IGN);
}


sig_handle()
{
    int mask = sigblock (0);

    sigsetmask (mask & ~((1 << SIGHUP) | (1 << SIGINT)));
    if (Longstack  &&  !Breakstack) 
        longjmp (env[Longstack], 1);
}


get_inode(file)
char *file;
{
    struct stat stats;

    if (stat (file, &stats) < 0)
        return (-1);
    return (stats.st_ino);
}

