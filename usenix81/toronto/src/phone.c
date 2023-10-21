/*
 *	Look up a phone number in a 'phone' file
 *
 *	Does a grep on the phone files listed in PHONE of the environment.
 *	(environ list of form "PHONE=$HOME/phonebook:/usr/adm/phonebook...")
 *
 *	- If no environ list it looks in the default file /usr/adm/phonebook
 *
 *	- If a directory is stated than phone looks for a file within it
 *	of name phonebook.
 *
 *	- If a file is stated than phone looks in that file
 *
 *	Original from Univ of Waterloo.
 *	Heavily modified by :
 *		Angus Telfer - Oct 14, 1980
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	NO	0
#define	YES	1

FILE	*freopen();
char	*getenv();

char	def[] = "/usr/adm/phonebook";

char	*phonptr, *phfiles;

char	filename[50];

main( argc, argv )

register char **argv;
register int argc;

{
	register int i, k;
	int	moreflg;
	char	fbuf[256], tbuf[256], ubuf[256];
	struct	stat	typename;

	if(argc < 2) {
		printf("Name: ");
		gets(tbuf);
		argv[0] = tbuf;
	}

	else {
		argv++;
		argc--;
	}

	for(i=0; i<argc; i++)
		lowercase(argv[i]);

	if( (phfiles = getenv( "PHONE" )) == (char *)NULL )
		phfiles = def;

	moreflg = YES;

	while( moreflg ) {
		for( k=0; *phfiles != ':' && *phfiles != '\0'; k++ ) {
			filename[k] = *phfiles;
			phfiles++;
		}

		if( *phfiles == ':' && k == 0 ) {
			strcpy( filename, "../" );
			k = 3;
		}

		if( *phfiles == '\0' ) {
			if( k == 0 ) {
				fprintf(stderr,"No search path\n");
				break;
			}

			moreflg = NO;
		}

		filename[k] = '\0';
		phfiles++;
		stat( filename, &typename );

		if( typename.st_mode & S_IFREG ) ;
		else if( typename.st_mode & S_IFDIR ) {
			if( filename[k-1] != '/' )
				strcat( filename, "/" );

			strcat( filename, "phonebook" );
		}
		else {
			fprintf(stderr,"Invalid type of phone file - %s\n",filename);
			continue;
		}

		if (freopen( filename, "r", stdin) == NULL) {
			fprintf(stderr,"Cannot open phone file - %s\n",filename);
			continue;
		}

		while (gets(fbuf)) {
			strcpy(ubuf, fbuf);
			lowercase(fbuf);

			for(i=0; i<argc; i++)

				if(match(argv[i], fbuf)) {
					printf("%s\n", ubuf);
					break;
				}
		}
	}
}

match(as1, as2)

char *as1, *as2;

{
	register char *rs1, *rs2, *s2;

	for (s2 = as2; *s2;) {
		rs1 = as1;
		rs2 = s2++;

		while(*rs1 && *rs1 == *rs2++) ++rs1;

		if(*rs1 == '\0')
			return(1);
	}

	return(0);
}

lowercase(s)

register char *s;

{
	register c;

	while (c = *s++)
		if (isupper(c))
			*--s = tolower(c);
}
