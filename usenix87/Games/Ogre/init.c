#include "ext.h"
#define UNDO	'^'

static char a, b;
static int cp_set;
static int infantry_points, armor_points, n_free;

init_units(mark)
{
    int unitcmp();

	init_screen();

    a = 10;
    b = 10;

    switch(mark) {

        case 0:			/* Raider O.G.R.E. a toughie */
            armor_points = 48;
            infantry_points = 12;
            break;

        case 1:
            armor_points = 8;	/* O.G.R.E I a creampuff */
            infantry_points = 12;
            break;

        case 2:			/* O.G.R.E. II hmm.... */
            armor_points = 12;
            infantry_points = 12;
            break;

        case 3:			/* O.G.R.E. III that's the idea! */
            armor_points = 40;
            infantry_points = 18;
            break;

        case 4:			/* O.G.R.E. IV more of the same */
            armor_points = 40;
            infantry_points = 24;
            break;

        case 5: 		/* O.G.R.E. V mama bear */
            armor_points = 72;
            infantry_points = 27;
            break;

        case 6:			/* O.G.R.E. VI papa bear */
            armor_points = 80;
            infantry_points = 33;
            break;

        case 7:			/* O.G.R.E. VII granddad bear */
            armor_points = 120;
            infantry_points = 60;
            break;

        case 8:			/* O.G.R.E. VII WHAT!!! */
            armor_points = 200;
            infantry_points = 72;
            break;

    }

    n_units = n_free = 0;
    cp_set = FALSE;

    while(armor_points > 0 || infantry_points > 0 || !cp_set) {
        display(16, "left to place: %d armor, %d infantry%s",
            armor_points, infantry_points,
            (cp_set) ? "." : ", CP");
        getunit();
    }

    /* sort the units so lower the i, the more valuable the unit. */
    qsort( (char *) unit, n_units, sizeof(UNIT), unitcmp);


}

getunit()
{
    char    no_new, bad_char;
    char    olda, oldb;
    char    dir, i;

    no_new = TRUE;
    bad_char = FALSE;

    movecur_hex(a, b);

    while(no_new) {

        olda = a;
        oldb = b;

        dir = readchar();
    
        switch(dir) {
    
            case RIGHT:
                a--;
                b--;
                break;
    
            case UPRIGHT:
                a--;
                break;
    
            case DOWNRIGHT:
                b--;
                break;
    
            case LEFT:
                a++;
                b++;
                break;
    
            case UPLEFT:
                b++;
                break;
    
            case DOWNLEFT:
                a++;
                break;
    
	    case UNDO:
		if ((i = occupied(a, b)) == 0 || unit[--i].status != OK) {
		    bad_char = TRUE ;
		    break ;
		    }
		if (unit[i].type == CP) {
			cp_set = FALSE ;
			armor_points += unit[i].movement*4 ;
			}
		else if (unit[i].type == INFANTRY) infantry_points += 1 ;
		else if (unit[i].type == HOWITZER) armor_points += 8 ;
		else if (unit[i].type == HVYTANK) armor_points += 4 ;
		else if (unit[i].type == MSLTANK) armor_points += 4 ;
		else if (unit[i].type == GEV) armor_points += 4 ;
		else if (unit[i].type == LGTTANK) armor_points += 3 ;
		else if (unit[i].type == MHOW) armor_points += 8 ;
		else if (unit[i].type == BUNKER) armor_points += 10 ;
		else broken("Internal error in init!") ;

		if (i < n_free) n_free = i ;
		unit[i].status = DESTROYED ;
		update_hex(a, b) ;
		no_new = FALSE ;
		break ;
			
            case CP:
                if(cp_set) {
                    bad_char = TRUE;
                }
                else {
                    add_unit(a, b, dir);
                    no_new = FALSE;
                    cp_set = TRUE;
                }
                break;

            case HVYTANK:
            case MSLTANK:
            case GEV:
                if(occupied(a, b) || blocked(a, b) || armor_points < 4) {
                    bad_char = TRUE;
                    break;
                }
                add_unit(a, b, dir);
                no_new = FALSE;
                armor_points = armor_points - 4;
                break;
    
	    case BUNKER:
                if(occupied(a, b) || blocked(a, b) || armor_points < 10) {
                    bad_char = TRUE;
                    break;
                }
                add_unit(a, b, dir);
                no_new = FALSE;
                armor_points = armor_points - 10;
                break;
    
	    case LGTTANK:
                if(occupied(a, b) || blocked(a, b) || armor_points < 3) {
                    bad_char = TRUE;
                    break;
                }
                add_unit(a, b, dir);
                no_new = FALSE;
                armor_points = armor_points - 3;
                break;
    
            case INFANTRY:
		dir = '3' ;
	    case '3':
	    case '2':
	    case '1':
		dir = dir - '0' ;
                if(blocked(a, b) || infantry_points < dir) {
                    bad_char = TRUE;
                    break;
                }
		if ((i = occupied(a, b)) != 0)
		    if (unit[--i].type != INFANTRY
		    || infantry_on(a, b) + dir > 3) {
			bad_char = TRUE ;
			break ;
			}
		while (dir--) {
                    add_unit(a, b, 'I');
                    infantry_points -= 1 ;
		    }
                no_new = FALSE;
                break;
    
            case HOWITZER:
                if(occupied(a, b) || blocked(a, b) || armor_points < 8) {
                    bad_char = TRUE;
                    break;
                }
                add_unit(a, b, dir);
                no_new = FALSE;
                armor_points = armor_points - 8;
                break;
    
	    case MHOW:
                if(occupied(a, b) || blocked(a, b) || armor_points < 8) {
                    bad_char = TRUE;
                    break;
                }
                add_unit(a, b, dir);
                no_new = FALSE;
                armor_points = armor_points - 8;
                break;
            default:
                bad_char = TRUE;
                break;
    
        }
    
        if(off_obstructed(a, b)  || 
            bad_char)
        {

	    putchar(BEEP) ;
            a = olda;
            b = oldb;
            bad_char = FALSE;
    
        }

        else {

            movecur_hex(a, b);

        }

    }

}

add_unit(a, b, c)
char a, b, c;
{
    int i, j;

    i = n_free;
    if (n_free == n_units) {
    	n_free = ++n_units ;
	unit[i].status = DESTROYED ;
	}
    else
    	while (++n_free < n_units)
	    if (unit[n_free].status == DESTROYED) break ;

    if (unit[i].status != DESTROYED)
	broken("Using non-free unit in add_unit!") ;
    if (n_units > N_UNITS)
	broken("Out of Units. Recompile with larger N_UNITS!") ;

    switch(c) {

        case CP:
            unit[i].type = CP;
            unit[i].attack = 0;
            unit[i].range = 0;
            unit[i].defend = 0;
	    j=10;
	    while ((j>2)||(j<0)&&(armor_points>j*4)){
		display(17, "Movement points for CP? ", 0) ;
		j = getchar() ;
 		if (j >= '0' && j <= '2') 
                      {j -= '0' ;}
                  else 
                      j = 0;
		display(17, "                        ", 0) ;
	    }
	    movecur(17, 0); eeol() ;
            unit[i].movement = j;
	    armor_points -= 4*j;
            break; 

        case HVYTANK:
            unit[i].type = HVYTANK;
            unit[i].attack = 4;
            unit[i].range = 2;
            unit[i].defend = 3;
            unit[i].movement = 3;
            break; 

        case BUNKER:
            unit[i].type = BUNKER;
            unit[i].attack = 0;
            unit[i].range = 0;
            unit[i].defend = 6;
            unit[i].movement = 0;
            break; 

        case LGTTANK:
            unit[i].type = LGTTANK;
            unit[i].attack = 2;
            unit[i].range = 2;
            unit[i].defend = 2;
            unit[i].movement = 3;
            break; 

        case MSLTANK:
            unit[i].type = MSLTANK;
            unit[i].attack = 3;
            unit[i].range = 4;
            unit[i].defend = 2;
            unit[i].movement = 2;
            break; 

        case GEV:
            unit[i].type = GEV;
            unit[i].attack = 2;
            unit[i].range = 2;
            unit[i].defend = 2;
            unit[i].movement = 4;
            break; 

        case HOWITZER:
            unit[i].type = HOWITZER;
            unit[i].attack = 6;
            unit[i].range = 8;
            unit[i].defend = 1;
            unit[i].movement = 0;
            break; 

        case MHOW:
            unit[i].type = MHOW;
            unit[i].attack = 6;
            unit[i].range = 6;
            unit[i].defend = 1;
            unit[i].movement = 1;
            break; 

	case INFANTRY:
            unit[i].type = INFANTRY;
            unit[i].attack = 1;
            unit[i].range = 1;
            unit[i].defend = 1;
            unit[i].movement = 2;
            break; 

    }

    unit[i].range_to_ogre = 0;
    unit[i].fired = 0;
    unit[i].status = OK;
    unit[i].moves_left = 0;
    unit[i].l_hex = a;
    unit[i].r_hex = b;

    disp_unit(i);

}

occupied(a, b)
char a,b;
{
    int i;

    for(i = 0; i < n_units; i++)
        if(unit[i].status != DESTROYED &&
           unit[i].l_hex == a &&
           unit[i].r_hex == b &&
	   unit[i].type != BUNKER ) return(++i);

    return(FALSE);

}

infantry_on(a, b)
char a,b;
{
    int i, c;

    for (c = i = 0; i < n_units; i++)
	if (unit[i].type == INFANTRY && unit[i].status != DESTROYED
	&& unit[i].l_hex == a && unit[i].r_hex == b)
	    c++ ;

    return c ;

}
	   
init_ogre(mark)
{

    ogre.l_hex = rand() % 7 + 22; /* 22 - 28 */
    ogre.r_hex = 50 - ogre.l_hex;

    switch(mark) {

        case 1:
            ogre.treads = 15;
            ogre.init_treads = 15;
            ogre.movement = 3;
            ogre.missiles = 0;
	    ogre.launchers = 0;
            ogre.main_bats = 1;
            ogre.sec_bats  = 0;
            ogre.ap = 2;
            break;

        case 2:
            ogre.treads = 30;
            ogre.init_treads = 30;
            ogre.movement = 3;
            ogre.missiles = 0;
	    ogre.launchers = 0;
            ogre.main_bats = 1;
            ogre.sec_bats  = 2;
            ogre.ap = 6;
            break;

        case 3:
            ogre.treads = 45;
            ogre.init_treads = 45;
            ogre.movement = 3;
            ogre.missiles = 2;
	    ogre.launchers = 0;
            ogre.main_bats = 1;
            ogre.sec_bats  = 4;
            ogre.ap = 8;
            break;

        case 4:
            ogre.treads = 45;
            ogre.init_treads = 45;
            ogre.movement = 3;
            ogre.missiles = 2;
	    ogre.launchers = 0;
            ogre.main_bats = 2;
            ogre.sec_bats  = 4;
            ogre.ap = 8;
            break;

        case 5:
            ogre.treads = 60;
            ogre.init_treads = 60;
            ogre.movement = 3;
            ogre.missiles = 5;
	    ogre.launchers = 0;
            ogre.main_bats = 2;
            ogre.sec_bats  = 6;
            ogre.ap = 10;
            break;

        case 6:
            ogre.treads = 75;
            ogre.init_treads = 75;
            ogre.movement = 3;
            ogre.missiles = 5;
	    ogre.launchers = 2;
            ogre.main_bats = 4;
            ogre.sec_bats  = 10;
            ogre.ap = 12;
            break;

        case 0:
            ogre.treads = 45;
            ogre.init_treads = 45;
            ogre.movement = 4;
            ogre.missiles = 2;
	    ogre.launchers = 2;
            ogre.main_bats = 1;
            ogre.sec_bats  = 2;
            ogre.ap = 12;
            break;

        case 7:
            ogre.treads = 90;
            ogre.init_treads = 90;
            ogre.movement = 4;
            ogre.missiles = 6;
	    ogre.launchers = 4;
            ogre.main_bats = 8;
            ogre.sec_bats  = 12;
            ogre.ap = 30;
            break;

        case 8:
            ogre.treads = 140;
            ogre.init_treads = 140;
            ogre.movement = 5;
            ogre.missiles = 10;
	    ogre.launchers = 12;
            ogre.main_bats = 12;
            ogre.sec_bats  = 24;
            ogre.ap = 60;
            break;

    }


    disp_ogre();

}

unitcmp(u1, u2)
UNIT *u1, *u2;
{
    int cmp;

    switch(u1 -> type) {

        case CP:

            switch(u2 -> type) {

                case CP: 
                    cmp = 0;
                    break;

                default:
                    cmp = -1;
                    break;

            }

            break;

        case HOWITZER:
            switch(u2 -> type) {

                case CP: 
                    cmp = 1;
                    break;

                case HOWITZER:
                    cmp = 0;
                    break;

                default:
                    cmp = -1;
                    break;

            }

            break;

        case HVYTANK:
            switch(u2 -> type) {

                case CP:
                case HOWITZER:
                    cmp = 1;
                    break;

                case HVYTANK:
                    cmp = 0;
                    break;

                default:
                    cmp = -1;
                    break;

            }

            break;

        case MSLTANK:
            switch(u2 -> type) {

                case CP:
                case HOWITZER:
                case HVYTANK:
                    cmp = 1;
                    break;

                case MSLTANK:
                    cmp = 0;
                    break;

                default:
                    cmp = -1;
                    break;

            }

            break;

        case GEV:
            switch(u2 -> type) {

                case INFANTRY:
                    cmp = -1;
                    break;

                case GEV:
                    cmp = 0;
                    break;

                default:
                    cmp = 1;
                    break;

            }

            break;

        case INFANTRY:
            switch(u2 -> type) {

                case INFANTRY: 
                    cmp = 0;
                    break;

                default:
                    cmp = 1;
                    break;

            }

            break;

        }

    return(cmp);

}

broken(thing) char *thing; {

	clear_screen() ;
	reset_term() ;
    	printf("Internal error: %s\n", thing) ;
	exit(1) ;
	}
