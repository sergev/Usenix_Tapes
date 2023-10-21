typedef struct {

    char    type;
    char    attack;
    char    range;
    char    defend;
    char    movement;
    char    range_to_ogre;
    char    fired;
    char    moves_left;
    char    status;
    char    l_hex;
    char    r_hex;

} UNIT;

typedef struct {

    char    missiles;
    char    launchers;
    char    main_bats;
    char    sec_bats;
    char    ap;
    char    treads;
    char    movement;
    char    moves_left;
    char    l_hex;
    char    r_hex;
    char    init_treads;
    char    where;

} OGRE;

/* unit types */

#define CP          'C'
#define BUNKER      '#'
#define HVYTANK     'T'
#define LGTTANK     'L'
#define MSLTANK     'M'
#define GEV         'V'
#define HOWITZER    'H'
#define INFANTRY    'I'
#define MHOW	    '@'

#define DEFEND_CP           1
#define DEFEND_HVYTANK 	    3
#define DEFEND_LGTTANK 	    2
#define DEFEND_MSLTANK 	    2
#define DEFEND_GEV     	    2
#define DEFEND_HOWITZER	    1
#define DEFEND_INFANTRY	    1
#define DEFEND_MHOW	    1

/* unit statuses */
#define OK          1
#define DISABLED    2
#define DESTROYED   3

/* directions */
#define RIGHT       'J'
#define UPRIGHT     'U'
#define DOWNRIGHT   'N'
#define LEFT        'G'
#define UPLEFT      'Y'
#define DOWNLEFT    'B'
#define SIT         'S'
#define PASS        'P'
#define REDRAW      '\014'
#define BEEP	    '\07'

#define TRUE        1
#define FALSE       0

#define N_UNITS     200

#define ATK_MISSILES        6
#define DEF_MISSILES        3
#define RANGE_MISSILES      5  

#define	ATK_LAUNCH          6
#define DEF_LAUNCH          3
#define RANGE_LAUNCH        5

#define ATK_MAIN            4
#define DEF_MAIN            4
#define RANGE_MAIN          3

#define ATK_SECONDARY       3
#define DEF_SECONDARY       3
#define RANGE_SECONDARY     2

#define ATK_AP              1
#define DEF_AP              1
#define RANGE_AP            1
