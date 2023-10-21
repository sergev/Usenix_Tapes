#include "lar.h"
/* cheap hack */

long rlwtol(lwa)
register lword *lwa;
{
	asm	mov	dh, [lwa]		/* hibyte */
	asm	mov	dl, [lwa+1]		/* mbyte1 */
	asm	mov	ah, [lwa+2]		/* mbyte2 */
	asm	mov	al, [lwa+3]		/* lobyte */
}
