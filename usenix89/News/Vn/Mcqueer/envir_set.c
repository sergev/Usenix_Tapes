/*
** vn news reader.
**
** envir_set.c - routine to obtain pertinent environment variable settings
**		and set up file / directory names
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>
#include <pwd.h>
#include <sys/param.h>
#include "config.h"

extern char *Editor,*Ps1,*Mailer,*Printer,*Poster;
extern char *Onews, *Newsrc, *Orgdir, *Savedir, *Ccfile;	/* path names */
extern char Cxitop[], Cxitor[], Cxrtoi[], Cxptoi[];

#ifdef SYSV
extern char *getcwd();
#define getwd(a) getcwd(a,sizeof(a))
#define	MAXPATHLEN 240
#else
extern char *getwd();
#endif

/*
	environment variable, original directory string setup.
*/

envir_set ()
{
 	char dbuf [MAXPATHLEN], *rcname, *ccname, *keyxln;
	char *getenv(), *getcwd(), *str_store();
	struct passwd *ptr, *getpwuid();

	if ((Ps1 = getenv("PS1")) == NULL)
		Ps1 = DEF_PS1;
	if ((Editor = getenv("EDITOR")) == NULL)
		Editor=DEF_ED;
	if ((Mailer = getenv("MAILER")) == NULL)
		Mailer=DEF_MAIL;
	if ((Poster = getenv("POSTER")) == NULL)
		Poster=DEF_POST;
	if ((Printer = getenv("PRINTER")) == NULL)
		Printer=DEF_PRINT;
	if ((rcname = getenv("NEWSRC")) == NULL)
		rcname=DEF_NEWSRC;
	if ((ccname = getenv("CCFILE")) == NULL)
		ccname=DEF_CCFILE;
	if ((keyxln = getenv("VNKEY")) == NULL)
		keyxln=DEF_KEYXLN;
	Savedir = getenv("VNSAVE");

	/*
		set original directory strings.  create empty Newsrc if it doesn't exist
	*/

	ptr = getpwuid (getuid());
	if ((Orgdir = getwd(dbuf)) == NULL)
		printex ("cannot stat pwd");
	Orgdir = str_store (Orgdir);
	if (Savedir == NULL)
		Savedir = Orgdir;
	if (*rcname != '/')
	{
		sprintf (dbuf, "%s/%s",ptr->pw_dir,rcname);
		Newsrc = str_store (dbuf);
	}
	else
		Newsrc = str_store (rcname);
	if (*ccname != '/')
	{
		sprintf (dbuf, "%s/%s",ptr->pw_dir,ccname);
		Ccfile = str_store (dbuf);
	}
	else
		Ccfile = str_store (ccname);
	sprintf (dbuf, "%s/%s%s",ptr->pw_dir,".vn","XXXXXX");
	Onews = str_store (mktemp(dbuf));
	if (access (Newsrc,0) != 0)
		creat (Newsrc,0666);

	if (*keyxln != '/')
	{
		sprintf(dbuf, "%s/%s",ptr->pw_dir,keyxln);
		set_kxln(dbuf);
	}
	else
		set_kxln(keyxln);
}

static
set_kxln(fname)
char *fname;
{
	FILE *fp;
	int i;
	char bufr[80];
	char in,out,*ptr;
	char *index(), xln_str();

	for (i=0; i < 128; ++i)
		Cxitop[i] = Cxitor[i] = Cxptoi[i] = Cxrtoi[i] = i;

	if ((fp = fopen(fname,"r")) != NULL)
	{
		while(fgets(bufr,79,fp) != NULL)
		{
			if (strncmp(bufr+1,"==",2) == 0)
				ptr = bufr+2;
			else
				ptr = index(bufr+1,'=');
			if (ptr == NULL)
				continue;
			*ptr = '\0';
			++ptr;
			in = xln_str(bufr+1);
			out = xln_str(ptr);
			switch(bufr[0])
			{
			case 'r':
			case 'R':
				Cxrtoi[out] = in;
				Cxitor[in] = out;
				break;
			case 'p':
			case 'P':
				Cxptoi[out] = in;
				Cxitop[in] = out;
			default:
				break;
			}
		}
		fclose(fp);
	}
}

static char
xln_str(s)
char *s;
{
	if (*s < '0' || *s > '9')
		return(*s & 0x7f);
	return((char)(atoi(s) & 0x7f));
}
