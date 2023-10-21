/*
 * %W% (mrdch&amnnon) %E%
 */

/*
 * this file contains all the constants used by the game.
 */

# define HOME            "/usr/games/lib/galaxy/"
# define LOCAL           "/usr/games/lib/galaxy/local"
# define WIZFIL          "/usr/games/lib/galaxy/wizards"
# define GALSCOR         "/usr/games/lib/galaxy/galaxy.scor"
# define ONLINE          "/usr/games/lib/galaxy/online/"
# define PAGER           "/usr/games/lib/galaxy/pager"

# define ESPSIZ          8      /* max nesting of espionage      */
# define MAXSHIPS        7      /* how many types of ships       */
# define MAXPL          91      /* how many planets are there    */
# define PLKINDS         5      /* how many kinds of planets     */
# define MAXCHAN         2      /* maximum no. of chanels to open */

# define NCREW          64      /* no. of fighters in A-type HAWK */
# define VISITORS      128      /* no. of additional people      */
# define MSGSIZ        150      /* max chars in a single masseg */

# define CLASES  6              /* how many clases of people     */

# define FIGT    0              /* fighters                      */
# define CITI    1              /* citizens                      */
# define SCIE    2              /* scientists                    */
# define BUIL    3              /* builders                      */
# define MINE    4              /* miners                        */
# define SLAV    5              /* slaves                        */

# define ESPTYP          7      /* different types of espionage: */

# define ESPKIND         0      /* what kind of planet is it     */
# define ESPPOP          1      /* how many people are there     */
# define ESPKNOW         2      /* what is the knowledge level   */
# define ESPMTL          3      /* how much metal was digged out */
# define ESPSHIP         4      /* what are the forces there     */
# define ESPALM          5      /* how many ALM were installed   */
# define ESPMSL          6      /* how many missiles are there  */

# define BUILD_MONEY     0      /* MONEY given to build ships */
# define LEVEL           1      /* the LEVEL of ships ordered  build */
# define NSHIPS          2      /* the NO. of ships ordered build */

# define N_PSI          20      /* max possible messages on screen */

# define ALMCOST       100      /* how much costs 1 ALM */
# define REMOVE_COST    50      /* to remove one if yours */
# define ALM_KILL_COST 200      /* to remove from the enemy's planet */

# define YEARLENGTH    180      /* a turn is 3 min. */
# define PERC_TRADE      5      /* the minimum % of profit in trade */
# define PERC_POPUL      10     /* the % of natural growth */
# define FEED_RATIO    100      /* no. people fed by 1 teller a year */
# define MINING_FACTOR 100      /* years_manpower 1A metals */
# define KNOW_FACTOR  2000      /* years_manpower to raise 1 level */
# define SHIP_COST     100      /* basic cost factor to build ship */
# define MISSILE_COST  400      /* to put a A-type missile */
# define REDUCE_RATE    10      /* detecting lost effect % in a year */
# define FADE_RATE      25      /* undetection paint fading rate */

# define MIN_ESP       100      /* the minimum to report ANYTHING */
# define KIND_ESP    MIN_ESP    /* to find out the planet's type */
# define POPUL_ESP  5*MIN_ESP   /* for it's population */
# define KNOW_ESP   3*MIN_ESP   /* the level of knowledge there */
# define METAL_ESP  2*MIN_ESP   /* how much metal has he */
# define FORCE_ESP 10*MIN_ESP   /* what are his forces there */
# define ALM_ESP    2*MIN_ESP   /* how many alms he's got */
# define MSL_ESP    8*MIN_ESP   /* how many missiles he set there */
