/* compare to integers of indeterminant length according to the condition
   given. return the appropriate status.  neither integer is affected.
*/

char *llltab[]	{
	"==",
	"!=",
	"<",
	">",
	"<=",
	">=",
	0
};

lcmpr(a, type, b, l)
int *a, *b, l;
char *type;
{
	int tmp1[50], tmp2[50], sc, rc, c, over, zero, sign;
	register int i;
	extern int lcary;

	zero = 0;
	clear(tmp1, sizeof tmp1);
	clear(tmp2, sizeof tmp2);
	copy(tmp1, a, l*2);
	if (b != 0)
		copy(tmp2, b, l*2);
	sc = ((tmp2[l - 1] & 0100000) == (tmp1[l - 1] & 0100000));
	_sub(tmp1, tmp2, l);
	c = lcary;
	rc = ((tmp2[l - 1] & 0100000) == (tmp1[l - 1] & 0100000));
	if (!sc && rc)
		over = 1;
	else
		over = 0;
	sign = tmp2[l - 1] < 0? 1: 0;
	for(i = 0; i < l; i++)
		zero =| tmp2[i];
	for(i = 0; llltab[i]; i++)	{
		if (scompr(llltab[i], type))	{
			switch(i)	{
			/* == */
			case 0:
				return(!zero);

			/* != */
			case 1:
				return(zero);

			/* < */
			case 2:
				return(!((sign != over) | !zero));

			/* > */
			case 3:
				return(!((sign == over) | !zero));

			/* <= */
			case 4:
				return((sign == over) | !zero);

			case 5:
			/* >= */
				return((sign != over) | !zero);
			}
		}
	}
	return(0);
}
