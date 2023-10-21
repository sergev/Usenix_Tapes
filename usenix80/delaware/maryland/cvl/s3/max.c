#
/* Pointwise maximum of two picture files.  Les Kitchen, 2nd August 1979.  */
#include  "/mnt/class/les/defns.t"    /* Useful definitions */
#include  "/mnt/class/les/binop.t"    /* Driver for binary operators */

char binop( p, q )  char  p, q;
{  return( p > q ? p : q );  }
