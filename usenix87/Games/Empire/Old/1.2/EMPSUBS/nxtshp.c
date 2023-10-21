#define D_SHPSTR
#define D_ICHRSTR
#define D_MCHRSTR
#define D_NSCSTR
#define D_FILES
#include        "empdef.h"

snxtshp(nbsp, ap, cntry, np)
struct  nbstr   *nbsp;
char    *ap, *np;
int     cntry;
{
        char    c, *getstri();
        int     i;

        nbsp->nb_sno = -1;
        nbsp->nb_cno = cntry;
        nbsp->nb_mode = 0;
        nbsp->nb_scnt = 0;
        nbsp->nb_fleet = 0;
        nbsp->nb_lx = nbsp->nb_hx = 0;
        nbsp->nb_ly = nbsp->nb_hy = 0;
        nbsp->nb_ncond = 0;
        cndcod(nbsp);
        if( ap == 0 ||
            *ap == '\0' ) {
                if( np == 0 ||
                    *np == '\0' ) return(0);
                ap = getstri(np);
        }
        if( *ap == '\0' ) return(-1);
        if( landorsea(ap) == LAND ) goto X266;
        goto X542;
X266:   
        nbsp->nb_mode = -2;
        if( sargs(ap) == -1 ) return(2);
        if( ix >= 0 ) {
                nbsp->nb_lx = lx;
                nbsp->nb_hx = hx - 1;
        } else {
                nbsp->nb_lx = hx + 1;
                nbsp->nb_hx = lx;
        }
        nbsp->nb_ix = ix;
        if( iy >= 0 ) {
                nbsp->nb_ly = ly;
                nbsp->nb_hy = hy - 1;
        } else {
                nbsp->nb_ly = hy + 1;
                nbsp->nb_hy = ly;
        }
        nbsp->nb_iy = iy;
        return(0);
X542:   
        c = (*ap == '~') ? ' ' : *ap;
        if( c < 'a' ) goto X562;
        if( c <= 'z' ) goto X612;
X562:   
        if( c < 'A' ) goto X602;
        if( c <= 'Z' ) goto X612;
X602:   
        if( c != ' ' ) goto X640;
X612:   
        nbsp->nb_mode = -1;
        nbsp->nb_fleet = c;
        return(0);
X640:   
        for( i=0; i < NBLISTMAX; ) {
                if( *ap < '0' || *ap > '9' ) break;
                nbsp->nb_nums[i++] = atoip(&ap);
                if( *ap++ != '/' ) break;
        }
        if( i < 1 ) return(-1);
        nbsp->nb_mode = i;
        return(0);
}

cndcod(nbsp)
struct  nbstr   *nbsp;
{
        register char   *cp, *bp;
        char    c, buf[32];

        cp = condarg;
X14:    
        if( *cp == '\0' ) goto X320;
        for( bp = buf; (c = *cp++) != '\0'; ) {
                if( c == '<' ) break;
                if( c == '=' ) break;
                if( c == '>' ) break;
                if( c == '#' ) break;
                *bp++ = c;
        }
        if( c == '\0' ) {
                printf("'%s'? -- meaningless condition?\n", buf);
                return(-1);
        }
        *bp = '\0';
        nbsp->nb_cond[nbsp->nb_ncond].n_oper = c;
        nbsp->nb_cond[nbsp->nb_ncond].n_fld1 = bencode(buf);
        nbsp->nb_cond[nbsp->nb_ncond++].n_fld2 = bencode(cp);
X274:   
        if( (c = *cp++) == '\0' ) goto X312;
        if( c != '&' ) goto X274;
X312:   
        if( c != '\0' ) goto X14;
X320:   
        return(0);
}

nxtshp(nbsp, sp)
struct  nbstr   *nbsp;
struct  shpstr  *sp;
{
        register        i;
        int     xp, yp;

        if( nbsp->nb_mode == 0 ) goto X22;
        goto X512;
X22:    
        i = nbsp->nb_sno += 1;
        if( getship(i, sp) != -1 ) goto X74;
        nbsp->nb_sno = -1;
X66:    
        return(0);
X74:    
        if( sp->shp_own == 0 ) goto X22;
        if( cndtst(nbsp, sp) == 0 ) goto X22;
        goto X502;
X126:   
        i = nbsp->nb_sno += 1;
        if( getship(i, sp) != -1 ) goto X172;
X164:   
        nbsp->nb_sno = 0;
        goto X66;
X172:   
        if( sp->shp_own == 0 ) goto X126;
        if( sp->shp_fleet != nbsp->nb_fleet ) goto X126;
        if( cndtst(nbsp, sp) == 0 ) goto X126;
        goto X502;
X242:   
        i = nbsp->nb_sno += 1;
        if( getship(i, sp) == -1 ) goto X164;
        if( cndtst(nbsp, sp) == 0 ) goto X242;
/*
        Get the x,y coordinates of the ship and try to position it
        within the range specified.  This assumes that snxtshp makes
        hx >= lx and hy >= ly .
*/
        xp = sp->shp_xp;
        while( xp > nbsp->nb_hx ) xp -= w_xsize;
        while( xp < nbsp->nb_lx ) xp += w_xsize;
        if( xp > nbsp->nb_hx ) goto X242;
        yp = sp->shp_yp;
        while( yp > nbsp->nb_hy ) yp -= w_ysize;
        while( yp < nbsp->nb_ly ) yp += w_ysize;
        if( yp > nbsp->nb_hy ) goto X242;
X502:   
        return(1);
X512:   
        if( nbsp->nb_mode != -1 ) goto X534;
        goto X126;
X534:   
        if( nbsp->nb_mode == -2 ) goto X242;
        if( nbsp->nb_mode >  0 ) goto X570;
        return(0);
X570:   
        i = nbsp->nb_scnt++;
        if( i <  nbsp->nb_mode ) goto X636;
        nbsp->nb_scnt = 0;
        goto X66;
X636:   
        i = nbsp->nb_sno = nbsp->nb_nums[i];
        if( getship(i, sp) == -1 ) goto X732;
        if( sp->shp_own == 0 ) goto X732;
        if( cndtst(nbsp, sp) != 0 ) goto X502;
X732:   
        printf("You have no ship #%d\n", i);
        goto X570;
}

cndtst(nbsp, sp)
struct  nbstr   *nbsp;
struct  shpstr  *sp;
{
        register        i;
        int     val1, val2, oper;

        if( nbsp->nb_cno == 0 ) goto X46;
        if( sp->shp_own == nbsp->nb_cno ) goto X46;
X40:    
        return(0);
X46:    
        i = nbsp->nb_ncond;
X56:    
        if( --i <  0 ) goto X276;
        val1 = bdecode(nbsp->nb_cond[i].n_fld1, sp);
        val2 = bdecode(nbsp->nb_cond[i].n_fld2, sp);
        oper = nbsp->nb_cond[i].n_oper;
        if( oper != '<' ) goto X216;
        if( val1 >= val2 ) goto X276;
X216:   
        if( oper != '=' ) goto X236;
        if( val1 != val2 ) goto X276;
X236:   
        if( oper != '>' ) goto X256;
        if( val1 <= val2 ) goto X276;
X256:   
        if( oper != '#' ) goto X56;
        if( val1 != val2 ) goto X56;
X276:   
        if( i >= 0 ) goto X40;
        return(1);
}

bencode(string)
char    *string;
{
        register char   *cp;
        register        i;

        cp = string;
        if( *cp >= '0' &&
            *cp <= '9' ) return(atoi(cp));
        if( *(cp + 1) == '\0' ||
            *(cp + 1) == '&' ) {
                for( i=0; mchr[i].m_name != '\0'; i++ ) {
                        if( *cp == *mchr[i].m_name ) return(i);
                }
                printf("%s -- not a ship designation", cp);
                return(-1);
        }
        i = stmtch(cp, &ichr[0].i_name, sizeof(struct ichrstr));
        return(i | 0200);
}

bdecode(code, sp)
int     code;
struct	shpstr	*sp;
{
        char    *cp;

        if( (code & 0200) != 0 ) {
                code &= 0177;
		cp = (int)ichr[code].i_shp + (char *)sp;
	/* horrible kluge for handling mobility as a short */
		if( cp == (char *)&sp->shp_mbl ) {
			code = sp->shp_mbl;
		} else {
			code = *cp;
		}
        }
        return(code);
}
