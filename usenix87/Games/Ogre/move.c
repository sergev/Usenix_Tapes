/*
 *  Move the defender's units.
 *
 *  Michael Caplinger, Rice University, March 1982.
 */

#include "ext.h"

move_def()
{
    int i,j;
    char moreunits, l_old, r_old, m_old ;

    for (moreunits = TRUE; moreunits;) {
	moreunits = FALSE ;
	for(i = 0; i < n_units; i++)
	    if (unit[i].status == OK && unit[i].moves_left > 0) {
		describe_action("Move", i);
		m_old = unit[i].moves_left ;
		l_old = unit[i].l_hex ;
		r_old = unit[i].r_hex ;
		while(unit[i].moves_left > 0 && unit[i].status == OK)
		    if (getmove(i, l_old, r_old, m_old)) {
		    	moreunits = TRUE ;
			break ;
			}
		}
	}
	for(i=0;i<n_units;i++)
		switch (unit[i].type){
			case MHOW:
				unit[i].defend = DEFEND_MHOW;
				break;
			case HOWITZER:
				unit[i].defend = DEFEND_HOWITZER;
				break;
			case LGTTANK:
				unit[i].defend = DEFEND_LGTTANK;
				break;
			case MSLTANK:
				unit[i].defend = DEFEND_MSLTANK;
				break;
			case HVYTANK:
				unit[i].defend = DEFEND_HVYTANK;
				break;
			case INFANTRY:
				unit[i].defend = DEFEND_INFANTRY;
				break;
			case CP:
				unit[i].defend = DEFEND_CP;
				break;
			case GEV:
				unit[i].defend = DEFEND_GEV;
				break;
		}

	for(i=0;i<n_units;i++)
		if (unit[i].type==BUNKER)
			for(j=0;j<n_units;j++)
				if ((unit[i].l_hex==unit[j].l_hex)&&(unit[i].r_hex==unit[j].l_hex)&&(i!=j))
					unit[j].defend=6;

   disp_craters();
}
/*
 * getmove - retrieves one move, verifies that the board is correct, and
 *	returns whether or not this unit can move again.
 */
getmove(i, l_old, r_old, m_old)
int i;
char l_old, r_old, m_old ;
{

    char    nomove, bad_char;
    char    a, b, dir;
    char    olda, oldb ;

    nomove = TRUE;

    while(nomove) {
    
        a = unit[i].l_hex;
        b = unit[i].r_hex;

        movecur_hex(a, b);

        bad_char = FALSE;
    
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
    
            case SIT:
            case ' ':
		for (dir = 0; dir < n_units; dir++)
		    if (unit[dir].type != BUNKER)
		    	if (dir != i && unit[dir].status != DESTROYED &&
				unit[dir].l_hex == a && unit[dir].r_hex == b)
			    	bad_char = unit[dir].type != INFANTRY
		               	|| unit[i].type != INFANTRY
			       	|| infantry_on(a, b) > 3 ;
			if (bad_char) break ;
	
                	unit[i].moves_left = 0;
			return FALSE ;

	    case PASS:
		unit[i].l_hex = l_old ;
		unit[i].r_hex = r_old ;
		unit[i].moves_left = m_old ;
		update_hex(a, b) ;	
		disp_unit(i) ;
	    	return TRUE ;

            default:
                bad_char = TRUE;
                break;
    
        }

        /* Rule 5.02 */

        if(bad_char ||
	    off_map(a, b) || 
            (blocked(a, b)&&(unit[i].type !=INFANTRY)) ||
	    ( ((dir = occupied(a, b)) && unit[i].moves_left == 1) &&
	      (unit[i].type != INFANTRY || unit[--dir].type !=INFANTRY ||
	       unit[i].attack + infantry_on(a, b) > 3)))
	
        {
	    putchar(BEEP) ;
            bad_char = FALSE;
    
        }

        else {
            /* move the thing */

	    if (unit[i].type==INFANTRY)
		unit[i].defend=1;
	    if (blocked(a,b))
		unit[i].defend=3;
	    olda = unit[i].l_hex;
	    oldb = unit[i].r_hex;
	    unit[i].l_hex = a;
            unit[i].r_hex = b;
	    update_hex(olda, oldb);
	    disp_unit(i) ;

            nomove = FALSE;
            unit[i].moves_left -= 1;

            def_ram(i);

        }

    }
    return FALSE  ;
}
