
/*
 * SHELL.H
 *
 * Matthew Dillon, 24 Feb 1986
 *
 */

#define MAXAV        128            /* Max. # arguments */

#define LEVEL_SET    0
#define LEVEL_ALIAS  1

#define M_RESET      0
#define M_CONT       1

#define V_PROMPT     "_prompt"      /* Special variable names */
#define V_HIST       "_history"

#define FL_DOLLAR    0x01  /* One of the following */
#define FL_BANG      0x02
#define FL_PERCENT   0x04
#define FL_QUOTE     0x08
#define FL_IDOLLAR   0x10  /* Any or all of the following may be set */
#define FL_EOC       0x20
#define FL_EOL       0x40
#define FL_OVERIDE   0x80
#define FL_WILD      0x100
#define FL_MASK      (FL_DOLLAR|FL_BANG|FL_PERCENT|FL_QUOTE)

#include <lattice/stdio.h>

struct HIST {
   struct HIST *next, *prev;     /* doubly linked list */
   char *line;                   /* line in history    */
};

struct PERROR {
   int errnum;                   /* Format of global error lookup */
   char *errstr;
};

struct DPTR {                    /* Format of directory fetch pointer */
   struct FileLock *lock;        /* lock on directory   */
   struct FileInfoBlock *fib;    /* mod'd fib for entry */
};

extern struct HIST *H_head, *H_tail;
extern struct PERROR Perror[];
extern struct DPTR *dopen();
extern char *set_var(), *get_var(), *next_word();
extern char *get_history(), *compile_av();
extern char *malloc(), *realloc(), *strcpy(), *strcat(), *AllocMem();
extern char **expand();

extern char *av[];
extern char *Current;
extern int  H_len, H_tail_base, H_stack;
extern int  ac;
extern int  Debug;
extern int  S_histlen;



