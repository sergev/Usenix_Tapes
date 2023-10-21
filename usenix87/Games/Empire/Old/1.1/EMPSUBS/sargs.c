#define D_NATSTR
#define D_FILES
#include        "empdef.h"

sargs(string)
char    *string;
{
        register char   c;
        register        i;
        char    *cp, *getstri();

        cp = string;
        if( cp == 0 ) goto X24;
        if( *cp != '\0' ) goto X40;
X24:    
        cp = getstri("(sects)? ");
X40:    
        c = *cp;
        if( c != '#' ) goto X204;
        cp++;
        i = 0;
        if( *cp < '0' ) goto X104;
        i = *cp - '0';
        cp++;
X104:   
        lx = nrealm[i].b_xl;
        hx = nrealm[i].b_xh;
        ly = nrealm[i].b_yl;
        hy = nrealm[i].b_yh;
        goto X446;
X204:   
        lx = ly = hx = hy = 0;
        if( c > '9' ) goto X250;
        if( c >= '0' ) goto X260;
        if( c == '-' ) goto X260;
X250:   
        return(-1);
X260:   
        lx = hx = atoip(&cp);
        if( *cp != ':' ) goto X334;
        cp++;
        hx = atoip(&cp);
X334:   
        if( *cp++ != ',' ) goto X250;
        if( *cp < '-' ) goto X250;
        ly = hy = atoip(&cp);
        if( *cp != ':' ) goto X436;
        cp++;
        hy = atoip(&cp);
X436:   
        if( *cp == '/' ) goto X250;
X446:   
        ix = (lx > hx) ? -1 : 1;
        iy = (ly <= hy) ? 1 : -1;
        hx += ix;
        hy += iy;
        return(0);
}
