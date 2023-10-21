
/*  exceptions.h
 *
 *  Code to provide exception handling.
 *  Written by George Bisset - June 1986.
 *
 *  Version 1.5
 *
 *  No responsibility is taken for any errors or inaccuracies inherent either to
 *  the comments or the code of this program, but if reported to me then an
 *  attempt will be made to fix them.
 */

#include <setjmp.h>

#define  MAXEXLV  5     /* Maximum number of nested exceptions. */

typedef struct
          {
            int cl ;
            jmp_buf jb[ MAXEXLV ] ;
          } exceptn ;

#define   exception(x)         exceptn x = { 0 } ;
#define exexception(x)  extern exceptn x  ;

/* x is of type exception */
#define when(x)   switch(setjmp(x.jb[x.cl++])) { \
                  default : (void) fprintf(stderr,"Unexpected exception raised") ; \
                            exit(1) ; \
                  case 0  : break ;

/* x is of type integer */
#define etype(x)  case x :

#define endwhen   }

#define forget(x)    x.cl--

/* x is of type exception and y is an integer */
#define raise(x,y)    longjmp(x.jb[x.cl - 1],y)
