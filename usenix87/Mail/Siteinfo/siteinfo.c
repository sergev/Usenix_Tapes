/*
 * siteinfo:
 *	store/retrieve uucp map info in dbm format
 *
From: Piet Beertema <seismo!mcvax!piet>
 *
 * siteinfo uses a dbm file with pointers into the real map files,
 * hence initialization MUST be done any time a map file has changed.
 * default dbm file name is "netdir"; alternate name may be specified
 *
 * usage:
 *   initialize:  siteinfo -i [-d dbmfile] mapfile1 mapfile2 ....
 *   retrieve  :  siteinfo [-d dbmfile] sitename1 sitename2 ....
 *
 */

#include <stdio.h>
#ifdef NULL
#undef NULL
#endif NULL
#include <dbm.h>

#define FNAME	"netdir"

#define SAME	0
#define NLEN	7	/* max sitename length for uucp */

typedef struct {
	long	pos;
	char	name[30];
} position;

datum fileinfo;
datum sitename;
char line[BUFSIZ];
FILE *fp;

main(argc, argv)
int argc;
char *argv[];
{
	register char c, *cp, *tp, first;
	char newdbm = 0;
	char fname[30];
	int namelen, ctr = 0;
	position *p;
	extern datum fetch();

	strcpy(fname, FNAME);
	argc--;
	while (argv++, argc--) {
		if (**argv == '-') {
			switch ((*argv)[1]) {
			case 'd':
				strcpy(fname, *++argv);
				argc--;
				break;
			case 'i': 
				newdbm = 1;
				break;
			default: 
				fprintf(stderr, "Unknown option %s\n", *argv);
				return 1;
			}
		}
		else
			break;
	}
	dbminit(fname);
	if (newdbm) {
		do {
			getfile(*argv);
		} while (argv++, argc--);
		return 0;
	}
	sitename.dptr = *argv;
	namelen = strlen(*argv);
	sitename..dsize = namelen > 7 ? 7 : namelen;
	fileinfo = fetch(sitename);
	printf("\n\nUUCP map data for site %7.7s:  \n\n", *argv);
	if ((p = (position *)fileinfo.dptr) == (position *)NULL) {
		printf("Not available\n\n");
		return 1;
	}
	else {
		if ((fp = fopen(p->name, "r")) == (FILE *)NULL) {
			fprintf(stderr, "Can't open %s\n", p->name);
			printf("Not available\n\n");
			return 1;
		}
		fseek(fp, p->pos, 0);
		first = 1;
		while (fgets(line, BUFSIZ, fp)) {
			if (*(cp = line) != '#') {
				printf(line);
				continue;
			}
			if ((c = *++cp) == 'N' && ctr++)
				break;
			for (cp++; *cp == ' ' || *cp == '\t'; cp++);
			tp = "";
			switch (c) {
			case 'N':
				tp = "System name";
				break;
			case 'S':
				tp = "System type";
				break;
			case 'O':
				tp = "Organization";
				break;
			case 'C':
				tp = "Contact person";
				break;
			case 'E':
				tp = "Electronic Address";
				break;
			case 'T':
				tp = "Telephone";
				break;
			case 'P':
				tp = "Postal Address";
				break;
			case 'L':
				tp = "Longitude/Latitude";
				break;
			case 'R':
				tp = "Remarks";
				break;
			case 'U':
				tp = "Usenet (news) links";
				break;
			case 'W':
				tp = "Last editor & date";
				break;
			default:
				if (first) {
					printf("\nConnect list etc.  :\n\n");
					first = 0;
				}
				continue;
			}
			printf("%-19.19s:	%s", tp, cp);
		}
		printf("\n\n");
		fclose(fp);
	}
}

getfile(filename)
char *filename;
{
	register char *cp;
	register int namelen;
	long pos = 0;
	position pp;

	if ((fp = fopen(filename, "r")) == (FILE *)NULL) {
		fprintf(stderr, "Can't open %s\n", filename);
		return;
	}
	fileinfo.dptr = (char *)&pp;
	fileinfo.dsize = sizeof pp.pos + strlen(filename) + 1;
	strcpy(pp.name, filename);
	while (fgets(line, BUFSIZ, fp)) {
		if (strncmp(line, "#N", 2) == SAME) {
			cp = line + 2;
			while (1) {
				while (*cp == ' ' || *cp == '\t' || *cp == ',')
					cp++;
				if (*cp == '\n')
					break;
				sitename.dptr = cp++;
				while (*cp != '\n' && *cp != ',' &&
				       *cp != ' ' && *cp != '\t')
					cp++;
				namelen = cp - sitename.dptr;
				sitename.dsize = namelen > NLEN ? NLEN : namelen;
				pp.pos = pos;
				store(sitename, fileinfo);
			}
		}
		pos += strlen(line);
	}
	fclose(fp);
}
