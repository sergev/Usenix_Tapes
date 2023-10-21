extern int      xflg, wflg, mflg;

/*
        n = ptr to name
        vp=ptr to current value
        wv=default value
*/
longfix(n, vp, wv)
char    *n;
long    *vp, wv;
{
        char    *cp, *getstri();
        long    atol();

        if( xflg ) return;
        if( wflg ) {
                *vp = wv;
                return;
        }
        printf("  %5s %ld  ", n, *vp);
        cp = getstri("");
        if( *cp == 'x' ) {
                xflg++;
                return;
        }
        if( *cp == '\0' ) return;
        *vp = atol(cp);
        mflg++;
        return;
}
