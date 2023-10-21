/*
 * defs.h - defines for news-related programs.
 *
 * By convention, the version of the software you are running is taken
 * to be news_version below.
 */

#define news_version "B 2.11-B 1/17/84"
/* SCCS ID @(#)defs.dist	2.23	5/3/83 */

#include "newsdefs.h"	/* this gets most of the definitions */

#define FOLLOWUP INEWS
#define NEWSRC	".newsrc"
/* #define BERKNAME "ARPAVAX"	/* name of local host on Berknet	*/

#define LINES	512	/* maximum no. of lines in .newsrc		*/
#define NEGCHAR	'!'	/* newsgroup negation character			*/
#define DEADTIME 45	/* no. of seconds to wait on deadlock		*/
#define FMETA	'%'	/* file meta-character for c option		*/
#define SYSPATH	"PATH=/local/bin:/bin:/usr/bin"	/* default, secure, vanilla path */
#define LNCNT	16	/* Articles with > LNCNT lines go through pager */

/* Things you probably won't want to change */
#define	SNLN	8	/* max significant characters in sysname	*/
#define	PROTO	'A'	/* old protocol name				*/
#define NETCHRS	"!:.@^%"/* Punct. chars used for various networks	*/
#define	TRUE	1	/* boolean true					*/
#define	FALSE	0	/* boolean false				*/
#define NGFSIZ  5000	/* legal newsgroup file size			*/
#define	NGDELIM	','	/* delimit character in news group line		*/
