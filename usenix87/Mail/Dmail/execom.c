
/*
 *  EXECOM.C
 *
 *  Matthew Dillon, 6 December 1985
 *
 *
 *  (C) 1985  Matthew Dillon
 *
 *  Routines to parse and execute command lines.
 *
 *  Global Routines:    DO_COMMAND()
 *                      EXEC_COMMAND()
 *                      FIX()
 *
 *  Static Routines:    E_COMMAND()
 *                      BREAKOUT()
 *                      FIND_COMMAND()
 */


#include <pwd.h>
#include <stdio.h>
#include <strings.h>
#include "dmail.h"
#include "execom.h"

#define C_NO        1
#define C_NOEXPN    2
#define F_EXACT     0
#define F_ABBR      1
#define SCRBUF      1024

extern char *breakout();

extern int do_quit(), do_exit(), do_help(), do_list(), do_setlist();
extern int do_select(), do_type(), do_header(), do_next(), do_mark();
extern int do_unmark(), do_reply(), do_delnext();
extern int do_write(), do_shell(), do_set_var(), do_unset_var();
extern int do_number(), do_cd(), do_source(), do_defer(), do_echo();
extern int do_go();


struct COMMAND Command[] = {
        do_number   , 0,        0,                  "",
        do_mark     , 0,        ST_DELETED,         "delete",
        do_unmark   , 0,        ST_DELETED,         "undelete",
        do_header   , 0,        0,                  "header",
        do_type     , 0,        0,                  "type",
        do_echo     , 0,        0,                  "echo",
        do_go       , 0,        0,                  "go",
        do_reply    , 0,        R_REPLY,            "reply",
        do_reply    , 0,        R_INCLUDE,          "Reply", 
        do_reply    , 0,        R_MAIL,             "mail",
        do_reply    , 0,        R_FORWARD,          "forward",  
        do_select   , 0,        0,                  "select",
        do_select   , 0,        1,                  "reselect",
        do_defer    , 0,        1,                  "defer",
        do_list     , 0,        0,                  "list",
        do_next     , 0,        1,                  "next",
        do_next     , 0,        -1,                 "back",
        do_next     , 0,        2,                  "_next",
        do_next     , 0,        -2,                 "_back",
        do_delnext  , 0,        0,                  "dt",
        do_set_var  , 0,        0,                  "set",
        do_unset_var, 0,        0,                  "unset",    
        do_set_var  , C_NOEXPN, 1,                  "alias",
        do_unset_var, C_NOEXPN, 1,                  "unalias",  
        do_set_var  , C_NO,     2,                  "malias",
        do_unset_var, C_NO,     2,                  "munalias", 
        do_setlist  , 0,        0,                  "setlist",
        do_cd       , 0,        0,                  "cd",
        do_source   , 0,        0,                  "source",
        do_unmark   , 0,        ST_READ | ST_STORED,"preserve",
        do_mark     , 0,        ST_READ,            "mark",
        do_mark     , 0,        ST_TAG,             "tag",
        do_unmark   , 0,        ST_TAG,             "untag",
        do_unmark   , 0,        ST_STORED,          "unwrite",
        do_write    , 0,        0,                  "write",
        do_shell    , 0,        0,                  "!",
        do_exit     , 0,        0,                  "x",
        do_quit     , 0,        0,                  "quit",
        do_exit     , 0,        1,                  "xswitch",
        do_quit     , 0,        1,                  "qswitch",
        do_help     , 0,        0,                  "help",
        do_help     , 0,        0,                  "?",
        NULL        , 0,        0,                  NULL };

char *Desc[] = {
        "",
        "<list>                   mark messages for deletion",
        "<list>                   UNDELETE & UNMARK messages",
        "[msg]                    Display header of a message",
        "[msg]                    type a message",
        "args....                 Echo to the screen",
        "#                        Go to a message, don't print out",
        "                         reply to mail",
        "                         reply to mail, include recv'd text",
        "user user ...            send mail to users",
        "user user ...            forward mail to users",
        "Field [!]match [match][ , Field match.]  SELECT from entire message list",
        "Field [!]match [match][ , Field match.]  SELECT from current message list",
        "                         De-select any read messages",
        "<list>                   list mail as specified by SETLIST",
        "[msg]                    type/header next or message #",
        "[msg]                    type/header previous or message #",
        "[msg]                    go to next or message #",
        "[msg]                    go to previous or message #",
        "                         delete current, type next",
        "[var [string]]           set a variable",
        "var var var ...          unset a variable",
        "[var [string]]           set an alias",
        "var var var ...          unset an alias",
        "[var [string]]           set a mail alias",
        "var var var ...          unset a mail alias",
        "[-s] [cols] Field [cols] Field...    SET LIST format for LIST",
        "path                     CD to a directory",
        "file                     Source a file",
        "<list>                   UNREAD & UNMARK messages",
        "<list>                   mark messages as 'read'",
        "<list>                   tag messages for whatever",
        "<list>                   untag messages",
        "<list>                   unwrite messages",
        "file <list>              append messages to a file, delete on quit",
        "[command]                execute a shell [command]",
        "                         EXIT, do not save changes",
        "                         QUIT, update files",
        "from to                  Exit and switch to a new from/to file",
        "from to                  Quit and switch to a new from/to file",
        "[topic]                  help on a topic",
        "[topic]                  alternate form of HELP",
        NULL };


do_command()
{
    int i;
    char *str;
    static char comline[1024];

    printf ("%3d:", Entry[Current].no);
    fflush (stdout);
    if (gets (comline) == NULL)
        done (1);
    exec_command(comline);
    return (1);
}



/*
 * EXEC_COMMAND()
 *
 *
 */


struct MLIST {
    struct MLIST *next;
};

static struct MLIST *Mlist;

char *
mpush(amount)
int amount;
{
    struct MLIST *ml;

    push_break();
    ml = (struct MLIST *)malloc (amount + sizeof(Mlist));
    ml->next = Mlist;
    Mlist = ml;
    pop_break();
    return ((char *)Mlist + sizeof(Mlist));
}


char *
mpop()
{
    char *old = NULL;

    push_break();
    if (Mlist == NULL) {
        puts ("MLIST INTERNAL ERROR");
    } else {
        old = (char *)Mlist + sizeof(Mlist);
        free (Mlist);
        Mlist = Mlist->next;
    }
    pop_break();
    return (old);
}

mrm()
{
    push_break();
    while (Mlist) {
        free (Mlist);
        Mlist = Mlist->next;
    }
    pop_break();
}


exec_command(base)
char *base;
{
    char *str;
    int i;

    if (push_base()) {
        push_break();
        pop_base();
        mrm();
        pop_break();
        return (-1);
    }
    strcpy (str = mpush(strlen(base) + 1), base);
    i = e_command(str);
    if (mpop() != str) 
        puts ("POP ERROR");
    pop_base();
    return (i);
}


static
e_command(base)
char *base;
{
    char *com, *start, *Scr, *avline, *alias;
    int flag = 0;
    int i, pcount, len, ccno;

loop:
    com = breakout (&base, &flag);
    if (*com == '\0') {
        if (flag > 1)
            return (1);
        goto loop;
    }
    if ((ccno = find_command(com, F_EXACT)) < 0) {
        if (*com == '$') 
            alias = get_var (LEVEL_SET, com + 1);
        else
            alias = get_var (LEVEL_ALIAS, com);
        if (alias == NULL) {
            if ((ccno = find_command (com, F_ABBR)) < 0) {
                printf ("%s Command Not found\n", com);
                return (-1);
            } else {
                goto good_command;
            }
        }

        /* At this point, base points to arguments */

        start = (flag == 0) ? base : "";
        while (flag == 0) {             /* find ';' or end of string        */
            flag = -1;                  /* disable breakout's "" terminator */
            breakout (&base, &flag);
        }

        /*
         * At this point, start points to all arguments, base set up for next
         * string
         */

        if (*alias == '%') {
            int xx = 0;
            char *select;

            alias = strcpy (mpush (strlen(alias) + 1), alias);
            select = breakout (&alias, &xx);
            set_var (LEVEL_SET, select + 1, start);
            i = e_command (alias);
            unset_var (LEVEL_SET, select + 1);
            mpop();
        } else {
            com = mpush (strlen(alias) + strlen(start) + 2);
            strcpy (com, alias);
            strcat (com, (flag == 1) ? ";" : " ");
            strcat (com, start);
            i = e_command (com);
            if (mpop() != com)
                puts ("ME BAE ERROR");
        }
        if (i < 0)
            return (-1);
        if (flag > 1)
            return (1);
        goto loop;
    }
good_command:
    pcount = 0;
    if (Command[ccno].stat & C_NO  &&  Debug == 0) {
        printf ("%s  Is currently being developed\n", Command[ccno].name);
        return (-1);
    }
    if (Debug)
        printf ("Good command, Raw: %s\n", com);
    i = pcount = 0;
    av[i] = mpush (strlen(com) + 1);
    ++pcount;
    strcpy (av[i++], com);
    while (flag < 1) {
        com = breakout (&base, &flag);
        if (Debug)
            printf ("BREAKOUT %d %s\n", strlen(com), com);
        if (*com == '\0')
            continue;
        switch (*com) {
        case '~':
            if (com[1] == '/'  ||  com[1] == '\0') {
                av[i] = mpush (strlen(home_dir) + strlen(com + 1) + 1);
                ++pcount;
                strcpy (av[i], home_dir);
                strcat (av[i], com + 1);
            } else {
                struct passwd *passwd;
                char *user = com;

                while (*com) {
                    if (*com == '/') {
                        *com = '\0';
                        ++com;
                        break;
                    }
                    ++com;
                }
                if ((passwd = getpwnam(user)) == NULL) {
                    printf ("USER %s Not found\n", user);
                    while (pcount--)
                        mpop();
                    return (-1);
                }
                av[i] = mpush (strlen(passwd->pw_dir) + strlen(com) + 2);
                ++pcount;
                strcpy (av[i], passwd->pw_dir);
                if (*com) {
                    strcat (av[i], "/");
                    strcat (av[i], com);
                }
            }
            break;
        case '\"':
            av[i] = com + 1;
            while (*++com && *com != '\"');
            *com = '\0';
            break;
        case '$':
            av[i] = get_var (LEVEL_SET, com + 1);
            if (av[i] == NULL) {
                printf ("Variable: %s Not found\n", com + 1);
                while (pcount--)
                    mpop();
                return (-1);
            }
            av[i] = strcpy (mpush(strlen(av[i]) + 1), av[i]);
            ++pcount;
            break;
        default:
            av[i] = com;
            break;
        }
        ++i;
    }
    av[i] = NULL;
    ac = i;
    for (len = 0, i = 0; i < ac; ++i) 
        len += strlen (av[i]) + 1;
    avline = mpush (len + 1);
    *avline = '\0';
    for (i = 0; i < ac; ++i) {
        strcat (avline, av[i]);
        if (i + 1 < ac)
            strcat (avline, " ");
    }
    if (Debug)
        printf ("DEST: %s\n", avline);
    i = (*Command[ccno].func)(avline, Command[ccno].val);
    if (mpop() != avline)
        puts ("AVLINE ERROR");
    while (pcount--)
        mpop();
    fix();
    if (i < 0)
        return (i);
    if (flag < 2)
        goto loop;
    return (1);
}


/*
 * BREAKOUT
 *
 * Breakout next argument.  If FLAG is set to 1 on return, the argument 
 * returned is the last in the command.  If FLAG is set to 2 on return, the
 * argument returned is the last, period.
 *
 */

static char *
breakout(base, flag)
int *flag;
char **base;
{
    register char *str, *scr;

loop:
    str = *base;                        /* next start           */
    while (*str == ' ' || *str == 9)    /* skip spaces and such */
        ++str;
    switch (*str) {
    case '\0':                          /* no more arguments    */
        *flag = 2;
        *base = str;
        return (str);
    case ';':                           /* no more args in this command */
        *flag = 1;
        *str = '\0';
        *base = str + 1;
        return (str);
    }
    scr = str;
    for (;;) {                          /* valid argument of somesort   */
        switch (*scr) {
        case ' ':
        case 9:
            if (*flag >= 0)
                *scr = '\0';
            *base = scr + 1;
            *flag = 0;
            return (str);
        case '\"':
            ++scr;
            while (*scr && (*scr++ != '\"'));   /* place to end of quote */
            break;
        case '\0':
            *flag = 2;
            *base = scr;
            return (str);
        case ';':
            *flag = 1;
            *base = scr + 1;
            *scr = '\0';
            return (str);
        default:
            ++scr;
        }
    }
}



fix()
{
    register int i;

    for (i = Current; i < Entries; ++i) {
        if (Entry[i].no  &&  !(Entry[i].status & ST_DELETED)) {
            Current = i;
            return (1);
        }
    }
    if (Current >= Entries) {
        Current = Entries - 1;
        if (Current < 0)
            Current = 0;
    }
    for (i = Current; i >= 0; --i) {
        if (Entry[i].no  &&  !(Entry[i].status & ST_DELETED)) {
            Current = i;
            return (-1);
        }
    }
    Current = 0;
    return (-1);
}


static
find_command(str, arg)
char *str;
int arg;
{
    int i;
    int len = strlen (str);

    if (*str >= '0'  &&  *str <= '9')
        return (0);
    for (i = 0; Command[i].func; ++i) {
        if (strncmp (str, Command[i].name, len) == 0) {
            if (arg == F_ABBR)
                return (i);
            if (strcmp (str, Command[i].name) == 0)
                return (i);
            return (-1);
        }
    }
    return (-1);
}

