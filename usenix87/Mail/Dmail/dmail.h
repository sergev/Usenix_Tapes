
/*
 * DMAIL.H
 */

#define MAXTYPE      16     /* Max number of different fields remembered     */
#define EXSTART      3      /* Beginning of dynamic fields, rest are wired   */
#define MAXLIST      16     /* Maximum # list elements in SETLIST            */
#define LONGSTACK    64     /* Maximum # levels for the longjump stack       */
#define MAILMODE     0600   /* Standard mail mode for temp. files            */
#define MAXFIELDSIZE 4096   /* Maximum handlable field size (& scratch bufs) */

#define LEVEL_SET    0      /* which variable set to use                     */
#define LEVEL_ALIAS  1
#define LEVEL_MALIAS 2

#define R_INCLUDE   1       /* Include message      For DO_REPLY()  */
#define R_FORWARD   2       /* Forward message      */
#define R_REPLY     3       /* Reply to message     */
#define R_MAIL      4       /* Mail from scratch    */

#define M_RESET     0
#define M_CONT      1


#define PAGER(Puf)      _pager(Puf, 1)      /* Auto newline */
#define FPAGER(Puf)     _pager(Puf, 0)      /* output as is */
#define push_base()     (setjmp (env[1 + Longstack]) ? 1 : (++Longstack, 0))
#define pop_base()      --Longstack
#define push_break()    ++Breakstack
#define pop_break()     --Breakstack

#define ST_DELETED  0x0001  /* Status flag.. item has been deleted  */
#define ST_READ     0x0002  /* item has been read or marked         */
#define ST_STORED   0x0010  /* item has been written                */
#define ST_TAG      0x0020  /* item has been taged                  */
#define ST_SCR      0x0080  /* scratch flag to single out messages  */

#include <stdio.h>
#include <setjmp.h>

struct ENTRY {
    long fpos;
    int  no;
    int  status;
    char *from;
    char *fields[MAXTYPE];
};

static struct FIND {
    char *search;
    int  len;
    int  notnew;
};

extern char *getenv(), *malloc(), *realloc(), *next_word(), *get_field();
extern char *alloca();
extern char *get_var();

extern char *mail_file;
extern char *user_name;
extern char *output_file;
extern char *home_dir;
extern char *visual;
extern char Buf[];
extern char Puf[];
extern char *av[];
extern int  _ls, Longstack, Breakstack;
extern int  Debug;
extern int  Entries, Current;
extern int  Silence;
extern int  ac;
extern FILE *m_fi;
extern struct ENTRY *Entry;
extern struct FIND  Find[];
extern jmp_buf env[];

extern int width[], header[], Listsize;
extern int No_load_mail;

extern char *S_sendmail;
extern int S_page, S_novibreak, S_verbose, S_ask;

