#define D_UPDATE
#define D_SECTDES
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_FILES
#include        "empdef.h"

rout()
{
        register char   *cp, *dp;
        register        i;
        char    c, line[132], *getstar();
        int     j;
        short   dx, dy;

        if( sargs(argp[2]) != -1 ) goto X36;
        return(SYN_RETURN);
X36:    
        if( (hx - lx) * ix <= 41 ) goto X104;
        printf("Max width (in sectors) is %d", 41);
        return(SYN_RETURN);
X104:   
        cp = getstar(argp[1], "item? ");
        for( i = 12; (c = ichr[i].i_mnem) != '\0'; i++ ) {
                if( c == *cp ) break;
        }
        if( c != 0 ) goto X212;
        printf("'%s'? The union won't let us deliver them!", cp);
        return(SYN_RETURN);
X212:   
        dp = (char *)&sect.sct_c_use + i - 12;
        printf("    ");
        for( i = lx; i != hx; i += ix ) {
        printf((i < -9) ? "%3d" : "%2d ", i);
        }
        printf("\n");
        for( j = ly; j != hy; j += iy ) {
                cp = line;
                for( i = lx; i != hx; i += ix ) {
                        if( getsect(i, j, UP_NONE) == -1 || owner == 0 ) {
                                *cp++ = ' ';
                                *cp++ = ' ';
                                *cp++ = ' ';
                                continue;
                        }
                        c = dchr[sect.sct_desig].d_mnem;
                        if( *dp == 2 ) {
                                *cp++ = '$';
                                *cp++ = c;
                                *cp++ = '$';
                                continue;
                        }
                        if( sect.sct_desig == S_XCHNG ) *dp = 0;
                        dx = (*dp)<<12;
                        dx = dx>>14;
                        dy = (*dp)<<14;
                        dy = (dy>>14) + 1;
                        switch( dx ) {
                        case -1:
                                *cp++ = "\\</"[dy];
                                *cp++ = c;
                                *cp++ = ' ';
                                break;
                        case 0:
                                *cp++ = "^ v"[dy];
                                *cp++ = c;
                                *cp++ = "^ v"[dy];
                                break;
                        case 1:
                                *cp++ = ' ';
                                *cp++ = c;
                                *cp++ = "/>\\"[dy];
                                break;
                        }
                }
                *cp = '\0';
                printf("%3d %s %d\n", j, line, j);
        }
        printf("    ");
        for( i=lx; i != hx; i += ix ) {
                printf((i < -9) ? "%3d" : "%2d ", i);
        }
        printf("\n");
        return(NORM_RETURN);
}
