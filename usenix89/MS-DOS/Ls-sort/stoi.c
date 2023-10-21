/*
*
*!AU: Michael A. Shiels
*!CD: 26-May-86
*!FR: Dr. Dobbs May 85
*
*/

#define LINT_ARGS
#include <stdio.h>

#define islower(c)      ( 'a' <= (c) && (c) <= 'z' )
#define toupper(c)      ( islower(c) ? (c) - ( 'a' - 'A' ) : (c) )

int     stoi(instr)
register char   **instr;
{
        register int    num = 0;
        register char   *str;
        int             sign = 1;

        str = *instr;

        while( *str == ' ' || *str == '\t' || *str == '\n' )
                str++;

        if( *str == '-' )
        {
                sign = -1;
                str++;
        }
        if( *str == '0')
        {
                ++str;
                if( *str == 'x' || *str == 'X' )
                {
                        str++;
                        while( ( '0' <= *str && *str <= '9' ) ||
                               ( 'a' <= *str && *str <= 'f' ) ||
                               ( 'A' <= *str && *str <= 'F' ) )
                        {
                                num *= 16;
                                num += ( '0' <= *str && *str <= '9' ) ?
                                        *str - '0' :
                                        toupper(*str) - 'A' + 10;
                                str++;
                        }
                }
                else
                {
                        while( '0' <= *str && *str <= '7' )
                        {
                                num *= 8;
                                num += *str++ - '0';
                        }
                }
        }
        else
        {
                while( '0' <= *str && *str <= '9' )
                {
                        num *= 10;
                        num += *str++ - '0';
                }
        }
        *instr = str;
        return( num * sign );
}
