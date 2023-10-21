#define	D_FILES
#include	"empdef.h"

wethr(i, j, dhrs)
int	i, j, dhrs;
{
	register	t, k;
	int	dh, dl, wx4, wy4;
	long	tvec;
	double	hi, lo;
	double	pslsin();

	time(&tvec);
	t = tvec/60 + dhrs * 60;
	wx4 = w_xsize>>2;
	wy4 = w_ysize>>2;
	hi = (pslsin(t * 19) + pslsin(t * 17)) * 85. + 180.;
	lo = (pslsin(t * 24) + pslsin(t * 15)) * 85. - 180.;
	k = (pslsin(t * 29) + pslsin(t * 14)) * wx4;
	wxh = k - capx;
	k = (pslsin(t * 28) + pslsin(t * 9)) * wy4;
	wyh = k - capy;
	k = (pslsin(t * 23) + pslsin(t * 13)) * wx4;
	wxl = k - capx;
	k = (pslsin(t * 26) + pslsin(t * 11)) * wy4;
	wyl = k - capy;
	dh = idist(xwrap(wxh - i), ywrap(wyh - j)) + 1;
	dl = idist(xwrap(wxl - i), ywrap(wyl - j)) + 1;
	k = (hi / dh + lo / dl) + 728.;
	return(k);
}
