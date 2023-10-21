#include "uutty.h"
/*
** Build name for a lockfile.
*/
lockname()
{	char *dp, *lp, *rp;

	D5("lockname(\"%s\")",device);
	devfld = dp = lastfield(device,'/');
	D4("devfld=\"%s\" device=\"%s\"",devfld,device);
	lp = lockfile;		/* Place to build lockfile name */
	rp = lockroot;		/* Place to build lockfile name */
	while (*rp) *lp++ = *rp++;	/* Copy root to lockname */
	while (*dp) *lp++ = *dp++;	/* Append the device name */
	*lp = 0;
	D4("lockname:lockfile=\"%s\"",lockfile);
}
