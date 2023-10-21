
#define VERSION "1.0"
#define SYSNAME "4.2 BSD"

/* max no of users expected to be on the system at any time */
#define MAXUSERS 20 

/* max no of ttys that one user can be expected to be on */
#define NO_TTYS  8

/* max no of chars in a uname, line name */
#define NMAX     8

/* max users that one user can watch at a time */
#define MAX_WATCH  10

#define MAX_ALARMS               50000

struct time_alarm {
    int  id;			/* unique id */
    char name[NMAX];		/* user who requested this */
    char msg[128];		/* msg at most 128 chars log */
    int  alarmtime;		/* when to ring the alarm */
    struct time_alarm *next;
};

struct ttyentry {
    char name[8];
    long idletime;		
    char touch;			
};

struct uentry {
    char name[8];
    char being_watched;		/* flag that tells us whether we should
                                   compute the idle time */
    char no_ttys;
    struct ttyentry ttys[NO_TTYS];
    struct uentry *next;
};

struct hunter {
    char name[NMAX];
    struct hunter *next;
    char hunted[MAX_WATCH][NMAX];
    int  idletime[MAX_WATCH];
    char imagic[MAX_WATCH][NO_TTYS];/* dont ask me about this kluge */
    char hmagic[MAX_WATCH][NO_TTYS];
    int  no_hunted;
};

/* status opcodes */
#define IDLE                      1
#define ACTIVE                    2
#define LOGIN                     3
#define LOGOUT                    4

#define ADD_USER                  1
#define DELETE_USER               2
#define STATUS                    3
#define OVERALL_STATUS            4
#define FLUSH_USER                5

#define ADD_TIME_ALARM           10
#define DELETE_TIME_ALARM        11
#define SHOW_TIME_ALARMS         12
#define SHOW_ALL_ALARMS          13
#define DELETE_ANY_ALARM         14

#define CONTROL_QUIT   		 20
#define CONTROL_QUIT_INTERACTIVE 21
#define CONTROL_KILL   		 22
#define CONTROL_STATUS 		 23
#define CONTROL_TUNE   		 24


