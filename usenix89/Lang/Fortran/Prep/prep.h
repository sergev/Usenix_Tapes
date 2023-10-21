#include "stdio.h"
#include "string.h"

/* define C compiler here */
#define CRAY			0	/* Cray C 1.0 under CTSS */
#define SVS			0	/* Silicon Valley C */


#define BLANK			' '
#define TAB			'\t'
#define TRUE			1
#define FALSE			0
#define	NOT			!
#define	DEF_UNROLL_DEPTH	8
#define	DEF_LINE_LIMIT		1
#define DEF_BUFFSIZE		200
#define	STORE_SIZE		1000
#define	NESTING			10
#define	MAX_TOKENS		2*NESTING	/* tokens and macro args */
#define exp			expression	/* used exp as a variable */

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

#define	IN_BUFF_DONE		in_buff[0] = NULL ;

#define	UNROLLING		( ( unroll_depth >  1          ) && \
				  ( mem_count    <= line_limit ) && \
				  ( var_count    >  1          ) )

#define	GET_MEM(S,A)\
if ( NULL == (S = malloc(A+1)) ) {\
	abort( "Memory allocation failed") ; }

#define MATCH(S)	( strncmp( combuff, S, (name_length=strlen(S)) ) == 0 )

#define put_string(s)	fputs( s, out ) ; putc( '\n', out ) ;


/* enumeration of command types */
enum Command {
type_begin, type_again, type_while, type_until,	type_leave, type_continue,
type_case, type_of, type_default, type_end_case, type_continue_case,
type_do_limits, type_osqb, type_csqb, type_vec, type_unroll,
type_do, type_end_do, type_leave_do, type_continue_do,
type_include,
type_ifdef, type_ifndef, type_else, type_endif,
normal, unknown
} ;

 
#if CRAY

/* the cray considers characters to be unsigned */
#undef	EOF
#define EOF	255

/* a few macros to adapt to cray namelength limitations */
#define continue_proc		cont_proc
#define continue_do_proc	cont_do_proc
#define leave_do_proc		le_do_proc
#define include_proc		inc_proc

#endif


#if SVS
#define fopen	fopena		/* cr-lf and ^Z conversion */
#endif

/* function type declarations */
char		*mat_del(), *line_end(), *get_rec(), get_a_char(),
		*malloc(), *calloc(), *realloc(), *strtokp(),
		*mac_proc(), *strupr(), *strtok(), *strchrq() ;



#ifdef	MAIN
/*	Included stuff for main routine of program PREP  */

/* global pointers & storage */
char	*in_buff, *out_buff ;		/* text buffer pointers */
char	*first_nonblank ;		/* first nb char in in_buff */
char	*mem_store[STORE_SIZE] ;	/* pointers to malloc areas */
char	errline[2*DEF_BUFFSIZE] ;	/* error message line */
char	dataf[DEF_BUFFSIZE] ;		/* data file name */

long	allocation ;          /* current size of in_buff */
int	tab_size = 7 ;        /* size of the tab in blanks */
int	unroll_depth = 0 ;    /* do loop unroll depth, 0 for no unrolling */
int	line_limit = 1000 ;   /* unroll loops if # lines <= line_limit */
int	mem_count = 0 ;       /* mem_store external counter */
int	include_count = 0 ;   /* index of filestack (for includes) */
int	name_length = 0 ;     /* current command name length */
int	vec_flag = FALSE ;    /* TRUE if in vector loop */
int	com_keep = FALSE ;    /* TRUE to keep comments */
int	underline_keep=FALSE; /* TRUE to keep underline characters */
int	macro_only = FALSE ;  /* TRUE to do only macro expansion */
int	ignore_flag = FALSE ;  /* conditional compilation flag */

FILE	*in, *out, *filestack[NESTING] ;



#else

/* Header stuff for the functions of program PREP */

/* global pointers & storage */
extern char	*in_buff, *out_buff, *mem_store[],
		*first_nonblank, dataf[], errline[] ;

extern int	tab_size, unroll_depth, line_limit, com_keep, vec_flag,
		mem_count, underline_keep, include_count, macro_only,
		name_length, ignore_flag ;

extern long	allocation ;

extern	FILE	*in, *out, *filestack[] ;

#endif
