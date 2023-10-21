#include <stdio.h>
#include <pwd.h>
#include <sys/param.h>
#include "config.h"

extern char *Editor,*Ps1,*Mailer,*Printer,*Poster;
extern char *Onews,*Newsrc,*Orgdir,*Savedir;	/* path names */

/*
	environment variable, original directory string setup.
*/

envir_set ()
{
	char dbuf [MAXPATHLEN], *rcname, *getenv(), *getwd(), *str_store();
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
	sprintf (dbuf, "%s/%s",ptr->pw_dir,rcname);
	Newsrc = str_store (dbuf);
	sprintf (dbuf, "%s/%s%s",ptr->pw_dir,".vn","XXXXXX");
	Onews = str_store (mktemp(dbuf));
	if (access (Newsrc,0) != 0)
		creat (Newsrc,0666);
}
