static char *RCSid = "$Header: arccode.c,v 1.1 86/06/26 14:59:53 turner Exp $";

/*
 * $Log:	arccode.c,v $
 * Hack-attack 1.3  86/12/20  01:23:45  wilhite@usceast.uucp
 * 	Bludgeoned into submission for VAX 11/780 BSD4.2
 *	(ugly code, but fewer core dumps)
 *
 * Revision 1.1  86/06/26  14:59:53  turner
 * initial version
 * 
 * 
 */

/*  ARC - Archive utility - ARCCODE

$define(tag,$$segment(@1,$$index(@1,=)+1))#
$define(version,Version $tag(
TED_VERSION DB =1.02), created on $tag(
TED_DATE DB =01/20/86) at $tag(
TED_TIME DB =13:33:35))#
$undefine(tag)#
    $version

(C) COPYRIGHT 1985 by System Enhancement Associates; ALL RIGHTS RESERVED

    By:  Thom Henderson

    Description:
         This file contains the routines used to encrypt and decrypt
         data in an archive.  The encryption method is nothing fancy,
         being just a routine XOR, but it is used on the packed data,
         and uses a variable length key.  The end result is something
         that is in theory crackable, but I'd hate to try it.  It should
         be more than sufficient for casual use.

    Language:
         Computer Innovations Optimizing C86
*/
#include <stdio.h>
#include "arc.h"

static char *p;                        /* password pointer */

INT setcode()                              /* get set for encoding/decoding */
{
    p = password;                      /* reset password pointer */
}

INT code(c)                            /* encode some character */
INT c;                                 /* character to encode */
{
    if(p)                              /* if password is in use */
    {    if(!*p)                       /* if we reached the end */
              p = password;            /* then wrap back to the start */
         return c^*p++;                /* very simple here */
    }
    else return c;                     /* else no encryption */
}
