
/*
 *
 * Search! Greg Ordy
 *
 * This file contains global declarations for the linked
 * lists used to hold the items on a players screen
 *
*/
int     wholist;
struct plist   *avl;
int     numbnode;

struct plist
{
    char    zx;
    char    zy;
    char    zchar;
    char    zflg;
    struct plist   *zpnt;
};
