#include        <stdio.h>

prmptrd(prompt, bufp, len)
char    *prompt, *bufp;
int     len;
{
        register        char    *cp;
        register        i;

        printf("%s", prompt);
        fflush(stdout);
        cp = bufp;
        for( i=0; i < len; i++ ) {
                if( read(0, cp++, 1) != 1 ) return(0);
                if( *(cp-1) == '\n' ) break;
        }
        *(cp-1) = '\0';
        return(cp - bufp);
}
