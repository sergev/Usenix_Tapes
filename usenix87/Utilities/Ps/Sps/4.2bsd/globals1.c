# include       "sps.h"
# include       "flags.h"

/* Read/Write Variables global to the code of sps */

struct info                     Info ;          /* Information structure */

struct flags                    Flg ;           /* Flag options */

struct summary                  Summary ;       /* Summary of processes */

union  userstate                User ;          /* Upage of one process */

int                             Flmem, Flkmem, Flswap ; /* File descriptors */

unsigned                        Termwidth ;     /* Width of output device */

short                           Lastpgrp ;      /* Last process pgrp printed */

short                           Lastuid ;       /* Last process uid printed */
