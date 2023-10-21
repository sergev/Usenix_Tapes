/*
**
**  Resolve.c
**
**  Routes then resolves addresses into UUCP or LOCAL.
**
*/
#ifndef lint
static char 	*sccsid="@(#)$Header: resolve.c,v 4.0 86/11/17 16:02:28 sob Exp $";
#endif

#include	<ctype.h>
#include	<stdio.h>
#include	"uuconf.h"



/*
**
**  rsvp(): how to resolve addresses.
**
**  After parsing an address into <form>, the resolved form will be
**  rsvp( form ).  If == ROUTE, we route the parsed address and parse again.
**
*/

# define rsvp(a) table[(int)a-1][handle-1]

int table[5][3] = {
/*	all		uucponly	none */
{ 	ERROR, 		ERROR, 		ERROR }, 	/* error */
{ 	LOCAL, 		LOCAL,	 	LOCAL }, 	/* local */
{ 	ROUTE, 		LOCAL, 		LOCAL }, 	/* domain */
{ 	UUCP, 		UUCP, 		LOCAL }, 	/* uucp */
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

resolve( address, domain, user )
char *address;				/* the input address 	*/
char *domain;				/* the returned domain 	*/
char *user;				/* the returned user 	*/
{
	int form;		/* the returned form	*/ 
	int parts;			/* to ssplit addresses	*/
	char *partv[PATHSIZ];		/* "  "      "		*/
 	char temp[PATHSIZ];		/* "  "      "		*/
	int i;
		

/*
**  If we set REROUTE and are prepared to deliver UUCP mail, we split the 
**  address apart at !'s and try to resolve successively larger righthand 
**  substrings until we succeed.  Regularly, we just resolve the whole thing 
**  once.
*/
	if ( rsvp( UUCP ) == UUCP )
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
#ifdef DEBUG
	if (Debug>2) 
	printf("resolve: parse address '%s' = %s @ %s (%d)\n",temp,user,domain,form);
#endif
		if ( i && form == LOCAL )
			continue;
/*
**  Routing is next step, so we break out if we don't have a UUCP form (if 
**  we are set to route ALWAYS or REROUTE) or a ROUTE form.
*/
		if ( rsvp( form ) != ROUTE && 
		    ( rsvp( form ) != UUCP ) )
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
#ifdef DEBUG
if (Debug>2)printf("resolve: parse route '%s' = %s @ %s (%d)\n",temp,user,domain,form);
#endif
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
#ifdef DEBUG
 	if (Debug) printf( "%s...couldn't resolve %s.\n", address, domain );
#endif
		form = ERROR;
	}
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

parse( address, domain, user )
char *address;		/* input address 	*/
char *domain;		/* output domain 	*/
char *user;		/* output user 		*/
{
	int parts;
	char *partv[PATHSIZ];				/* to crack address */

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
int form;			/* domain is UUCP host? */
char *domain;			/* domain or host name 	*/
char *user;			/* user name 		*/
char *result;			/* output route 	*/
{
	int	domains, step;			/* to split domain	*/
	char	*domainv[MAXDOMS];		/* "  "     "		*/
	char	temp[PATHSIZ], path[PATHSIZ];

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
	while ( step<domains && getpath( domainv[step]-1, path,paths )
							 /* w/dot */
			     && getpath( domainv[step]  , path, paths) )
							/* no dot */
		step++;
	if ( step == domains )
	{
#ifdef DEBUG
	if(Debug>2) printf( "resolve: getpath '%s' failed\n", domain );
#endif
		exitstat = EX_NOHOST;
		return( exitstat );
	}
#ifdef DEBUG
	if(Debug>2)printf("resolve: getpath '%s' (%s) = %s\n",domain,domainv[step],path);
#endif
/*
**  If we matched on the entire domain name, this address is fully resolved, 
**  and we plug <user> into it.  If we matched on only part of the domain 
**  name, we plug <domain>!<user> in.  
*/
	build( domain, user, step ? UUCP:LOCAL, temp+1 );
	(void) sprintf( result, path, temp+1 );
	exitstat=EX_OK;
	return( exitstat );
}

/*
 * Return 1 iff the string is "UUCP" (ignore case).
 */
isuucp(str)
char *str;
{
	if (islower(*str) != 'u') return 0;
	++str;
	if (islower(*str) != 'u') return 0;
	++str;
	if (islower(*str) != 'c') return 0;
	++str;
	if (islower(*str) != 'p') return 0;
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
int form;
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
**  ssplit(): split a line into array pointers.
**
**  Each pointer wordv[i] points to the first character after the i'th 
**  occurence of c in buf.  Note that each wordv[i] includes wordv[i+1].
**
*/

ssplit( buf, c, ptr )
register char *buf;		/* line to split up 		*/
char c;				/* character to split on	*/
char **ptr;			/* the resultant vector		*/
{
        int count = 0;
        int wasword = 0;

        for( ; *buf; buf++ )
        {
		if ( !wasword )
			count++, *ptr++ = buf;
		wasword = ( c != *buf );
        }
	if ( !wasword )
		count++, *ptr++ = buf;
        *ptr = NULL;
        return( count );
}

