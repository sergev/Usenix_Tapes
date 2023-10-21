
/*
**  fixcpio.c -- fix troubled cpio archive by skipping trashed members.
**
**  Dave Brower, 12/13/86
**  {sun, amdahl, mtxinu}!rtech!daveb
**
**  Usage:  fixcpio [ infile [ outfile ] ]
**
**  Writes a cpio -c archive to outfile (or stdout) from the infile (stdin).
**  ("-" may be used as the stdin/stdout filename.)
**
**  Skips over junk members.  This is how to recover when you've lost
**  floppy 9 of a 30 disk backup.  Eliminates "Out of phase -- get help"
*/

# include <stdio.h>

/* size blocks to write */

# define BLKSIZ	512

/* Maximum reasonable pathname in a header record */

# define MAXPATH 128

typedef struct
{
    /* these are ints for scanf's benefit. */
    int		h_magic,
    		h_dev,
    		h_ino,
    		h_mode,
		h_uid,
		h_gid,
    		h_nlink,
    		h_rdev;
    long	h_longtime;
    int		h_namesize;
    long	h_longfile;
} CHARHDR;

typedef struct
{
    CHARHDR	h;
    char 	h_name[ MAXPATH ];
} CHARREC;

CHARREC CRec = { 0 };			/* Character header */
char Trailer[] = "TRAILER!!!";		/* Magic string */
char Tmpfile[] = "/tmp/fixcpioXXXXXX";	/* temp file template */
int Debug;				/* Debugging? */

void outerr();				/* error writing output file */
void tmperr();				/* error writing temp file */
void writeerr();			/* error writing file */
int fprintf();				/* libc defined, -1 on error */

/*
** main() -- fix a cpio archive with "Out of phase -- get help" problems.
*/
main(argc, argv)
int argc;
char **argv;
{
    register int last;			/* last char processed */
    register int this;			/* current chars */
    register int nmagic;		/* "07"s in magic "070707" seen */

    register FILE *ifp = stdin;		/* input stream */
    register FILE *ofp = stdout;	/* output stream */
    register FILE *tfp = NULL;		/* temp file */

    int done = 0;			/* all done flag */
    long nbytes = 0;			/* count of bytes written */

    char buf[ 512 ];			/* holds a trailer. */

    char *getenv();			/* libc defined */
    FILE *efopen();			/* fopen, fatal on error */
    FILE *getmember();			/* stash a member in a temp file */
    long putmember();			/* write temp file */

    /* Set "secret" debugging flag */
    Debug = getenv("FIXCPIO") != NULL;

    if( argc > 3 )
    {
    	fprintf(stderr, "Usage: fixcpio [ infile [ outfile ] ]\n");
	return( 1 );
    }

    if( --argc > 0 && strcmp( *++argv, "-" ) )
	ifp = efopen( *argv, "r" );

    if( --argc > 0 && strcmp( *++argv, "-" ) )
    	ofp = efopen( *argv, "w" );

    /*
    ** Process chars of input.  When you see a magic number, try
    ** to accumulate the archive member on a temp file.  Write out
    ** good members as they are validated, skipping trouble makers.
    */
    for ( nmagic = last = this = 0 ; !done ; last = this )
    {
	switch( this = getc( ifp ) )
	{
	case '0':

	    /* maybe a header, no special action */
	    break;

	case '7':

	    /* Maintain count of special "07" pairs */
	    nmagic = last == '0' ? nmagic + 1 : 0;

	    /* It's a magic number, try to process as a header */
	    if( nmagic == 3 )
	    {
		nmagic = 0;

		/* stashed entry is good, write it */
		if( tfp )
		    nbytes += putmember( tfp, ofp );

		/* stash this possible entry into tfp, get CRec */
		tfp = getmember( ifp );
	    }
	    break;

	case EOF:

	    done = 1;
	    /* Fall into... */

	default:

	    /* Any existing entry is garbage... */
	    nmagic = 0;
	    if( tfp )
	    {
		if( !strcmp( CRec.h_name, Trailer ) )
		    done = 1;
		else
		    fprintf(stderr, "Skipping bad member \"%s\"\n",
				CRec.h_name );
		(void)fclose( tfp );
		tfp = NULL;
	    }
	    break;
	} /* switch */
    } /* for */

    /* flush pending good member */
    if( tfp )
    {
        nbytes += putmember( tfp, ofp );
	tfp = NULL;
    }

    /* Write a trailer -- remember to terminate the name string! */
    (void)sprintf( buf,	"070707%06o%06o%06o%06o%06o%06o%06o%011o%06o%011o%s",
	0, 0, 0, 0, 0, 0, 0, 0, sizeof(Trailer) + 1, 0, Trailer );
    if( fprintf(ofp, "%s", buf) < 0  || putc( 0, ofp ) < 0 )
    	outerr();
    nbytes += strlen( buf ) + 1;

    /* round output to an even block */
    nbytes = BLKSIZ - (nbytes % BLKSIZ);
    while( nbytes-- )
        if( putc( 0, ofp ) < 0 )
	    outerr();

    if( fclose( ofp ) < 0 )
        outerr();
    return( 0 );
}

/*
** getmember() -- save an archive member to a temp file
**
** When positioned after the magic number in a cpio file on ifp,
** copy the member to a temp file, and return it's fp.  The temp file
** contains a complete member (including magic number) and is positioned
** for catting directly to the real output file.
**
** If there are problems getting the member, return NULL.
*/
FILE *
getmember( ifp )
register FILE *ifp;
{
    register int c;			/* character of the member name */
    register int nr;			/* number read or scanned */
    register FILE *ofp;			/* temp file */
    long len;				/* actual member length */

    char name[ sizeof(Tmpfile) + 1 ];	/* name of the temp file */

    /* number of chars to read for a -c header */
#   define NCREAD	( (8 * 6) + (2 * 11) )

    char buf[ NCREAD + 1 ];		/* raw header */

    char *mktemp();			/* libc, make temp file name */
    char *strcpy();			/* libc, copy string */
    long ncat();			/* cat file to a length */

    if( NCREAD != ( nr = fread( buf, 1, NCREAD, ifp ) ) )
    {
	fprintf(stderr, "Couldn't read header:  Wanted %d, got %d\n",
		NCREAD, nr);
	return (NULL);
    }

    if( Debug )
    {
	fprintf(stderr,
    "dev  |ino  |mode |uid  |gid  |nlink|rdev |longtime  |nsize|longfile\n" );
	fprintf(stderr, "%s\n", buf );

    }

    if( 10 != ( nr = sscanf( buf, "%6o%6o%6o%6o%6o%6o%6o%11o%6o%11o",
			&CRec.h.h_dev,  &CRec.h.h_ino,  &CRec.h.h_mode,
			&CRec.h.h_uid,  &CRec.h.h_gid,  &CRec.h.h_nlink,
			&CRec.h.h_rdev, &CRec.h.h_longtime,
			&CRec.h.h_namesize , &CRec.h.h_longfile ) ) )
    {
	fprintf(stderr, "Couldn't scan header:  Wanted 10, got %d\n", nr);
	return (NULL);
    }

    if( Debug )
    {
 	fprintf(stderr, "dev 0%o ino 0%o mode 0%o uid %d gid %d\n",
		CRec.h.h_dev, CRec.h.h_ino, CRec.h.h_mode,
		CRec.h.h_uid, CRec.h.h_gid );
 	fprintf(stderr,
		"nlink %d rdev 0%o longtime 0%o namesize %d longfile 0%o\n",
		CRec.h.h_nlink, CRec.h.h_rdev, CRec.h.h_longtime,
		CRec.h.h_namesize, CRec.h.h_longfile );
    }

    /* Ridiculous name size?  probably trashed entry */
    if( !CRec.h.h_namesize || CRec.h.h_namesize > sizeof( CRec.h_name ) )
    {
	fprintf(stderr, "Bad namesize %d\n", CRec.h.h_namesize );
        return (NULL);
    }

    /* Get the name */
    nr = 0;
    while( nr < CRec.h.h_namesize && ( c = getc( ifp ) ) != EOF )
    	CRec.h_name[ nr++ ] = c;

    if( c == EOF )
    {
	fprintf(stderr, "Unexpected EOF reading name in header\n");
        return (NULL);
    }

    if( Debug )
	fprintf(stderr, "name \"%s\"\n", CRec.h_name );

    /* create a new temp file, and mark it for delete on close */
    (void)strcpy( name, mktemp( Tmpfile ) );
    ofp = efopen( name, "w+" );
    (void)unlink( name );

    /* Write a header */
    fprintf( ofp, "070707%06o%06o%06o%06o%06o%06o%06o%011o%06o%011o",
    			CRec.h.h_dev,  CRec.h.h_ino,  CRec.h.h_mode,
			CRec.h.h_uid,  CRec.h.h_gid,  CRec.h.h_nlink,
			CRec.h.h_rdev,  CRec.h.h_longtime,
			CRec.h.h_namesize, CRec.h.h_longfile ) ;

    for( nr = 0; nr < CRec.h.h_namesize ; )
        putc( CRec.h_name[ nr++ ], ofp );

    /* now copy the file body */
    if( CRec.h.h_longfile != (len = ncat( CRec.h.h_longfile, ifp, ofp ) ) )
    {
	fprintf(stderr, "Bad member length:  Should be %ld, was %ld\n",
		CRec.h.h_longfile, len );
	(void)fclose( ofp );
	return( NULL );
    }

    if( fseek( ofp, 0L, 0 ) < 0L )
        tmperr();
    return( ofp );

}

/*
** putmember() -- Write member, close input and return bytes written
*/
long
putmember( ifp, ofp )
register FILE * ifp;
register FILE * ofp;
{
    register long n;
    long cat();

    fprintf(stderr, "%s\n", CRec.h_name );
    n = cat( ifp, ofp );
    (void)fclose( ifp );
    return ( n );
}



/*
** cat() -- copy one stream to another, returning n bytes copied
*/
long
cat( ifp, ofp )
register FILE *ifp;
register FILE *ofp;
{
    register int c;
    register int n;

    for( n = 0 ; ( c = getc( ifp ) ) != EOF ; n++ )
    	if( putc( c, ofp ) < 0 )
	    outerr();

    return ( n );
}

/*
** ncat() -- copy up to n bytes from one stream to another, return actual
*/
long
ncat( in, ifp, ofp )
register long in;
register FILE *ifp;
register FILE *ofp;
{
    register int c;
    register long on;

    for( on = 0; in-- && ( c = getc( ifp ) ) != EOF ; on++ )
        if( putc( c, ofp ) < 0 )
	    tmperr();

    return ( on );
}

/*
** efopen() -- fopen() that fatals on error
*/
FILE *
efopen( file, mode )
char *file;
char *mode;
{
    FILE * fp;

    if( NULL == (fp = fopen( file, mode ) ) )
    {
	fprintf(stderr, "Can't open \"%s\" mode \"%s\"\n", file, mode );
	perror("efopen");
	exit( 1 );
    }
    return( fp );
}

/*
** outerr() -- handle error writing output file
*/
void
outerr()
{
    writeerr( "output" );
}

/*
** tmperr() -- handle error writing temp file
*/
void
tmperr()
{
    writeerr( "temp" );
}

/*
** writeerr() -- handle write errors, gracelessly.
*/
void
writeerr( what )
char *what;
{
    fprintf(stderr, "\007Error writing %s file", what );
    perror("");

}

/* end of fixcpio.c */
