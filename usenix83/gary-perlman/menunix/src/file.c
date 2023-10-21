/*Copyright (c) 1981 Gary Perlman  All rights reserved*/
#include "menu.h"
char *
prname (s, dirpath, pathlength) char *s; char dirpath[MAXDEPTH][NAMELENGTH];
	{
	int	i;
	strcpy (s, "/");
	for (i = 0; i < pathlength; i++)
		{
		strcat (s, dirpath[i]);
		strcat (s, "/");
		}
	return (s);
	}
char *
pwd (s) char *s;
	{
	char	*pwd;
	char	pathname[MAXDEPTH][NAMELENGTH];
	ino_t	inode[MAXDEPTH];
	struct	direct dirbuf;
	struct	stat statbuf;
	dev_t	dotdevno;
	ino_t	rootinode;
	FILE	*ioptr;
	int	i;

	pathlength = 0;
	if (stat ("/", &statbuf)) printf ("Can't stat /\n");;
	rootinode = statbuf.st_ino;
	if (stat (".", &statbuf)) printf ("Can't stat .\n");
	dotdevno = statbuf.st_dev;
	for (;;)
	{
	if ((ioptr = fopen (".", "r")) == NULL)
		{
		printf ("Can't open current directory");
		return (NULL);
		}
	if (fread (&dirbuf, sizeof (dirbuf), 1, ioptr) != 1)
	    {
	    fclose (ioptr);
	    return (NULL);
	    }
	else inode[pathlength] = dirbuf.d_ino;
	fread (&dirbuf, sizeof (dirbuf), 1, ioptr);
	if (pathlength)
	    while (fread (&dirbuf, sizeof (dirbuf), 1, ioptr))
		if (dirbuf.d_ino == inode[pathlength-1])
		    {
		    strncpy (pathname[pathlength-1], dirbuf.d_name, DIRSIZ);
		    break;
		    }
	fclose (ioptr);
	if (inode[pathlength] == rootinode)
	    {
	    chdir ("/");
	    ioptr = fopen ("/", "r");
	    while (fread (&dirbuf, sizeof (dirbuf), 1, ioptr))
		{
		if (stat (dirbuf.d_name, &statbuf)) continue;
		if (statbuf.st_dev == dotdevno)
		    {
		    strncpy (dirpath[0], dirbuf.d_name, DIRSIZ);
		    pathlength++;
		    for (i = 1; i < pathlength; i++)
			strcpy (dirpath[i], pathname[pathlength-i-1]);
		    if (pathlength == 1 && dirpath[0][0] == '.')
		    pathlength = 0;
		    chdir (pwd = prname (s, dirpath, pathlength));
		    fclose (ioptr);
		    return (pwd);
		    }
		}
	    }
	pathlength++;
	fclose (ioptr);
	chdir ("..");
	}
	}

cd (dirname) char *dirname;
	{
	char	vardir[BUFSIZ];
	char	*interpolate ();
	char	*getenv ();
	int	i;
	if (!dirname) return (-1);
	if (!*dirname) dirname = getenv ("HOME");
	if (chdir (dirname))
		{
		sprintf (vardir, "%c%s", varchar, dirname);
		dirname = interpolate (vardir);
		if (*dirname == NULL) return (-1);
		if (chdir (dirname = interpolate (vardir))) return (-1);
		}
	if (!strcmp (dirname, ".")) return (0);
	if (*dirname == '/' || *dirname == '.')
		{
		pwd (pwdname);
		return (0);
		}
	if (!strcmp (dirname, ".."))
		if (pathlength) pathlength--;
		else return (0);
	else strcpy (dirpath[pathlength++], dirname);
	prname (pwdname, dirpath, pathlength);
	return (0);
	}

newdir ()
	{
	int	flecmp ();
	int	nonames;
	struct	direct d;
	struct	stat buf;
	FILE	*ioptr = fopen (".", "r");
	if (ioptr == NULL) return (0);
	nonames = 0;
	while (fread (&d, sizeof (d), 1, ioptr) == 1)
		if (d.d_ino)
			{
			if (!strcmp (d.d_name, ".")) continue;
			if (!strcmp (d.d_name, "..")) continue;
			if ((!trueval ("printdotfiles")) && *d.d_name == '.')
				continue;
			if (stat (d.d_name, &buf))
				{
				printf ("Can't stat %s", filent[nonames].f_name);
				getchar ();
				continue;
				}
			strncpy (filent[nonames].f_name, d.d_name, DIRSIZ);
			setmode (filent[nonames].f_protect, &buf);
			filent[nonames].f_size = buf.st_size;
			if (++nonames == MAXENTRIES) break;
			}
	qsort (filent, nonames, sizeof (struct FILENT), flecmp);
	fclose (ioptr);
	return (nonames);
	}

#define UNSET '-'
#define UPPER 'A'-'a'
setmode (s, statbuf) char *s; struct stat *statbuf;
	{
	int	i;
	short	mode = statbuf->st_mode;
	for (i = 1; i < 10; i++) s[i] = UNSET;
	s[10] = NULL;
	switch (mode & S_IFMT)
		{
		case S_IFREG: s[0] = '-'; break;
		case S_IFDIR: s[0] = 'd'; break;
		case S_IFCHR: s[0] = 'c'; break;
		case S_IFBLK: s[0] = 'b'; break;
		case S_IFMPC: s[0] = 'C'; break;
		case S_IFMPB: s[0] = 'B'; break;
		default: s[0] = '?';
		}
	for (i = 0; i < 9; i+=3)
		{
		if (mode & (S_IREAD  >> i)) s[i + 1] = 'r';
		if (mode & (S_IWRITE >> i)) s[i + 2] = 'w';
		if (mode & (S_IEXEC  >> i)) s[i + 3] = 'x';
		}
	if (uid == statbuf->st_uid)
		{
		for (i = 1; i <= 3; i++)
			if (s[i] != UNSET) s[i] += UPPER;
		}
	else if (gid == statbuf->st_gid)
		{
		for (i = 4; i <= 6; i++)
			if (s[i] != UNSET) s[i] += UPPER;
		}
	else for (i = 7; i <= 9; i++)
		if (s[i] != UNSET) s[i] += UPPER;
	}

flecmp (f1, f2) struct FILENT *f1, *f2;
	{
	int	strcmp ();
	if (f1->f_protect[0] == 'd' && f2->f_protect[0] != 'd') return (-1);
	if (f2->f_protect[0] == 'd' && f1->f_protect[0] != 'd') return (1);
	return (strcmp (f1->f_name, f2->f_name));
	}

vdir (page, nonames)
	{
	int	i;
	int	reali;
	char	pagecount[10];
	int	npages = nonames/PAGESIZE + (nonames%PAGESIZE?1:0);
	if (page < 1) page = npages;
	else if (page > npages) page = 1;
	wclear (filewin);
	for (i = 0; i < PAGESIZE; i++)
		{
		if ((reali = i + (page-1)*PAGESIZE) == nonames) break;
		mvwprintw (filewin,1+i, 0, "%c", '1'+i);
		if (filent[reali].f_protect[0] == 'd') /* directory */
			{
			if (trueval ("highlight")) wstandout (filewin);
			mvwprintw (filewin,1+i, INDENT, "%s/",
				filent[reali].f_name);
			if (trueval ("highlight")) wstandend (filewin);
			}
		else	mvwprintw (filewin,1+i, INDENT, "%s",
				filent[reali].f_name);
		mvwprintw (filewin, 1+i, 20, "%9d %10s",
			filent[reali].f_size, filent[reali].f_protect);
		}
	if (trueval ("highlight")) wstandout (filewin);
	mvwprintw (filewin,0,0, "%s", pwdname);
	if (trueval ("highlight")) wstandend (filewin);
	sprintf (pagecount, "%d/%d", page, npages);
	mvwprintw (filewin,0,COLS/2-5, "%5s", pagecount);
	wrefresh (filewin);
	return (page);
	}

fileprocess (c) char c;
    {
    char	*getargs (), *getval ();
    char	*strptr;
    int	i = c - '1' + (page-1)*PAGESIZE;
    if (i >= nonames)
	{
	printf ("");
	return;
	}
    mvwprintw (filewin, 1+c-'1', 2, "<-");
    wrefresh (filewin);
    if (filent[i].f_protect[0] == 'd' && access (filent[i].f_name, 1) == 0)
	{
	cd (filent[i].f_name);
	nonames = newdir ();
	page = vdir (page=1, nonames);
	}
    else if (access (filent[i].f_name, 1) == 0)
	{
	sprintf (command, "%s ", filent[i].f_name);
	if ((strptr = getargs ("{arguments?}", NULL)) == NULL) return;
	strcat (command, strptr);
	clear (); refresh ();
	syscall (command);
	GETRETURN
	clear (); refresh ();
	flipped = 0;
	display (menu);
	}
    else if (access (filent[i].f_name, 4) == 0)
	{
	sprintf (command, "$editor %s", filent[i].f_name);
	move (LINES-1, 0); clrtoeol (); refresh ();
	syscall (command);
	clear (); refresh ();
	flipped = 0;
	display (menu);
	}
    else printf ("");
    }
