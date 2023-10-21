/*
*
* Getargs.c
*
*!AU: Michael A. Shiels
*!CD: 21-May-86
*!FR: Dr. Dobb's May 1985
*
*/

#define GETARGS_C
#define LINT_ARGS
#include <stdio.h>
#include <stdlib.h>
#include "masdos.h"
#include "getargs.h"

extern void usage();

char	ARG_Switch = '/';
int		ARG_ICase = 0;

static  char    *setarg( argp, linep )
ARG             *argp ;
char            *linep ;
{
        ++linep ;
 
        switch( argp->type )
        {
        case    ARG_INTEGER :
                *argp->variable = stoi( &linep ) ;
                break ;
 
        case    ARG_FBOOLEAN :
                *argp->variable = 1;
                break ;
 
        case    ARG_TBOOLEAN :
                *argp->variable = 0;
                break ;
 
        case    ARG_SBOOLEAN :
                *argp->variable = ~*argp->variable;
                break ;
 
        case    ARG_CHARACTER :
                *argp->variable = *linep++ ;
                break ;
 
        case    ARG_STRING :
                *(char **)argp->variable = linep ;
                linep = "" ;
                break;

        default :
                fprintf( stderr, "GetArgs: Bad Argument Type\n" ) ;
                break ;
        }
        return( linep ) ;
}
 
static  ARG     *findarg( c, tabp, tabsize )
int             c, tabsize ;
ARG             *tabp ;
{
        for( ; --tabsize >= 0 ; tabp++ )
        {
			if (ARG_ICase && toupper(tabp->arg) == toupper(c) )
				return( tabp );
			if ( tabp->arg == c )
				return( tabp );
		}
		return( NULL );
}
 
static  pr_usage( tabp, tabsize )
ARG		*tabp;
int		tabsize;
{
        for( ; --tabsize >= 0 ; tabp++ )
        {
                switch( tabp->type )
                {
                case    ARG_INTEGER :
                        fprintf(stderr, "%c%c<num> %-40s (value is ",
                                ARG_Switch, tabp->arg, tabp->errmsg) ;
                        fprintf(stderr, "%-5d)\n", *(tabp->variable) ) ;
                        break ;
 
                case    ARG_TBOOLEAN :
                        fprintf(stderr, "%c%c      %-40s (value is ",
                                ARG_Switch, tabp->arg, tabp->errmsg) ;
                        fprintf(stderr, "%-5s)\n", *(tabp->variable)
                                ? "True" : "False") ;
                        break ;
 
                case    ARG_FBOOLEAN :
                        fprintf(stderr, "%c%c      %-40s (value is ",
                                ARG_Switch, tabp->arg, tabp->errmsg) ;
                        fprintf(stderr, "%-5s)\n", *(tabp->variable)
                                ? "True" : "False") ;
                        break ;
 
                case    ARG_CHARACTER :
                        fprintf(stderr, "%c%c<c>   %-40s (value is ",
                                ARG_Switch, tabp->arg, tabp->errmsg) ;
                        fprintf(stderr, "%-5c)\n", *(tabp->variable) ) ;
                        break ;
 
                case    ARG_STRING :
                        fprintf(stderr, "%c%c<str> %-40s (value is ",
                                ARG_Switch, tabp->arg, tabp->errmsg) ;
                        fprintf(stderr, "<%s>)\n",
                                *(char **)tabp->variable) ;
                        break ;

                }
        }
}
 
#define ARG_ERRMSG "Illegal argument <%c>. Legal arguments are:\n\n"
 
int             getargs( argc, argv, tabp, tabsize )
int             argc, tabsize ;
char            **argv ;
ARG             *tabp ;
{
        register int    nargc ;
        register char   **nargv, *p ;
        register ARG    *argp ;
        char buf[2], *bufp = buf;

		if( (bufp = getenv("SWITCHAR")) )
			ARG_Switch = *bufp;

        nargc = 1 ;
        for( nargv = ++argv ; --argc > 0 ; argv++ )
        {
                if( **argv != ARG_Switch )
                {
                        *nargv++ = *argv ;
                        nargc++ ;
                }
                else
                {
                        p = ( *argv ) + 1 ;
 
                        while( *p )
                        {
                                if( argp = findarg( *p, tabp, tabsize ) )
                                {
                                        p = setarg( argp, p ) ;
                                }
                                else
                                {
                                        usage();
                                        fprintf( stderr, ARG_ERRMSG, *p );
                                        pr_usage( tabp, tabsize );
                                        exit( 1 );
                                }
                        }
                }
        }
        return nargc ;
}
