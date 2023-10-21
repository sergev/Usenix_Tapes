#
/*
 *	versatek plotter driver
 */
#include "/usr/sys/param.h"
#include "/usr/sys/user.h"
#define DONE	0200
#define IENB	0101
#define LVPRI	-20
struct {
	int	csr;
	int	dbr;
};
#define LVADDR	0172550
int	lv_lock	0;
lvopen(dev,flag)
{
	if(lv_lock==0 && LVADDR->csr > 0 && flag) {
		lv_lock++;
	} else {
		u.u_error=ENXIO;
	}
}
lvclose()
{
	lv_lock=0;
	LVADDR->csr=0;	/* interupts off */
}
lvwrite()
{
	register int c;
	extern lbolt;
	while((c=cpass()) >= 0) {
		while((LVADDR->csr&0100200) != 0200)
			if(LVADDR->csr < 0)
				sleep(&lbolt,LVPRI);
			else {
				LVADDR->csr =| IENB;
				sleep(&lv_lock,LVPRI);
			}
		LVADDR->dbr = ~c;
	}
}
lvintr()
{
	if(LVADDR->csr < 0)
		timeout(&lvintr,HZ*4);
	else {
		LVADDR->csr =& ~IENB;
		wakeup(&lv_lock);
	}
}
