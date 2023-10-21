same(a, b)
char    *a, *b;
{
        register char   *ap, *bp;

        ap = a;
        bp = b;

loop:   if( *ap != *bp++) return(0);
        if( *ap++ != 0) goto loop;
        return(1);
}
