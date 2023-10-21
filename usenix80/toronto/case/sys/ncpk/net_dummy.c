#

/*
 * Dummy network kernel
 */

#include "../param.h"
#include "../user.h"

netopen()
{
	u.u_error = ENCPKNI;	/* no ncp kernel in system */
}

netclose()
{
	u.u_error = ENCPKNI;	/* no ncp kernel in system */
}

netread()
{
	u.u_error = ENCPKNI;	/* no ncp kernel in system */
}

netwrite()
{
	u.u_error = ENCPKNI;	/* no ncp kernel in system */
}

netstat()
{
	u.u_error = ENCPKNI;	/* no ncp kernel in system */
}
