/*
 * $Header: openpty.h,v 1.1 87/04/15 21:29:48 josh Exp $
 *
 * $Log:	openpty.h,v $
 * Revision 1.1  87/04/15  21:29:48  josh
 * Initial revision
 * 
 */


/*
 *	This file defines the "ptydesc" structure which is returned
 *	by the routine "openpty".
 */

struct ptydesc {
	int		pt_pfd;		/* file descriptor of master side */
	int		pt_tfd;		/* file descriptor of slave side */
	char		*pt_pname;	/* master device name */
	char		*pt_tname;	/* slave device name */
};
