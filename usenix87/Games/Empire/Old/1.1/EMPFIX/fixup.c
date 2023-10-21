extern int      xflg, wflg, mflg;

/*
        n = ptr to name
        v = current value
        w = default value
*/
fixup(n, v, wv)
char    *n;
int     v, wv;
{
        char    *cp, *getstri();

        if( wflg ) return(wv);
        if( xflg ) return(v);
        printf("  %6s %d  ", n, v);
        cp = getstri("");
        if( *cp == 'x' ) {
                xflg++;
                return(v);
        }
        if( *cp == 'w' ) {
                wflg = xflg = mflg = 1;
                return(wv);
        }
        if( *cp == '\0' ) return(v);
        v = atoi(cp);
        mflg++;
        return(v);
}
