/*
*
*!AU: Michael A. Shiels
*!CD: 1-Jun-86
*!FR: Dr. Dobbs July 1985
* 
*/

#define LINT_ARGS
#include <stdio.h>
#include "masdos.h"

#define PTEXT_IBM

/*        ptext( dirc, dirv, Arg_Num_Cols, 79 / Arg_Num_Cols,
                (dirc/Arg_Num_Cols) + (dirc % Arg_Num_Cols != 0) ); */

ptexth( linec, linev, numcols, colwidth, numrows )
int             linec, numcols, colwidth;
char            **linev;
int             numrows;
{
        register int    j,i;
        register char   **lineend, **line, **nextline;

        D_IN("ptexth");
        D_I("ptexth","numcols=",numcols);
        D_I("ptexth","colwidth=",colwidth);
        D_I("ptexth","numrows=",numrows);
         
        lineend = &linev[linec - 1];
         
        for( j = numrows ; --j >= 0 ; )
        {
#ifdef PTEXT_IBM
                putchar( ' ' );
#endif
                for( i = numcols ; --i >= 0 && *linev ; linev++ )
                {
                        pr_line( *linev, colwidth,
                                linev <= lineend );
                }
                printf( "\n" );
        }
        D_OUT("ptexth");
}

ptextv( linec, linev, numcols, colwidth, numrows )
int             linec, numcols, colwidth;
char            **linev;
int             numrows;
{
        register int    j;
        register char   **lineend, **line, **nextline;

        D_IN("ptextv");
        D_I("ptextv","numcols=",numcols);
        D_I("ptextv","colwidth=",colwidth);
        D_I("ptextv","numrows=",numrows);
         
        lineend = &linev[linec - 1];
         
        for( j = numrows ; --j >= 0 ; )
        {
#ifdef PTEXT_IBM
                putchar( ' ' );
#endif
                for( line = linev++ ; line <= lineend ; line = nextline )
               {
                        nextline = line + numrows;
                        pr_line( *line, colwidth,
                                nextline <= lineend );
                }
                printf( "\n" );
        }
        D_OUT("ptextv");
}

static pr_line( str, width, padded )
register char  *str;
int            width, padded;
{
        int             col = 0;
         
        while( col < width && *str )
        {
                if( *str == '\n' )
                        break;
                
                else if( *str == '\r' )
                {
                        while( col > 0 )
                        {
                                --col;
                                putchar( '\b' );
                        }

                        str++;
                }
                else if( *str == '\t' )
                {
                        str++;
                        col++;
                        putchar( ' ' );
                         
                        while( ( col % 8 ) && col < width )
                        {
                                putchar( ' ' );
                                col++;
                        }
                }
                else if( *str == Q_ESC )
                {
                        putchar( *str++ );
                        if( !*str )
                                break;

                        putchar( *str++ );
                        if( !*str )
                                break;

                        putchar( *str++ );
                        if( !*str )
                                break;

                        putchar( *str++ );
                        if( !*str )
                                break;
                }
                else if( *str == '\b' )
                        --col;
                else if( *str == ' ' )
                        ++col;
                               
                putchar( *str++ );
                ++col;
        }
        if( padded )
                while( col++ < width )
                        putchar( ' ' );
}
