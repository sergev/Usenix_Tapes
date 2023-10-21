#define D_NATSTR
#include "empdef.h"

double
tfact(cno, value)
int cno;
double value;
{
	double teffect, log();

	getnat(cno);
	teffect = log(nat.nat_t_level + 10.0) / 9.2;
	teffect = (teffect < 1.0 ? teffect : 1.0);
	return(teffect * value);
}
