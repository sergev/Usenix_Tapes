/*
**
**  Resolve.c
**
**  Routes then resolves addresses into UUCP or LOCAL.
**
*/
#ifndef lint
static char 	*sccsid="@(#)resolve.c	1.9   (UUCP-Project/CS)   6/27/86";
#endif

#include	<ctype.h>
#include	<stdio.h>
#include	"defs.h"

extern int exitstat;		/* set if address doesn't resolve 	*/
extern enum ehandle handle;	/* what mail we can handle		*/
extern enum edebug debug;	/* verbose and debug modes		*/
extern enum erouting routing;	/* when to route addresses		*/
extern char hostdomain[];	/* for qualifying abbreviated addr's	*/
extern char *pathfile;		/* location of path database		*/


/*
**
**  rsvp(): how to resolve addresses.
**
**  After parsing an address into <form>, the resolved form will be
**  rsvp( form ).  If == ROUTE, we route the parsed address and parse again.
**
*/

# define rsvp(a) table[(int)a][(int)handle]

enum eform table[5][3] = {
/*	all		uucponly	none */
{	ERROR, 		ERROR, 		ERROR }, 	/* error */
{	LOCAL, 		LOCAL,	 	LOCAL }, 	/* local */
{	ROUTE, 		LOCAL, 		LOCAL }, 	/* domain */
{	UUCP, 		UUCP, 		LOCAL }, 	/* uucp */
{	ERROR, 		ERROR, 		ERROR }};	/* route */


/*
**  NOTE: in this module <domainv> replaces <hostv>. <domainv> contains 
**  the domain part of each address, though by the time it leaves here it 
**  can only be a host name.
*/


/*
**
**  resolve(): resolve addresses to <host, user, form>.
**
**  This is a gnarly piece of code, but it does it all.  Each section 
**  is documented.
**
*/

enum eform resolve( address, domain, user )
char *address;				/* the input address 	*/
char *domain;				/* the returned domain 	*/
char *user;				/* the returned user 	*/
{
	enum eform form;		/* the returned form	*/ 
	enum eform parse();		/* to crack addresses	*/
	int parts;			/* to ssplit addresses	*/
	char *partv[MAXPATH];		/* "  "      "		*/
	char temp[SMLBUF];		/* "  "      "		*/
	int i;
		

/*
**  If we set REROUTE and are prepared to deliver UUCP mail, we split the 
**  address apart at !'s and try to resolve successively larger righthand 
**  substrings until we succeed.  Regularly, we just resolve the whole thing 
**  once.
*/
	if ( routing == REROUTE && rsvp( UUCP ) == UUCP )
		parts = ssplit( address, '!', partv );
	else
		parts = 1, partv[0] = address;
/*
**  This for( i ) loop selects successively larger righthand substrings 
**  for BULLYing, see above. 
*/
	for( i = parts - 1; i >= 0; i-- )
	{
/*
**  Parse the address.  If we are BULLYing and our substring parses to 
**  the LOCAL address, we skip to the next larger.
*/
		(void) strcpy( temp, partv[i] );
		form = parse( temp, domain, user );
	DEBUG("parse address '%s' = %s @ %s (%d)\n",temp,user,domain,form);
		if ( i && form==LOCAL )
			continue;
/*
**  Routing is next step, so we break out if we don't have a UUCP form (if 
**  we are set to route ALWAYS or REROUTE) or a ROUTE form.
*/
		if ( rsvp( form ) != ROUTE && 
		    ( rsvp( form ) != UUCP || routing == JUSTDOMAIN ) )
			break;
/*
**  Apply router.  If BULLYing and routing failed, try next larger substring.
*/
		if ( route( form, domain, user, temp ) )
			continue;
/*
**  After routing, reparse and resolve.
*/
		form = parse( temp, domain, user );
	DEBUG("parse route '%s' = %s @ %s (%d)\n",temp,user,domain,form);
		break;
	}
/*
**  For LOCAL mail in non-local format, we rewrite the full address into 
**  <user> and leave <domain> blank.
*/
	if ( rsvp( form ) == LOCAL && form != LOCAL )
	{
		build( domain, user, form, temp );
		(void) strcpy( user, temp );
		(void) strcpy( domain, "" );
		form = LOCAL;
	}
/*
**  If we were supposed to route and address but failed (form == ERROR), 
**  or after routing once we are left with an address that still needs to
**  be routed (rsvp( form ) == ROUTE), complain.  It is possible that we 
**  may want to resolve this address more than once (if the routing tables
**  return a domain-style address), but most likely this address partially 
**  resolved to this host.
*/
	if ( form == ERROR || rsvp( form ) == ROUTE )
	{
		exitstat = EX_NOHOST;
		(void) printf( "%s...couldn't resolve %s.\n", address, domain );
		form = ERROR;
	}
	ADVISE("resolve '%s' = %s @ %s (%d)\n",address,user,domain,form);
	return ( form );
}


/*
**
** parse(): parse <address> into <domain, user, form>.
**
** 	input		form
**	-----		----
**	user		LOCAL
**	domain!user	DOMAIN
**	user@domain	DOMAIN
**	@domain,address	LOCAL	(just for sendmail)
**	host!address	UUCP
**
*/

enum eform parse( address, domain, user )
char *address;		/* input address 	*/
char *domain;		/* output domain 	*/
char *user;		/* output user 		*/
{
	int parts;
	char *partv[MAXPATH];				/* to crack address */

/*
**  If this is route address form @hosta,@hostb:user@hostd, break for
**  LOCAL since only sendmail would want to eat it.
*/
	if ( *address == '@' )
		goto local;
/*
**  Try splitting at !. If it works, see if the piece before the ! has
**  a . in it (domain!user, form DOMAIN) or not (host!user, form UUCP).
*/
	if ( ssplit( address, '!', partv ) > 1 )
	{
		(void) strcpy( user, partv[1] );
		(void) strncpy( domain, partv[0], partv[1]-partv[0]-1 );
		domain[partv[1]-partv[0]-1] = '\0';
		if( ( parts = ssplit( domain, '.', partv ) ) < 2 )
			return( UUCP );
		if( partv[parts-1][0] == '\0' )	
			partv[parts-1][-1] = '\0'; /* strip trailing . */
		return ( DOMAIN );
	}
/*
**  Try splitting at @.  If it work, this is user@domain, form DOMAIN.
**  Prefer the righthand @ in a@b@c.
*/
	if ( ( parts = ssplit( address, '@', partv ) ) >= 2 )
	{
		(void) strcpy( domain, partv[parts-1] );
		(void) strncpy( user, partv[0], partv[parts-1]-partv[0]-1 );
		user[partv[parts-1]-partv[0]-1] = '\0';
		return ( DOMAIN );
	} 
/* 
**  Done trying.  This must be just a user name, form LOCAL.
*/
local:
	(void) strcpy( user, address );
	(void) strcpy( domain, "" );
	return( LOCAL );				/* user */
}


/*
**
**  route(): route domain, plug in user.
**
**  Less complicated than it looks.  Each section is documented.
**
*/

route( form, domain, user, result )
enum eform form;		/* domain is UUCP host? */
char *domain;			/* domain or host name 	*/
char *user;			/* user name 		*/
char *result;			/* output route 	*/
{
	int	domains, step;			/* to split domain	*/
	char	*domainv[MAXDOMS];		/* "  "     "		*/
	char	temp[SMLBUF], path[SMLBUF];

/*
**  Fully qualify the domain, and then strip the last (top level domain) 
**  component off, so that we look it up separately.
*/
	(void) strcpy( temp, ".");
	(void) strcat( temp, domain );

	domains = ssplit( temp+1, '.', domainv );
	/* If the domain ends in .UUCP, trim that off. */
	if ( domains && isuucp(domainv[domains-1]))
		domainv[domains-1][-1] = '\0';
/*
**  Try to get the path for successive components of the domain.  
**  Example for osgd.cb.att.uucp:
**	osgd.cb.att
**	cb.att
**	att
**	uucp ( remember stripping top level? )
**  Returns with error if we find no path.
*/
	step = 0;
	while ( step<domains && getpath( domainv[step]-1, path )  /* w/dot */
			     && getpath( domainv[step]  , path ) )/* no dot */
		step++;
	if ( step == domains )
	{
		DEBUG( "getpath '%s' failed\n", domain );
		return( EX_NOHOST );
	}
	DEBUG("getpath '%s' (%s) = %s\n",domain,domainv[step],path);

/*
**  If we matched on the entire domain name, this address is fully resolved, 
**  and we plug <user> into it.  If we matched on only part of the domain 
**  name, we plug <domain>!<user> in.  
*/
	build( domain, user, step ? UUCP:LOCAL, temp+1 );
	(void) sprintf( result, path, temp+1 );
	return( EX_OK );
}

/*
 * Return 1 iff the string is "UUCP" (ignore case).
 */
isuucp(str)
char *str;
{
	if (lower(*str) != 'u') return 0;
	++str;
	if (lower(*str) != 'u') return 0;
	++str;
	if (lower(*str) != 'c') return 0;
	++str;
	if (lower(*str) != 'p') return 0;
	++str;
	if (*str != '\0') return 0;
	return 1;
}

/*
**
** qualifydomain(): turn domain into full domain name.
**
** Best explained by examples, if hostdomain = a.b.c.UUCP
**	host.domain.UUCP -> host.domain.UUCP
**	host.b -> host.b.c.UUCP
**	host.x -> host.x.UUCP
**
*/

/* qualifydomain():
 * Taken out 3/21/86 by MRH - if hostdomain is, say. a.b.c.COM,
 * and domain is x.UUCP, it turns it into x.UUCP.COM and then
 * barfs on it.  I don't see a way to handle PQN's this easily.
 */

build( domain, user, form, result )
char *domain;
char *user;
enum eform form;
char *result;
{
	switch( form )
	{
	case LOCAL:
		(void) sprintf( result, "%s", user ); 
		break;
	case UUCP:
		(void) sprintf( result, "%s!%s", domain, user );
		break;
	case DOMAIN:
		(void) sprintf( result, "%s@%s", user, domain );
		break;
	}
}



/*
**
** getpath(): look up key in ascii sorted path database.
**
** Binary searches a la look(1).  Sort -f to fold cases.
**
*/

getpath( key, path )
char *key;		/* what we are looking for */
char *path;		/* where the results go */
{
	long pos, middle, hi, lo;
	static long pathlength = 0;
	register char *s;
	int c;
	static FILE *file;
	int flag;

	if( !pathlength )	/* open file on first use */
	{
		if( ( file=fopen( pathfile, "r" ) ) == NULL )
		{
			(void) printf( "can't access %s.\n", pathfile );
			pathlength = -1;
		} else {
			(void) fseek( file, 0L, 2 );		/* find length */
			pathlength = ftell( file );
		}
	}
	if( pathlength == -1 )
		return( EX_OSFILE );

	lo = 0;
	hi = pathlength;
	(void) strcpy( path, key );
	(void) strcat( path, "\t" );
/*
** "Binary search routines are never written right the first time around."
** - Robert G. Sheldon.
*/
	for( ;; ) 
	{
		pos = middle = ( hi+lo+1 )/2;
		(void) fseek( file, pos, 0 );	/* find midpoint */
		if ( pos )		/* to beginning of next line */
			while( ( c=getc( file ) ) != EOF && c != '\n' );
		for( flag = 0, s = path; !flag; s++ ) /* match??? */
		{
			if ( *s == '\0' )
				goto solved;
			c = getc( file );
			flag = lower( c ) - lower( *s );
		} 
		if ( lo>=middle )		/* failure? */
			return( EX_NOHOST );
		if ( c != EOF && flag < 0 )	/* close window */
			lo = middle;
		else 
			hi = middle - 1;
	} 
/* 
** Now just copy the result.
*/
solved:
	while( ( c=getc( file ) ) != EOF && c != '\n' )
		*path++ = c;
	*path = '\0';
	return ( EX_OK );
}
