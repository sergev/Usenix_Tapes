/*
 * %W% (mrdch&amnnon) %G%
 */

# include "header"

# define TORETURN 3
/* times to press '-' for a message to vanish */

extern  changestat;

/*
 * This tiny function exists only for the menu "write"
 * function. This fuction is called only with the
 * command string.
 */

write_enemy (s)
char   *s;
{
    /* THIS SHOULD NOT BE HERE BUT THIS WAS THE ONLY EDITED FILE */
    (void) chdir("/usr/games/lib/galaxy");
    skipwhite (s);
    dowrite_enemy (s, !player);
}

/*
 * Here we link a new piece of information to the list of
 * messages that apears on the status line beneath the map.
 * If this information is the first, the bell on the terminal
 * of the player that has received the message is ringed.
 * Here is the structure definition.
 *
 * typedef struct _info    info;
 * struct _info {
 *     int     owner;
 *     int          nmsg ;
 *     char    msg[MSGSIZ];
 *     info * next;
 * };
 */

dowrite_enemy (s, p)
char   *s;
int     p;
{
    FILE * newtty = tty;        /* keep the current terminal     */
    info * head = info_head[p];
    info * ialloc ();
    info * inf = ialloc ();     /* get a new chunk of space      */
    char * cp = s;

    inf -> owner = player;
    if ( !*s )
        return ;                /* return on empty lines        */
    if(*s < 15) {
        s[MSGSIZ-1] = '\0';     /* truncate too long msgs        */
        flip();
        print("WRITE BOGUS FOUND %d AT END OF STRING\n\r", s[MSGSIZ-1]);
        print("string is %s\n", s);
        print("s[0]=%d s[1]=%d s[2]=%d s[3]=%d s[4]=%d s[5]=%d\n\r",
        s[0],s[1],s[2],s[3],s[4],s[5]);
        while(*cp < 15)
                cp++;
        print("first knowledgable character begins at %d\n\r", cp-s);
        flip();
    }
    (void) strcpy (inf -> msg, s);

    if (head == 0) {            /* NO -More- */
        if (changestat)
            tty = ttys[!p];
        else
            tty = ttys[p];      /* the correct terminal to ring */
        disch ('\07');          /* ring it                       */
        tty = newtty;           /* restore the previous one      */
    }
    dolink (&head, inf);
    info_head[p] = head;
}

/* actively display the messages gathered on screen */
/* REWRITE */
putmsgs (cc)
char    cc;
{
    static int  nreturns = TORETURN;
    static int  nid = 1;        /* flag to indicate repeated request */
    info * i = info_head[player];

    if (cc == '-')
        nreturns = TORETURN;    /* he wants to see NOW */
    if (nreturns++ < TORETURN)
        return;
    if (i == 0) {               /* no msgs */
        if (nid)
            return;             /* nothing there to show or clear */
        nreturns = TORETURN;
        cleol (21, 0);          /* clear the last one */
        nid = 1;
        return;
    }
    nreturns = 0;
    nid = 0;
    cleol (21, 0);              /* clear the last one */
    msg ("%s", i -> msg);               /* display that message */
    if (i -> next != 0)
        so (20, 5, "--More--"); /* show that more are pending */
    else
        cleol (20, 5);          /* erase the -more- message      */
    info_head[player] = i -> next;/* advance to the next one     */
    ifree (i);                  /* and throw the last    */
}

/*
 * This part contains all functions dealing with linked lists.
 * linked list of type `info' are used by : ps, en, wr, newyear.
 */

# define        HEAD    (*head)

/*
 * perform linking of a new info structure.
 * only the nmsg is updated here.
 */
dolink (head, inf)
info ** head;
info * inf;
{
    info * i = HEAD;

    inf -> next = 0;            /* after it? nothing yet */
    if (HEAD == 0) {            /* if list empty-        */
        HEAD = inf;             /* point to the first    */
        inf -> nmsg = 1;        /* and declare it so     */
        return;
    } else {
        while (i -> next)       /* search to it's end    */
            i = i -> next;

        inf -> nmsg = i -> nmsg + 1;
        i -> next = inf;        /* and link it there     */
    }
}

spy_msg (msg, own, pl, lvl)
char   *msg;
int     own;
planet * pl;
int     lvl;
{
    info * ialloc ();
    info * inf = ialloc ();

    (void) sprintf (inf -> msg, "Y: %d L: %d %s", year,
            lvl, msg);
    inf -> owner = own;

    dolink (&(pl -> reports), inf);
}

/*
 * delete the first few msgs in order to scroll old msgs.
 */
scroll_msgs (head, player)
info ** head;
{
    info * h = HEAD;
    info * x, *x1;

    int     nmsgs = count_msgs (h, player);

    if (nmsgs <= N_PSI)
        return;

    x1 = 0;
    x = HEAD;
    while (nmsgs > N_PSI && x -> next)  {
        if(x->owner == player) {                /* remove it. */
                if(x == HEAD) {
                        x = x->next;
                        ifree(HEAD);
                        HEAD = x;
                        nmsgs--;
                } else {
                        x1 ->next = x->next;
                        ifree(x);
                        nmsgs--;
                        x = x1 -> next;
                }
        } else {
                x1 = x;
                x = x->next;
        }
    }
}

/*
 * allocate space for info structure.
 */
info *
ialloc () {
    char   *malloc ();
    char   *p = malloc (sizeof (info));

    if (p == 0)
        (void) bug ("Unable to allocate space for messages.");

    ((info *) p) -> next = 0;
    return ((info *) p);
}

ifree (i)
info * i;
{
# ifndef ENQDBG
# define ENQDBG 0
# else
# undef ENQDBG
# define ENQDBG 1
# endif

    if(!(debuggingmode() || ENQDBG))
        free ((char *) i);
}
