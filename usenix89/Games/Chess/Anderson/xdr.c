/*
 * specialized xdr stuff
 */

#include <rpc/rpc.h>
#include <strings.h>

#include "nchess.h"

int
XdrGameReq(xdrsp, grp)
    XDR * xdrsp;
    GameRequest * grp;
{
    unsigned int size;
    char * cp;

    if ( ! xdr_u_long(xdrsp, &grp->progNum))
	return(0);
    if ( ! xdr_int(xdrsp, &grp->color))
	return(0);
    if ( ! xdr_int(xdrsp, &grp->resumeGame))
	return(0);
    size = sizeof(grp->hostName);
    cp = grp->hostName;
    if ( ! xdr_bytes(xdrsp, &cp, &size, size))
	return(0);
    size = sizeof(grp->userName);
    cp = grp->userName;
    if ( ! xdr_bytes(xdrsp, &cp, &size, size))
	return(0);
    return(1);
}

int
XdrMove(xdrsp, movep)
    XDR * xdrsp;
    Move * movep;
{
    if ( ! xdr_int(xdrsp, &movep->x1))
	return(0);
    if ( ! xdr_int(xdrsp, &movep->y1))
	return(0);
    if ( ! xdr_int(xdrsp, &movep->x2))
	return(0);
    if ( ! xdr_int(xdrsp, &movep->y2))
	return(0);
    if ( ! xdr_int(xdrsp, &movep->newPieceType))
	return(0);
    return(1);
}

int
XdrSetup(xdrsp, setup)
    XDR * xdrsp;
    SetupChange * setup;
{
    if ( ! xdr_int(xdrsp, &setup->x))
	return(0);
    if ( ! xdr_int(xdrsp, &setup->y))
	return(0);
    if ( ! xdr_int(xdrsp, (int *) &setup->type))
	return(0);
    if ( ! xdr_int(xdrsp, &setup->color))
	return(0);
    return(1);
}

int
XdrString(xdrsp, cp)
    XDR * xdrsp;
    char * cp;
{
    char c[128];
    strncpy(c, cp, sizeof(c));
    c[127] = '\0';
    if ( ! xdr_string(xdrsp, &cp, sizeof(c)))
	return(0);
    return(1);
}
