/*
 * opath.c - get an optimal uucp path from the path database, using an
 * RFC882-style address as input.  The '%' character is properly translated,
 * and gateway-substitution is done to get mail onto other networks.
 *
 * This program requires the DBM-style pathalias database.
 *
 * Eric Roskos, Perkin-Elmer Corp. SDC
 * Version 3.2 created 85/09/12 15:21:51
 */

static char *opathsccsid = "@(#)opath.c	3.2 (peora) 15:21:51 - 85/09/12";

#include <stdio.h>
#include <whoami.h>

/**
 ** User-configurable parameters
 **/

/* locations of files */

static char *archive = "/usr/lib/uucp/alpath";  /* pathalias database   */
static char *domfile = "/usr/lib/uucp/domains"; /* gateway/domain table */
static char *logfile = "/usr/adm/opath.log";    /* activity log */

/**
 ** Global Variables
 **/

static char pval[150]; /* the path string is built here */

/*
 * the following are used to pass results by side-effect from the domain()
 * routine.
 */

static char prefix[80], suffix[80], fullsite[80];

/**
 ** Subroutines
 **/

/*
 * The Domain Table and its associated routines
 */

static struct domains
{
	char dom[50];
	char pre[50];
	char suf[50];
	char map[50];
} domtab[100];

/* Inline routine to copy a domain into the domain table */

#define DOMCPY(fld) { int i = 0; q=dp->fld; while (*p!=','&&*p!='\n'&&*p) \
		    {*q++ = *p++; if (i++>=48) break;} \
		    *q++ = '\0'; if (!*p) { \
		    fprintf(stderr,"opath: fld(s) missing in %s at %s\n", \
		    s, buf); \
		    dp++; continue;} p++; }

/* Load the domain table from disk */

static int
loaddomtab(s)
char *s;
{
FILE *f;
char buf[100];
register char *p,*q;
struct domains *dp;

	f = fopen(s,"r");
	if (f==NULL)
	{
		fprintf(stderr,"opath: can't open domain file '%s'\n",s);
		exit(1);
	}

	dp = domtab;

	while (fgets(buf,100,f))
	{
		if (buf[0]=='#') continue; /* comments start with "#" */
		p = buf;
		DOMCPY(dom);
		DOMCPY(pre);
		DOMCPY(suf);
		DOMCPY(map);
		if (dp->map[0] == '\0')
		{
			fprintf(stderr,"opath: bad route template in %s\n",s);
			strcpy(dp->map,"Invalid");
		}
		dp++;
	}

	dp->map[0] = '\0';  /* mark end of table */
	fclose(f);

	return(0);
}

/* Get a UUCP path from the pathalias database */

static char *
gpath(s)
char *s;
{
static char path[100];
char *uupath();

	strcpy(path,uupath(s));
	return(path);
}

/* String compare: entire first argument must match suffix of 2nd argument */

static int
domcmp(ss,tt)
char *ss, *tt;
{
char *s, *t;
int cmp;

	s = ss + strlen(ss) - 1;
	t = tt + strlen(tt) - 1;

	do
	{
		if (*s - *t) break;
		s--;
		t--;
	} while (s >= ss);

	if (++s == ss) return(0);
	else return(1);
}

/* Look up a domain, and by side effect set prefix and suffix appropriately */

char *domain(s)
char *s;
{
struct domains *d;
char *p;
static int loaded = 0;

	if (!loaded++) loaddomtab(domfile);

	if (*s!='.') /* default to UUCP domain */
	{
		prefix[0]=suffix[0]='\0';
		return("%R!%U");
	}

	for (p=s; *p; p++) if (*p>='a' && *p<='z') *p -= 'a'-'A';

	for (d = &domtab[0]; (int)d->map[0]; d++)
	{
		if (domcmp(d->dom,s)==0) break;
	}

	strcpy(prefix,(d->pre[0]=='>')? gpath(&d->pre[1]) : d->pre);
	strcpy(suffix,d->suf);

	return(d->map);
}

/* opath: generates a UUCP path from an RFC-822 address */

#define COPYON(s) {char *r; r=s; while (*r) *p++ = *r++; *p = '\0';}

char *
opath(s)
char *s;
{
char user[50],site[50];
static char cm[150];
FILE *f;
char *p, *q, *t;
char *d;
int i;
int found;
char *suf;

	for (p=user,q=s;(*p = *q)!='@'; p++,q++)
		if (*q=='\0') return(s);
	*p = '\0';

	strcpy(fullsite,++q);

	for (p=site;(*p = *q)!='.'; p++,q++)
		if (*q=='\0') break;
	*p = '\0';

	d = domain(q);

	if (d[0]=='\0') return(s); /* unknown domain - do nothing */

	for (p=pval, q=d; *q; q++)
	{
		if (*q=='%')
		{
			switch(*++q)
			{
			case 'P':
				COPYON(prefix);
				break;

			case 'S':
				COPYON(suffix);
				break;

			case 'U':
				COPYON(user);
				break;

			case 'N':
				COPYON(site);
				break;

			case 'D':
				COPYON(fullsite);
				break;

			case 'R':
				COPYON(gpath(site));
				break;

			case '%':
				*p++ = '%';
				break;
			}
		}
		else
			*p++ = *q;
	}

	return(pval);
}

/* oupath: generates a uucp path from a (possibly disconnected) uucp path */

char *oupath(s)
char *s;
{
char *p,*q;
static char adr[100];
char first[100];
int found;

	for (p=s,q=first,found=0; *p!='!' && *p!='\0'; p++)
	{
		if (*p=='.') found++;
		*q++ = *p;
	}
	if (*p=='\0') return (s);

	*q = '\0';

	if (found)
	{
		strcpy(adr,++p);
		strcat(adr,"@");
		strcat(adr,first);
		return(opath(adr));
	}
	else
	{
	int i;
		strcpy(adr,gpath(first));
		strcat(adr,/* ++ */p);

		return(adr);
	}
}

/**
 ** Public-domain subroutines obtained from net.sources
 **/

/*
 * The following is extracted from William Sebok's uupath program
 */

/* * * * uupath - look up path to computer in database * * * W.Sebok 11/4/83 */
#ifdef NULL
#undef NULL
#define NULL 0
#endif

typedef struct {char *dptr; int dsize;} datum;
datum fetch();
datum firstkey();
datum nextkey();


static char *
uupath(s)
char *s;
{
	char *fil;
	int ret;
	datum key;
	static datum result;
	char buf[BUFSIZ];
	static int inited = 0;

	fil = archive;
	strcpy(buf,s);
/***    strcat(buf,"!");  ***/
	key.dptr = buf;
	key.dsize = strlen(key.dptr) + 1 ;
	if (!inited++) {
		ret = dbminit(fil);
		if (ret != 0) {
			fprintf(stderr,"Can't open database '%s'\n",fil);
			return(s);
		}
	}
	result = fetch(key);
	if (result.dptr != NULL) {
		if (domcmp("!%s",result.dptr)==0) {
			result.dptr[result.dsize-4] = '\0';
		}
		/* next line handles strange pathalias "feature" */
		if (strcmp(result.dptr,"%s")==0) return(SYSNAME);
		return(result.dptr);
	} else {
		fprintf(stderr,"%s not found\n",s);
		return(s);
	}
	return(s);
}

opathlog(fmt,a,b,c,d)
char *fmt;
int a,b,c,d;
{
FILE *f;

	f = fopen(logfile,"a");
	if (f==NULL) return;

	fprintf(f,fmt,a,b,c,d);
	fclose(f);
}
