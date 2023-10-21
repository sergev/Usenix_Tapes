#define D_UPDATE
#define D_NATSTAT
#define D_SECTDES
#define D_SCTSTR
#define D_FILES
#include        "empdef.h"

int     gslx, gsly, gsli;

getsect(i, j, selup)
int     i, j, selup;
{
        register        k;
        long    addr, lseek();

        k = (((long)capy + j + w_ysize)%w_ysize)*w_xsize + ((long)capx + i + w_xsize)%w_xsize;
        addr = (long)k * sizeof(sect);
        lseek(sectf, addr, 0);
        if( read(sectf, &sect, sizeof(sect)) < sizeof(sect) ) {
                sprintf(fmtbuf,"getsect(%d, %d) failed for country #%d", i, j, cnum);
                erlog(fmtbuf);
                return(-1);
        }
        if( (sect.sct_desig & 0177) > S_XCHNG ) {
                printf("Program Error (sct_desig)  Save output & notify %s (%s)\n", privname, privlog);
                sprintf(fmtbuf,"getsect(%d, %d) got a desig=%d for cnum=%d", i, j, sect.sct_desig, cnum);
                erlog(fmtbuf);
                return(-1);
        }
        gslx = i;
        gsly = j;
        gsli = k;
        owner = (cnum == sect.sct_owned || nstat == STAT_GOD) ? 1 : 0;
        if( selup == UP_NONE ) return(0);
        if( cnum != 0 || (selup & (UP_OWN|UP_ALL|UP_TIME|UP_GOD)) ) {
                if( update(i, j, selup) == 1) putsect(i, j);
        }
        return(0);
}

putsect(i, j)
int     i, j;
{
        register        k;
        register struct sctstr  *sp;
        struct sctstr   auxsect;
        long    addr, lseek();

        sp = &sect;
        if( sp->sct_desig <= S_XCHNG ) goto X136;
X32:    
        printf("Program error! Save output & notify %s (%s)\n", privname, privlog);
        abort();
        sprintf(fmtbuf,"putsect(%d, %d) got a desig=%d for cnum=%d", i, j, sp->sct_desig , cnum);
        erlog(fmtbuf);
        return(-1);
X136:   
        if( sp->sct_desig != S_BSPAN ) goto X264;
        if( sp->sct_effic >= 20 ) goto X264;
        sp->sct_desig = S_WATER;
        sp->sct_mobil = sp->sct_dfend = sp->sct_civil = 0;
        sp->sct_milit = sp->sct_shell = sp->sct_guns = 0;
        sp->sct_plane = sp->sct_ore = sp->sct_gold = 0;
        printf("A bridge span at %d,%d has collapsed\n", i, j);
X264:   
        if( sp->sct_civil != 0 ) goto X302;
        if( sp->sct_milit != 0 ) goto X302;
        sp->sct_owned = 0;
X302:   
        if( sp->sct_owned != 0 ) goto X312;
        sp->sct_lstup = 0;
X312:   
        k = (((long)w_ysize + j + capy)%w_ysize)*w_xsize + ((long)w_xsize + i + capx)%w_xsize;
        if( gsli == k ) goto X570;
        printf("Program error! Save output & notify %s (%s).\n", privname, privlog);
        abort();
        sprintf(fmtbuf,"putsect(%d, %d) followed getsect(%d, %d)", i, j, gslx, gsly);
        erlog(fmtbuf);
        return(-1);
X570:   
        addr = (long)k * sizeof(sect);
        lseek(sectf, addr, 0);
        read(sectf, &auxsect, sizeof(auxsect));
        if( sect.sct_desig == S_WATER && auxsect.sct_desig != S_WATER &&
            auxsect.sct_desig != S_BSPAN ) goto X32;
        if( sect.sct_desig == S_MOUNT && auxsect.sct_desig != S_MOUNT ) goto X32;
        if( sect.sct_desig == S_RURAL && 
            (auxsect.sct_miner != sect.sct_miner ||
            auxsect.sct_gmin != sect.sct_gmin) ) goto X32;
        lseek(sectf, addr, 0);
        write(sectf, &sect, sizeof(sect));
        return(0);
}
