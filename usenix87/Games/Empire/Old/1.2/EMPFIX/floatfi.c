extern int      xflg, wflg, mflg;

/*
        n = ptr to name
        vp= ptr to current value
        wv= default value
*/

floatfi(n, vp, wv)
char    *n;
float   *vp, wv;
{
        char    *cp, *getstri();
        double  atof();

        if( xflg ) return;
        if( wflg ) {
                *vp = wv;
                return;
        }
        printf("  %5s %.2f  ", n, *vp);
        cp = getstri("");
        if( *cp == 'x' ) {
                xflg++;
                return;
        }
        if( *cp == '\0' ) return;
        *vp = atof(cp);
        mflg++;
        return;
}
