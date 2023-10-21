extern int      xflg, wflg, mflg;

/*
        ccp = ptr to char string to modify
*/
chfix(ccp)
char    *ccp;
{
        char    *cp, *getstri();

        if( xflg ) return;
        printf("%s  ", ccp);
        cp = getstri("");
        if( *cp == '\0' ) return;
        if( *cp == 'x' ) {
                xflg++;
                return;
        }
        do {
                *ccp++ = *cp++;
        } while( (*ccp++ = *cp++) != '\0' );
        mflg++;
        return;
}
