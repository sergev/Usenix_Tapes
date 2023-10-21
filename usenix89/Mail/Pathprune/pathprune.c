#ifndef lint
static char sccsID[] = "@(#)pathprune.c	1.1 Delta: 14:58:26 3/6/87";
#endif
/*
 *  Prune down a pathalias file by throwing out all unnecessary entries.
 *
 *  Usage
 *	pathprune [options] [ infile [outfile] ]
 *
 *	-u	prune .uucp(implied) entries
 *	-t	prune top level (via .other) entries
 *	-d dom	sacred domain, do not prune domain gateways
 *	-h dom	sacred hosts, do not prune hosts in this domain (or subdomains)
 *	-r dom	remove all hosts in this domain (and subdomains)
 *	-j	junk all hosts with bogus top-level domains
 *	-v	verbose, print statistics
 *
 *  Compilation
 *	cc -O -o pathprune pathprune.c			# for USG systems
 *	cc -DBSD -O -o pathprune pathprune.c getopt.o	# for BSD systems
 *
 *  Disclaimer
 *	Pathprune is in the public domain.  It may be used by any person or
 *	organization, in any way and for any purpose.  There is no warranty
 *	of any kind for this program.  What you see is what you get.
 *
 *  History
 *	1.1	Mar 6, 1987
 *		Written by Matt Costello
 */
# include <stdio.h>

# define VOID	(void)	/* define empty for non-voids */
# define UUCPNAME	"uucp"
# define OTHERNAME	"other"

# ifdef BSD

# include <strings.h>
# define strchr(s,c) index(s,c)
# define strrchr(s,c) rindex(s,c)
extern char	*gets();

char *
strtok( str, delims )
register char *str, *delims;
{
	static char *strnext;
	char *token;
	if (!str) str=strnext;
	while (*str && index(delims,*str))
		str++;	/* skip leading delimiters */
	if (!*str)
		return (NULL);
	token = str;
	while (*str && !index(delims,*str))
		str++;	/* skip token characters */
	if (*str)	strnext = str;
	else 		*strnext++ = '\0';
	return (token);
}

# else /* !BSD */

# include <string.h>
# include <memory.h>
extern void	exit();
extern void	perror();

# endif /* !BSD */

extern char	*malloc();
extern char	*calloc();
char		*stralloc();		/* FORWARD */

struct dnode {
	struct dnode *lnode;	/* pointer to littler node name */
	struct dnode *bnode;	/* pointer to bigger node name */
	struct dnode *children;	/* pointer to childrens node names */
	struct dnode *order;	/* pointer to next node in original order */
	char *fullname;		/* full name of this domain gateway */
	char *nodename;		/* last portion of the domain name */
	char *pathname;		/* path name to gateway for this node */
	char *line;		/* the full line from the paths file */
	int flags;
};
# define SACRED_DOMAIN	001
# define SACRED_HOST	002
# define REMOVE_HOST	004

struct dnode *rootnode = NULL;	/* root of the domain (node) tree */
struct dnode *look_node();	/* look for a node */
struct dnode *add_node();	/* look for a node */

struct dnode *firstnode = NULL;	/* to keep original order */
struct dnode *lastnode = NULL;	/* for performance */

char *		otherpath = "!!!!!!!";
char *		uucppath = "!!!!!!!";
int		flg_topdel;
int		flg_dontprune;
int		flg_junkbogus;
int		flg_uucpdel;
int		flg_removeuucp;
int		flg_verbose;

int	gw_in, gw_out;
int	dom_in, dom_out;
int	hst_in, hst_out;

main(argc,argv)
int argc;
char **argv;
{
	register struct dnode *p;
	register i;
	extern int optind;
	extern char *optarg;
	char linebuf[BUFSIZ];

	while ((i = getopt(argc,argv,"utjvd:h:r:")) != EOF) switch (i) {
		case 'u':	/* prune host.uucp entries where possible */
			flg_uucpdel++;
			break;
		case 't':	/* prune top level domain names */
			flg_topdel++;
			break;
		case 'j':	/* just hosts in bogus top-level domains */
			flg_junkbogus++;
			break;
		case 'v':
			flg_verbose++;
			break;
		case 'd':
			domflag( optarg, SACRED_DOMAIN );
			break;
		case 'h':
			domflag( optarg, SACRED_HOST );
			break;
		case 'r':
			domflag( optarg, REMOVE_HOST );
			break;

		default:
Usage:
VOID fputs("Usage: pathprune [-vutj] [-[dhr] dom] [ infile [outfile] ]\n",
				stderr );
			exit(2);
	}
	if ( (optind < argc) &&
	     (freopen( argv[optind++], "r", stdin ) == NULL) ) {
		VOID fputs("pathprune: cannot open ", stderr );
		perror( argv[--optind] );
		exit(1);
	}
	if ( (optind < argc) &&
	     (freopen( argv[optind++], "w", stdout ) == NULL) ) {
		VOID fputs("pathprune: cannot create ", stderr );
		perror( argv[--optind] );
		exit(1);
	}
	if (optind < argc) {
		goto Usage;
	}


	/*  Read in the file a line at a time.  There are two kinds of lines:
	 *  node lines and gateway lines.  While we are reading gateway lines
	 *  we just read them in and save them.  Once we get a non-gateway
	 *  line, we prune.
	 */

	/*  Read in all the domain gateways.
	 */
	for (;;) {
		if (gets( linebuf ) == NULL)
			fatal("unexpected EOF");
		if (linebuf[0] != '.')
			break;
		gw_in++;
		add_gateway( linebuf );
	}
	if (rootnode == NULL)
		fatal("no domain gateways");

	/*  Now prune out any unnecessary domain gateways.  They will be
	 *  unnecessary if they are a subdomain and the path passes
	 *  through the domain gateway.  We will leave alone any entries
	 *  for ourselves (%s) because we may use them to qualify.
	 */
	if ((p = look_node( &rootnode, UUCPNAME )) != NULL) {
		uucppath = p->pathname;
		p->flags |= SACRED_DOMAIN;
		if (p->flags & REMOVE_HOST)
			flg_removeuucp++;
	}
	if ((p = look_node( &rootnode, OTHERNAME )) != NULL) {
		if (flg_topdel)
			otherpath = p->pathname;
		if (p->flags & SACRED_DOMAIN)
			flg_dontprune++;
		p->flags |= SACRED_DOMAIN;
	}

	if (!flg_dontprune)
		prune_gateway( rootnode, otherpath );

	/*  Now print out the gateway entries in the original order.
	 */
	for ( p = firstnode; p != NULL; p = p->order ) {
		if (p->line != NULL) {
			gw_out++;
			VOID puts( p->line );
		}
	}

	/*  All the remaining lines in the file will be node names.
	 *  Look up the domain name to see what gateway we would use;
	 *  If the gateway is on the path to the node name then we do
	 *  not need this entry.
	 *  If they are simple node names we have to pass them right
	 *  through unless we have a usefull ".uucp" gateway along
	 *  the way.
	 */
	for (;;) {
		check_needed( linebuf );

		/*  Now get another line.
		 */
		if (gets( linebuf ) == NULL)
			break;
		if (linebuf[0] == '.')
			fatal("unexpected domain gateway");
	}

	if (flg_verbose) {
		VOID fprintf(stderr,"gateway\t%8d%8d\t%5.2f\n",
			gw_in, gw_out, (double)gw_out / (double)gw_in );
		VOID fprintf(stderr,"domain\t%8d%8d\t%5.2f\n",
			dom_in, dom_out, (double)dom_out / (double)dom_in );
		VOID fprintf(stderr,"host\t%8d%8d\t%5.2f\n",
			hst_in, hst_out, (double)hst_out / (double)hst_in );
	}

	exit(0);
}


domflag( name, flags )
register char *name;
int flags;
{
	register struct dnode **pp;
	register struct dnode *p;
	register char *s;

	pp = &rootnode;
	while ((s = strrchr( name, '.' )) != NULL) {
		*s++ = '\0';	/* to most significant part of domain name */
		p = add_node( pp, s );
		pp = &(p->children);
	}
	if (*name)
		p = add_node( pp, name );

	p->flags |= flags;
}

add_gateway( buf )
register char *buf;
{
	register struct dnode **pp;
	register struct dnode *p;
	register char *s;
	char * line;
	char * gw;
	char * path;

	/*  Save the original line because we will need it later.
	 */
	line = stralloc( buf );

	if ((gw = strtok( buf, " \t" )) == NULL)
		fatal("missing gateway on line");
	gw = stralloc( gw );
	if ((path = strtok( (char *)NULL, " \t" )) == NULL)
		fatal("missing path on gateway line");
	path = stralloc( path );

	/*  Go down through all the nodes looking for this one.
	 *  We won't actually find it but will create everything
	 *  along the way.
	 */
	pp = &rootnode;
	while ((s = strrchr( buf, '.' )) != NULL) {
		*s++ = '\0';	/* to most significant part of domain name */
		p = add_node( pp, s );
		pp = &(p->children);
	}

	/*  Make sure that this one hasn't already been used.
	 */
	if (p->line != NULL)
		fatal("duplicate gateway name found");
	p->line = line;
	p->fullname = gw;
	p->pathname = path;

	/*  Build the correct pointers so we can process these nodes
	 *  in the original order when it comes time to print them
	 *  back out.
	 */
	if (firstnode == NULL)
		firstnode = p;
	if (lastnode != NULL)
		lastnode->order = p;
	lastnode = p;
}


struct dnode *
look_node( pp, name )
register struct dnode **pp;
register char *name;
{
	register i;
	/*  Search down the node tree looking for a node with the
	 *  given name.
	 */
	while (*pp != NULL) {
		i = strcmp( name, (*pp)->nodename );
		if (i == 0)
			return (*pp);
		else if (i < 0)
			pp = &((*pp)->lnode);
		else
			pp = &((*pp)->bnode);
	}
	return (NULL);
}

struct dnode *
add_node( pp, name )
register struct dnode **pp;
register char *name;
{
	register i;
	/*  Search down the node tree looking for a node with the
	 *  given name.
	 */
	while (*pp != NULL) {
		i = strcmp( name, (*pp)->nodename );
		if (i == 0)
			return (*pp);
		else if (i < 0)
			pp = &((*pp)->lnode);
		else
			pp = &((*pp)->bnode);
	}

	/*  Couldn't find such a node, so create one.
	 */
	*pp = (struct dnode *) calloc( 1, sizeof(struct dnode) );
	if (*pp == NULL)
		fatal("cannot allocate node");
	(*pp)->nodename = stralloc( name );
	return (*pp);
}

prune_gateway( p, parpath )
register struct dnode *p;
register char *parpath;		/* parents path */
{
	register parlen = strlen( parpath ) - 2;
	while (p != NULL) {
		/*  Do the left side of the tree first.
		 */
		prune_gateway( p->lnode, parpath );

		/*  Check to see if we share a common path with our
		 *  parent.  If we do this is a redundant gateway and
		 *  can be eliminated.
		 */
		if ( (parlen <= 0) || (p->line == NULL) ||
		     (p->flags & SACRED_DOMAIN) ||
		     (strcmp( p->pathname, "%s" ) == 0) ) {
			/*  We leave these alone.
			 */
		} else if (strncmp( p->pathname, parpath, parlen ) == 0) {
			/*  This is not one we need to know.  Remove it
			 *  from the ranks of the living.
			 */
			p->line = NULL;
		}

		/*  Check our subdomains for uselessness.
		 */
		if (!(p->flags & SACRED_DOMAIN))
			prune_gateway( p->children,
				       (p->line) ? p->pathname : parpath );

		/*  Do the right side of the tree using tail
		 *  recursion.
		 */
		p = p->bnode;
	}
}


check_needed( buf )
register char *buf;
{
	register struct dnode **pp;
	register struct dnode *p;
	register char *s;
	char * path;
	char * gwpath;
	int	isdomain;
	char	tmpbuf[BUFSIZ];

	VOID strcpy( tmpbuf, buf );
	/*  Extract the host/domain name.  */
	if (strtok( tmpbuf, " \t" ) == NULL)
		fatal("blank line");
	if (strchr( tmpbuf, '.' ) != NULL) {
		isdomain = 1;
		dom_in++;
	} else {
		isdomain = 0;
		hst_in++;
	}
	/*  Extract the path.  */
	if ((path = strtok( (char *)NULL, " \t" )) == NULL)
		fatal("missing path");
	if (strcmp(path, "%s") == 0) {
		/*  This is for us.  We don't dare delete it.
		 */
		goto keep;
	}

	/*  Go down through all the nodes looking for this one.
	 *  We won't actually find it but will create everything
	 *  along the way.
	 */
	if (isdomain)
		gwpath = otherpath;	/*  It is in a domain.  */
	else if (flg_removeuucp)
		return;			/*  They all go away.  */
	else if (flg_uucpdel)
		gwpath = uucppath;	/*  .uucp may be sufficient  */
	else
		gwpath = NULL;		/*  Keep this one.  */
	pp = &rootnode;
	while ((s = strrchr( tmpbuf, '.' )) != NULL) {
		*s++ = '\0';	/* to most significant part of domain name */
		p = look_node( pp, s );
		if (p == NULL) {
			/*  Haven't seen this domain before.  If this is
			 *  a bugs top-level domain name we want to junk it.
			 */
			if (flg_junkbogus && pp == &rootnode)
				return;
			break;
		}
		if (p->flags & SACRED_HOST) {
			gwpath = NULL;
			break;
		}
		if (p->flags & REMOVE_HOST) {
			/*  Don't want any hosts in this domain.  */
			return;
		}
		if (p->line != NULL)
			gwpath = p->pathname;
		pp = &(p->children);
	}

	if ( (gwpath == NULL) || (strcmp( gwpath, "%s" ) == 0) ||
	     (strncmp( path, gwpath, strlen(gwpath)-2 ) != 0) ) {
keep:
		if (isdomain)	dom_out++;
		else		hst_out++;
		VOID puts( buf );
	}
}

char *
stralloc( str )
char *str;
{
	register char *s;
	if ((s = malloc( (unsigned)(strlen(str) + 1) )) == NULL)
		fatal("cannot allocate string");
	VOID strcpy( s, str );
	return (s);
}

fatal( reason )
char * reason;
{
	VOID fprintf(stderr,"pathprune: %s\n", reason );
	exit(1);
}
