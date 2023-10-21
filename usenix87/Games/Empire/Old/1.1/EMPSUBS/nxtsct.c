#define D_UPDATE
#define D_NATSTR
#define D_SCTSTR
#define D_DCHRSTR
#define D_ICHRSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

snxtsct(nsp, sectarg)
struct  nstr    *nsp;
char    *sectarg;
{
        register char   *cp;
        char    c, *bp, buf[16];

        if( sargs(sectarg) == -1 ) {
                printf("'%s'? -- Can't decipher sector(s) arg.\n", sectarg);
                return(-1);
        }
        nsp->n_lx = lx;
        nsp->n_hx = hx;
        nsp->n_ly = ly;
        nsp->n_hy = hy;
        nsp->n_ncond = 0;
        cp = condarg;
X146:   
        if( *cp == '\0' ) goto X442;
        for( bp = buf; (c = *cp++) != '\0'; bp++ ) {
                if( c == '<' ) break;
                if( c == '=' ) break;
                if( c == '>' ) break;
                if( c == '#' ) break;
                *bp = c;
        }
        if( c == '\0' ) {
                printf("'%s'? -- meaningless condition?\n", condarg);
                return(-1);
        }
        *bp = '\0';
        nsp->n_cond[nsp->n_ncond].n_oper = c;
        if( (nsp->n_cond[nsp->n_ncond].n_fld1 = encode(buf)) == -1 ) return(-1);
        if( (nsp->n_cond[nsp->n_ncond++].n_fld2 = encode(cp)) == -1 ) return(-1)
;
X416:   
        if( (c = *cp++) == '\0' ) goto X434;
        if( c != '&' ) goto X416;
X434:   
        if( c != '\0' ) goto X146;
X442:   
        nsp->n_ix = ix;
        nsp->n_iy = iy;
        nsp->n_x = nsp->n_lx - nsp->n_ix;
        nsp->n_y = nsp->n_ly;
        return(0);
}

nxtsct(nsp, selup)
struct  nstr    *nsp;
int     selup;
{
        register        i, oper;
        int     val1, val2, pkgs;

X10:    
        nsp->n_x += nsp->n_ix;
        if( nsp->n_x != nsp->n_hx ) goto X130;
        nsp->n_x = nsp->n_lx;
        nsp->n_y += nsp->n_iy;
        if( nsp->n_y != nsp->n_hy ) goto X130;
        return(0);
X130:   
        if( getsect(nsp->n_x, nsp->n_y, selup) <  0 ) goto X10;
        pkgs = dchr[sect.sct_desig].d_pkg;
        i = nsp->n_ncond;
X216:   
        i--;
        if( i <  0 ) goto X424;
        val1 = decode(nsp->n_cond[i].n_fld1, pkgs);
        val2 = decode(nsp->n_cond[i].n_fld2, pkgs);
        oper = nsp->n_cond[i].n_oper;
        if( oper != '<' ) goto X352;
        if( val1 >= val2 ) goto X424;
X352:   
        if( oper != '=' ) goto X370;
        if( val1 != val2 ) goto X424;
X370:   
        if( oper != '>' ) goto X406;
        if( val1 <= val2 ) goto X424;
X406:   
        if( oper != '#' ) goto X216;
        if( val1 != val2 ) goto X216;
X424:   
        if( i <  0 ) goto X434;
        goto X10;
X434:   
        return(1);
}

encode(string)
char    *string;
{
        register char   *cp;
        register        i;

        cp = string;
        if( *cp < '0' ) goto X34;
        if( *cp > '9' ) goto X34;
        return(atoi(cp) & 03777);
X34:    
        if( *(cp+1) == '\0' ) goto X52;
        if( *(cp+1) != '&' ) goto X156;
X52:    
        i = 0;
        goto X102;
X56:    
        if( *cp != dchr[i].d_mnem ) goto X100;
        return(i);
X100:   
        i++;
X102:   
        if( dchr[i].d_mnem != '\0' ) goto X56;
        printf("%s -- not a designation", cp);
        return(-1);
X156:   
        i = stmtch(cp, &ichr[0].i_name, sizeof(struct ichrstr));
        if( i <  0 ) goto X212;
        return(i | 04000);
X212:   
        printf("%s -- %s commodity", cp, ((i == -2) ? "ambiguous" : "not a"));
        return(-1);
}

decode(code, pkgs)
int     code, pkgs;
{
        register char   *cp;

        if( (04000 & code) != 0 ) {
                code &= 0177;
                cp = (char *)&sect + (int)code;
                code = ichr[code].i_pkg[pkgs] * *cp;
        }
        return(code);
}
