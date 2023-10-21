/*
 * %W% (mrdch&amnnon) %E%
 */

/*
 * this file defines the score information structure.
 */

struct  score                           /* score log structure */
{
        char    win[20] ;               /* winner name          */
        char    los[20] ;               /* looser name          */
        int     years ;                 /* game years played    */
        time_t  played_at ;             /* date played at       */
} ;
