#define D_UPDATE
#define D_SECTDES
#define D_SHIPTYP
#define D_NEWSVERBS
#define D_TRTYCLAUSE
#define D_SCTSTR
#define D_SHPSTR
#define D_DCHRSTR
#define D_MCHRSTR
#define D_FILES
#include        "empdef.h"

fire()
{
        register char   *cp;
        register        i;
        char     *xytoa(), *splur(), *getstar(), vfleet;
        int     ax, ay, vx, vy;
        int     aship, vship, alors, vlors;
        int     adef, vdef, vnum, dam, nshells;
        short   dx, dy;
        double  arng, vrng, agun, vgun, dsq, q, error;
        double  seagun(), tfact(), landgun(), sqrt();
        struct  shpstr  as, vs;

/* Get victim vlors, vnum, vx, vy, & vdef */

        cp = getstar(argp[1], "Firing on ");
        vlors = landorsea(cp);
        if( vlors == LAND ) {
                if( sargs(cp) == -1 ||
                    getsect(lx, ly, UP_ALL) == -1 ) return(SYN_RETURN);
                if( sect.sct_desig == S_SANCT ) {
                        printf("%d,%d is a %s!!", lx, ly, dchr[sect.sct_desig].d_name);
                        return(FAIL_RETURN);
                }
                vnum = sect.sct_owned;
                vx = lx;
                vy = ly;
                vdef = landdef(sect.sct_desig); /* banks=320 all others 160 */
                sprintf(fmtbuf,"Firing on sector %d,%d from ", vx, vy);
                cp = fmtbuf;
        } else {
                vship = getshno(cp, "Victim ship? ", &vs);
                if( vship == -1 ||
                    vs.shp_own == 0 ) {
                        printf("No such ship exists");
                        return(SYN_RETURN);
                }
                vnum = vs.shp_own;
                vfleet = vs.shp_fleet;
                vx = vs.shp_xp;
                vy = vs.shp_yp;
                vdef = seadef(vs.shp_type); /* m_prdct */
                sprintf(fmtbuf,"Firing on ship #%d from ", vship);
                cp = fmtbuf;
        }

/*  Get attacker alors, ax, ay, adef, agun & arng */

        cp = getstar(argp[2], cp);
        alors = landorsea(cp);
        if( alors == LAND ) {

/*  Attacking from land  */

                if( sargs(cp) == -1 ||
                    getmysect(lx, ly, UP_ALL) == -1 ) return(SYN_RETURN);
                if( sect.sct_desig != S_FORTR ) {
                        printf("Not a fortress");
                        return(SYN_RETURN);
                }
                if( sect.sct_guns == 0 ) {
                        printf("Insufficient arms");
                        return(SYN_RETURN);
                }
                if( sect.sct_shell == 0 ) {
                        printf("Klick!     ...");
                        return(FAIL_RETURN);
                }
                if( sect.sct_milit < 5 ) {
                        printf("Not enough military for firing crew");
                        return(FAIL_RETURN);
                }
                sect.sct_shell--;
                nshells = 1;
                arng = (sect.sct_guns < 7) ? sect.sct_guns : 7;
                arng = tfact(cnum, arng);
                agun = landgun(sect.sct_effic);
                adef = landdef(sect.sct_desig);
                putsect(lx, ly);
                ax = lx;
                ay = ly;
        } else {

/*  Attacking from sea  */

                if( getmyship(aship = atopi(cp), &as) == -1 ) return(SYN_RETURN);
                if( mchr[as.shp_type].m_frnge == 0 ||
                    as.shp_gun == 0 ) {
                        printf("Insufficient arms");
                        return(SYN_RETURN);
                }
                if( as.shp_shels == 0 ) {
                        printf("Klick!     ...");
                        return(FAIL_RETURN);
                }
                if( as.shp_effc < 60 ) {
                        printf("Ship #%d is crippled (%d%%)", aship, as.shp_effc);
                        return(FAIL_RETURN);
                }
                ax = as.shp_xp;
                ay = as.shp_yp;
                arng = tfact(cnum, mchr[as.shp_type].m_frnge / 2.);
                printf("Efficiency %d%%   max range %.2f  ", as.shp_effc, arng);
                i = (as.shp_gun > mchr[as.shp_type].m_gun) ? mchr[as.shp_type].m_gun : as.shp_gun;
                if( as.shp_type == S_TEN && i > 1 ) i = 1;
                i = (i < as.shp_shels) ? i : as.shp_shels;
                sprintf(fmtbuf,"How many guns (%d max) ?", i);
                nshells = onearg(argp[3], fmtbuf);
                nshells = (i < nshells) ? i : nshells;
                if( nshells == 0 ) return(FAIL_RETURN);
                as.shp_shels -= nshells;
                agun = seagun(as.shp_type, as.shp_effc, nshells);
                adef = seadef(as.shp_type);
                as.shp_mbl = (as.shp_mbl < 0) ? as.shp_mbl : 0;
                putship(aship, &as);
        }

/*  Calculate range to see if we have a hit or a miss  */

        dx = xwrap(vx - ax);
        dy = ywrap(vy - ay);
        dsq = dx*dx + dy*dy;
        if( (vlors == SEA) && (dsq > 50.) ) {
                printf("Victim ship not sighted");
                return(FAIL_RETURN);
        }
        if( dsq < 50. && vnum != 0 ) {
                if( vlors == LAND ) {
                        if( trechk(cnum, vnum, LANFIR) == -1 ) return(FAIL_RETURN);
                } else {
                        if( trechk(cnum, vnum, SEAFIR) == -1 ) return(FAIL_RETURN);
                }
        }
        printf("Kaboom!!!\n");
        error = dsq / (arng*arng);
        error = error*error;
        if( error > (rand() % 32768) / (5. * 32767.) + .9 ) {

/*  Missed - treat it like firing on a land sector; change x,y vdef & vnum  */

                q = arng / sqrt(dsq);
                vx = ax + (short)(dx * q);
                vy = ay + (short)(dy * q);
                printf("Out of range, shell%s short in %d,%d\n", ((nshells == 1) ? " falls" : "s fall"), vx, vy);
                vlors = LAND;
                if( getsect(vx, vy, UP_ALL) == -1 ) return(SYN_RETURN);
                vdef = landdef(sect.sct_desig);
                vnum = sect.sct_owned;
        } else {
                if( error * 32767. > rand() % 32768 ) {
                        agun *= .5;
                        printf("Wind deflects shell%s.\n", splur(nshells));
                }
        }
        if( vlors == LAND ) {
                getsect(vx, vy, UP_NONE);
                if( sect.sct_desig == S_SANCT ) {
                        printf("Bounce!");
                        return(FAIL_RETURN);
                }
                if( sect.sct_desig == S_WATER ) {
                        printf("SPLASH..");
                        return(FAIL_RETURN);
                }
        }
        sigsave();
        dam = shelldam(agun, vdef);
        ntused = agun / 20. + ntused + .5;
        vgun = 0;
        if( vlors == SEA ) goto X4120;

/*  Shell hit land  */

        nreport(cnum, N_SCT_SHELL, vnum);
        if( (vnum != 0) && (vnum != cnum) ) {
                sprintf(fmtbuf,"Country #%d shell%s did %d%% damage in %s", cnum, splur(nshells), dam, xytoa(vx, vy, vnum));
                wu(0, vnum, fmtbuf);
        }

/* Do damage to the sector, then see if it is a fort and can return fire */

        getsect(vx, vy, UP_NONE);
        sectdam(dam);
        printf("%d%% damage done to sector %d,%d\n", dam, vx, vy);
        if( vnum == cnum || vnum == 0 ) {
                putsect(vx, vy);
                return(NORM_RETURN);
        }
        vrng = tfact(vnum, (double)((sect.sct_guns < 7) ? sect.sct_guns : 7));
        vgun = landgun(sect.sct_effic);
        if( sect.sct_desig == S_FORTR &&
            sect.sct_shell != 0 &&
            dsq <=  vrng*vrng &&
            vgun > 0 ) {
                sect.sct_shell--;
                putsect(vx, vy);
                fireback(alors, vnum, ax, ay, aship, vgun, adef);
                sprintf(fmtbuf,"Sector %s returned fire on %s %s", xytoa(vx, vy, vnum), ((alors == LAND) ? "sector" : "ship at"), xytoa(ax, ay, vnum));
                wu(0, vnum, fmtbuf);
        } else {
                putsect(vx, vy);
        }

/*  See if the sector is defended, and if defending fort can return fire  */

        getsect(vx, vy, UP_NONE);
        if( sect.sct_dfend == 0 ) return(NORM_RETURN);
        dx = sect.sct_dfend<<8;
        dx = (dx>>12) + vx;
        dy = sect.sct_dfend<<12;
        dy = (dy>>12) + vy;
        getsect(dx, dy, UP_NONE);
        if( vnum == sect.sct_owned &&
            sect.sct_desig == S_FORTR &&
            sect.sct_shell !=  0 ) {
                q = min(sect.sct_guns, 7);
                if( tfact(vnum, q) < idist(dx -ax, dy - ay) ) return(NORM_RETURN);
                sect.sct_shell--;
                putsect(dx, dy);
                vgun = landgun(sect.sct_effic);
                fireback(alors, vnum, ax, ay, aship, vgun, adef);
               sprintf(fmtbuf,"Defending sector %s returned fire on %s %s", xytoa(dx, dy, vnum), ((alors == LAND) ? "sector" : "ship at"), xytoa(ax, ay, vnum));
                wu(0, vnum, fmtbuf);
        }
        return(NORM_RETURN);

/*  Shell hit ship  */

X4120:  
        shipdam(&vs, dam);
        nreport(cnum, N_SHP_SHELL, vnum);
        sprintf(fmtbuf,"Country #%d shell%s did %d%% damage to  %s #%d", cnum, splur(nshells), dam, mchr[vs.shp_type].m_name, vship);
        wu(0, vnum, fmtbuf);
        putship(vship, &vs);
        if( vs.shp_effc > 20 ) {        /* if ship still floats */
                printf("%d%% damage done to %s #%d\n", dam, mchr[vs.shp_type].m_name, vship);
        }

/* See if any ships in the fleet can return fire on attacker  */

        for( i = 0; getship(i, &ship) != -1; i++ ) {
                if( vnum != ship.shp_own ||
                    vfleet != ship.shp_fleet ||
                    ship.shp_gun == 0 ||
                    ship.shp_shels == 0 ||
                    ship.shp_type == S_FRE ) continue;
                if( idist(ax - ship.shp_xp, ay - ship.shp_yp) > tfact(vnum, mchr[ship.shp_type].m_frnge / 2.) ) continue;
                nshells = (ship.shp_gun < ship.shp_shels) ? ship.shp_gun : ship.shp_shels;
                if( ship.shp_type == S_TEN && nshells > 1 ) nshells = 1;
                if( (vgun = seagun(ship.shp_type, ship.shp_effc, nshells)) == 0 ) continue;
                ship.shp_shels -= nshells;
                putship(i, &ship);
                sprintf(fmtbuf,"Ship #%d returned fire on %s %s", i, ((alors == LAND) ? "sector" : "ship at"), xytoa(ax, ay, vnum));
                wu(0, vnum, fmtbuf);
                if( fireback(alors, vnum, ax, ay, aship, vgun, adef) == -1 ) return(NORM_RETURN);
        }
        return(NORM_RETURN);
}

/*
        Fire back on attacking fort or ship.  Return -1 for sinking
        attacking ship.  Return 0 for everything else.
*/

fireback(alors, vnum, ax, ay, aship, vgun, adef)
int     alors, vnum, ax, ay, aship, adef;
double  vgun;
{
        int     dam;
        struct  shpstr  as;

        printf("Return fire!\n");
        nreport(vnum, N_FIRE_BACK, cnum);
        dam = shelldam(vgun, adef);
        if( alors == LAND ) {
                getsect(ax, ay, UP_NONE);
                sectdam(dam);
                putsect(ax, ay);
        } else {
                getship(aship, &as);
                shipdam(&as, dam);
                putship(aship, &as);
                if( as.shp_effc <= 20 ) {
                        sprintf(fmtbuf,"Country #%d %s #%d at %s sunk!\n", cnum, mchr[as.shp_type].m_name, aship, xytoa(ax, ay, vnum));
                        wu(0, vnum, fmtbuf);
                        return(-1);
                }
        }
        return(0);
}
