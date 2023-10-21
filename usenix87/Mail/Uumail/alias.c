#include "uuconf.h"
#ifndef lint
static char rcsid[] = "$Header: alias.c,v 4.0 86/11/17 16:02:00 sob Exp $";
#endif
/***************************************************************************
This work in its current form is Copyright 1986 Stan Barber
with the exception of opath, gethostname and the original getpath which
as far as I know are in the Public Domain. This software may be distributed
freely as long as no profit is made from such distribution and this notice
is reproducted in whole.
***************************************************************************
This software is provided on an "as is" basis with no guarantee of 
usefulness or correctness of operation for any purpose, intended or
otherwise. The author is in no way liable for this software's performance
or any damage it may cause to any data of any kind anywhere.
***************************************************************************/
/* These routines based in part on the aliasing facility of MH Version 1.7 */
/* $Log:	alias.c,v $
 * Revision 4.0  86/11/17  16:02:00  sob
 * Release version 4.0 -- uumail
 * 
 * Revision 3.3  86/10/21  15:06:12  sob
 * Added lint #indef to make lint happier
 * 
 * Revision 3.2  86/07/11  17:57:42  sob
 * renamed parse to aliasparse to avoid name conflict with resolve
 * 
 * Revision 3.1  86/05/13  12:36:47  sob
 * Added the ability to escape sensitive characters per suggestion
 * of tp@ndm20.UUCP.
 * 
 * Revision 3.0  86/03/14  12:04:41  sob
 * Release of 3/15/86 --- 3rd Release
 * 
 * Revision 1.10  86/03/14  11:57:13  sob
 * updated copyright
 * 
 * Revision 1.9  86/03/11  11:28:40  sob
 * Added Copyright Notice
 * 
 * Revision 1.8  86/03/03  17:16:39  sob
 * Added fixes provided by desint!geoff.
 * Stan
 * 
 * Revision 1.7  86/02/26  03:07:20  sob
 * This forward method seems to work. It is a bit awkward, but it does seem
 * to work. We will freeze the release here.
 * 
 * Revision 1.6  86/02/24  12:46:31  sob
 * Fixed some problems with .forward. Still not completely correct, but better.
 * Stan
 * 
 * Revision 1.5  86/02/23  23:48:50  sob
 * This version contains the first attempt to make .forwards work.
 * Stan
 * 
 * Revision 1.4  86/02/23  23:01:12  sob
 * This version will correctly make note of programs that can have output
 * of uumail directly piped into (in place of mail or uux).
 * 
 * Revision 1.3  86/02/18  01:56:12  sob
 * MH aliasing facility has been installed. Now comes time to test.
 * Stan
 * 
 * Revision 1.2  86/02/17  18:42:47  sob
 * First update to add linked list of addresses. Real aliasing to be
 * added next.
 * 
 * Revision 1.1  86/02/10  16:54:12  sob
 * Initial revision
 * 
 *
 */
#ifdef NOALIAS
char *
alias()
{
	return;
}
#else
EXTERN struct mailname addrlist;
#define GROUP "/etc/group"
char *termptr;


/* Conditional free -- perform a free call if the address passed
 * is in free storage;  else NOP
 */


cndfree(addr)
char *addr;
{
	extern char end;

	if(addr >= &end) free(addr);
}

uleq(c1, c2)
register char *c1, *c2;
{
	register int c;

	while(c = *c1++)
		if((c|040) != (*c2|040))
			return(0);
		else
			c2++;
	return(*c2 == 0);
}
/* modifications to allow quoting of characters that usually indicate
 * address delimiters provided by tp@ndm20.UUCP
 */
char *aliasparse(ptr, buf)
register char *ptr;
char *buf;
{
	register char *cp;

	cp = buf;
	while(isspace(*ptr) || *ptr == ',' || *ptr == ':')
		ptr++;
	while(isalnum(*ptr) || *ptr == '/' || *ptr == '-' || *ptr == '.' || *ptr == '!' || *ptr == '@' || *ptr == '%' || *ptr == '\\') {
		if(*ptr == '\\') ptr++;
		if(*ptr != '\0') *cp++ = *ptr++;
	}
	if(cp == buf) {
		switch(*ptr) {
		case '<':
		case '|':
		case '=':
			*cp++ = *ptr++;
		}
	}
	*cp = 0;
	if(cp == buf)
		return 0;
	termptr = ptr;
	return buf;
}
char *
advance(ptr)
	register char *ptr;
{
	return(termptr);
}

alias()
{
	register char *cp, *pp;
	register struct mailname *lp;
	char line[256], pbuf[64];
	FILE *a;

	if((a = fopen(AliasFile, "r")) == NULL)
		return;
	while(fgets(line, sizeof line, a)) {
		if(line[0] == ';' || line[0] == '\n')   /* Comment Line */
			continue;
		if((pp = aliasparse(line, pbuf)) == NULL) {
	    oops:       fprintf(stderr, "Bad alias file %s\n", AliasFile);
			fprintf(stderr, "Line: %s", line);
			exit(EX_OSFILE);
		}
		for(lp = &addrlist; lp->m_next; lp = lp->m_next) {
			if(aleq(lp->m_next->m_name, pp)) {
				remove(lp);
				if(!(cp = advance(line)) ||
				   !(pp = aliasparse(cp, pbuf)))
					goto oops;
				switch(*pp) {
				case '<':       /* From file */
					cp = advance(cp);
					if((pp = aliasparse(cp, pbuf)) == NULL)
						goto oops;
					addfile(pp);
					break;
				case '=':       /* UNIX group */
					cp = advance(cp);
					if((pp = aliasparse(cp, pbuf)) == NULL)
						goto oops;
					addgroup(pp);
					break;
			       case '|':	/* pipe through a program */
					cp = advance(cp);
					if ((pp=aliasparse(cp,pbuf)) == NULL)
						goto oops;
					addprgm(pp);
					break;
				default:        /* Simple list */
					for(;;) {
						insert(pp);
						if(!(cp = advance(line)) ||
						   !(pp = aliasparse(cp, pbuf)))
							break;
					}
				}
				break;
			}
		}
	}
}





addfile(file)
	char *file;
{
	register char *cp, *pp;
	char line[128], pbuf[64];
	FILE *f;
#ifdef DEBUG
	if (Debug >3) printf("addfile(%s)\n", file);          
#endif
	if((f = fopen(file, "r")) == NULL) {
		fprintf(stderr, "Can't open ");
		perror(file);
		exit(EX_OSFILE);
	}
	while(fgets(line, sizeof line, f)) {
		cp = line;
		while(pp = aliasparse(cp, pbuf)) {
			insert(pp);
			cp = advance(cp);
		}
	}
	fclose(f);
}

addgroup(group)
	char *group;
{
	register char *cp, *pp;
	int found = 0;
	char line[128], pbuf[64], *rindex();
	FILE *f;
#ifdef DEBUG
	if(Debug>3)printf("addgroup(%s)\n", group);        
#endif
	if((f = fopen(GROUP, "r")) == NULL) {
		fprintf(stderr, "Can't open ");
		perror(GROUP);
		exit(EX_OSFILE);
	}
	while(fgets(line, sizeof line, f)) {
		pp = aliasparse(line, pbuf);
		if(strcmp(pp, group) == 0) {
			cp = rindex(line, ':');
			while(pp = aliasparse(cp, pbuf)) {
				insert(pp);
				cp = advance(cp);
			}
			found++;
		}
	}
	if(!found) {
		fprintf(stderr, "Group: %s non-existent\n", group);
		exit(EX_DATAERR);
	}
	fclose(f);
}

addprgm(name)
char *name;
{
	register struct mailname *mp;
	char * getcpy();
	for(mp = &addrlist; mp->m_next; mp = mp->m_next)
		if(uleq(name, mp->m_next->m_name))
			return;         /* Don't insert existing name! */
	mp->m_next = (struct mailname *) malloc(sizeof *mp->m_next);
	mp = mp->m_next;
	mp->m_next = 0;
	mp->m_name = getcpy(name);
	mp->m_pipe = 1;
}	
remove(mp)              /* Remove NEXT from argument node! */
	register struct mailname *mp;
{
	register struct mailname *rp;

	rp = mp->m_next;
	mp->m_next = rp->m_next;
	cndfree(rp->m_name);
	cndfree(rp);
}
insert(name)
	char *name;
{
	register struct mailname *mp;
	char *getcpy();
#ifdef DEBUG
	if(Debug>3)  printf("insert(%s)\n", name);   
#endif
	for(mp = &addrlist; mp->m_next; mp = mp->m_next)
		if(uleq(name, mp->m_next->m_name))
			return;         /* Don't insert existing name! */
	mp->m_next = (struct mailname *) malloc(sizeof *mp->m_next);
	mp = mp->m_next;
	mp->m_next = 0;
	mp->m_pipe=0;
	mp->m_name = getcpy(name);
}

aleq(string, aliasent)
	register char *string, *aliasent;
{
	register int c;

	while(c = *string++)
		if(*aliasent == '*')
			return 1;
		else if((c|040) != (*aliasent|040))
			return(0);
		else
			aliasent++;
	return(*aliasent == 0 | *aliasent == '*');
}

forward()
{

	FILE * fp;
	struct passwd * pwd;
	struct mailname *lp;
	char forwardfile[BUFSIZ];
	extern struct passwd * getpwnam ();

	for (lp = addrlist.m_next;lp;lp=lp->m_next){
	if (index(lp->m_name,'!')) continue; /* not local */
	if (index(lp->m_name,'@')) continue; /* ditto */
	if (index(lp->m_name,'%')) continue; /* ditto */
	if (index(lp->m_name,'/')) continue; /* filename */
	if ((pwd = getpwnam(lp->m_name)) == NULL) continue;
	sprintf(forwardfile,"%s/.forward",pwd->pw_dir);
	if((fp=fopen(forwardfile,"r")) != NULL){
			strcpy(lp->m_name,"");
			addfile(forwardfile);
		fclose(fp);
		}
	}
}
#endif


/* add names to the address list */


add(name, list)
char *name;
struct mailname *list;
{
	register struct mailname *mp;
	char *getcpy();

	for(mp = list; mp->m_next; mp = mp->m_next)
		;
	mp->m_next = (struct mailname *) malloc(sizeof *mp->m_next);
	mp = mp->m_next;
	mp->m_next = 0;
	mp->m_pipe = 0;
	mp->m_name = getcpy(name);
}

char *getcpy(str)
{
	register char *cp;

	cp = (char *) malloc(strlen(str) + 1);
	strcpy(cp, str);
	return(cp);
}







