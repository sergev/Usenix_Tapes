/*
 * alert.h - main include file
 */

#define VERSION "1.0"
#define SYSNAME "4.2 BSD"

#define NAMSIZ	10		/* no of chars in a username */
#define NOLINES	10		/* no of lines that one guy can login */

struct hunter {
    char name[NAMSIZ];		/* username */
    char nottys;		/* noofttys */
    char *ttys[NOLINES];	/* tty devs */
};

struct time_alarm {
    time_t atime;		/* alarmtime */
    char   *amsg;		/* alarm msg */
    struct hunter *areq;	/* who */
    struct time_alarm *link;	/* next */
};

/* think this datastructure through a little more carefully
 * this is the one that's going to affect performance drastically..
 */

struct hunted {
    char name[NAMSIZ];		/* his name */
    char *ttys[NOLINES];	/* lines he is logged in on */
    char stats[NOLINES];
    char ostats[NOLINES];
    char idlest[NOLINES];
};

/*
 * New watch stuctures.
 */

struct watcher {
    char name[NAMSIZ];
    char *ttys[NOLINES];
    struct watcher *next_watcher;
};

/*
 *  type: USER   - watching a user
 *        LINE   - watch a tty line
 *        HOST   - watch a host going up or down
 *        MAIL   - watch a user's mailbox
 *
 *  Note: Not all fields in the structure are used for all watch types.
 */
struct watched {
    char type;                      /* see above */
    char name[NAMSIZ];             /* name of the user being watched */
    char host[NAMSIZ];             /* host of user */
    char *ttys[NOLINES];	    /* lines he is logged in on */
    char ttys_state[NOLINES];       /* watching this line or not */
    char stats[NOLINES];
    char ostats[NOLINES];
    char idlest[NOLINES];
    struct watcher_list *watcher_list;  
    struct watched *next_watched;   /* link to the next one of these fields */
};

struct watcher_list {
    struct watcher_list *watcher_list_next;
    struct watcher *watcher;
};


    
    
    

      
