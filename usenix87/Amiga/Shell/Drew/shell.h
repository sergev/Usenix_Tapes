
/*
 * SHELL.H
 *
 * (c)1986 Matthew Dillon     9 October 1986
 *
 *
 * SHELL include file.. contains shell parameters and extern's
 *
 * version 2.06M (Manx Version and Additions) by Steve Drew 28-May-87
 *
 */

#include <stdio.h>
#include <time.h>
#include <exec/types.h>
#include <exec/exec.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <exec/memory.h>
#include <exec/tasks.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <functions.h>
#include <fcntl.h>

#define bmov   movmem
#define STDBUF 1

#define MAXAV	     256	    /* Max. # arguments		    */
#define MAXSRC	     5		    /* Max. # of source file levels */
#define MAXIF	     10		    /* Max. # of if levels	    */
#define MAXALIAS     20		    /* Max. # of alias levels	    */

#define LEVEL_SET    0		    /* which variable list to use   */
#define LEVEL_ALIAS  1
#define LEVEL_LABEL  2

#define V_PROMPT     "_prompt"	    /* your prompt (ascii command)   */
#define V_HIST	     "_history"	    /* set history depth (value)     */
#define V_HISTNUM    "_histnum"	    /* set history numbering var     */
#define V_DEBUG	     "_debug"	    /* set debug mode		     */
#define V_VERBOSE    "_verbose"	    /* set verbose for source files  */
#define V_STAT	     "_maxerr"	    /* worst return value to date    */
#define V_LASTERR    "_lasterr"	    /* return value from last comm.  */
#define V_CWD	     "_cwd"	    /* current directory	     */
#define V_EXCEPT     "_except"	    /* "nnn;command"		     */
#define V_PASSED     "_passed"	    /* passed arguments to source fle*/
#define V_PATH	     "_path"	    /* search path for external cmds */

	    /* EXECOM.C defines */

#define FL_DOLLAR    0x01  /* One of the following */
#define FL_BANG	     0x02
#define FL_PERCENT   0x04
#define FL_QUOTE     0x08
#define FL_IDOLLAR   0x10  /* Any or all of the following may be set */
#define FL_EOC	     0x20
#define FL_EOL	     0x40
#define FL_OVERIDE   0x80
#define FL_WILD	     0x100
#define FL_MASK	     (FL_DOLLAR|FL_BANG|FL_PERCENT|FL_QUOTE)


#define VERSION	  "V2.06M  (c)1986 Matthew Dillon.  Manx version by Steve Drew"

#ifndef NULL
#define NULL 0L
#endif

#define CHECKBREAK() ( breakcheck() ? (printf("^C\n"),1) : 0)

#ifndef AZTEC_C
struct _dev	 {
		long  fd;
		short mode;
	};
#endif

struct HIST {
   struct HIST *next, *prev;	 /* doubly linked list */
   char *line;			 /* line in history    */
};

struct PERROR {
   int errnum;			 /* Format of global error lookup */
   char *errstr;
};

struct DPTR {			 /* Format of directory fetch pointer */
   struct FileLock *lock;	 /* lock on directory	*/
   struct FileInfoBlock *fib;	 /* mod'd fib for entry */
};

extern struct HIST *H_head, *H_tail;
extern struct PERROR Perror[];
extern struct DPTR *dopen();
extern char *set_var(), *get_var(), *next_word();
extern char *get_history(), *compile_av();
extern char *malloc(), *strcpy(), *strcat();
extern char **expand();
extern char *av[];
extern char *Current;
extern int  H_len, H_tail_base, H_stack;
extern int  E_stack;
extern int  Src_stack, If_stack;
extern int  ac;
extern int  debug, Rval, Verbose, disable, Quit;
extern int  Lastresult;
extern int  Exec_abortline, Exec_ignoreresult;
extern int   S_histlen;
extern long  Uniq;
extern long  Cin, Cout, Cout_append;
extern char *Cin_name, *Cout_name;
extern char  Cin_type,	Cout_type;  /* these variables are in transition */
extern char *Pipe1, *Pipe2;

extern long Src_base[MAXSRC];
extern long Src_pos[MAXSRC];
extern char If_base[MAXIF];
extern struct Process *Myprocess;
