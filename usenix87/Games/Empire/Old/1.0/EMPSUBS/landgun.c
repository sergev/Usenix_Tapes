#define	D_MCHRSTR
#define	D_SHIPTYP
#include	"empdef.h"

double
landgun(efficien)
int	efficien;
{
	return((float)(rand()%10) * efficien * .01 + 20.);
}

double
seagun(stype, efficien, numguns)
int	stype, efficien, numguns;
{
	double	d, gun;

	if( stype == S_TEN ) {
		gun = d = .9 - efficien * .0002;
	} else {
		gun = d = .9 - mchr[stype].m_gun * efficien * .0002;
	}
	while( --numguns > 0 ) gun *= d;
	return(100. - 100. * gun);
}

landdef(designat)
int	designat;
{
	return( (designat == 22) ? 320 : 160);
}

seadef(stype)
int	stype;
{

	return(mchr[stype].m_prdct);
}

shelldam(gun, def)
double	gun;
int	def;
{
	register	i;

	i = (100. * gun) / (def + gun + 1.0);
	return(i);
}
