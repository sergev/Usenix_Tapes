#include <stdio.h>
#include <utmp.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"

int force_update=0;
struct uentry utab[MAXUSERS];
struct uentry *u_first=utab;
struct uentry *u_free=NULL;
static int    active;

/* to print out the time when we get to it */
struct timeval lastupdate;
struct timezone tzp;

/*  nowbuf contains the last time the utmp file was updated 
 *  and nowtime contains the last time the alertd ran updating its
 *  data structures. 
 */
char   nowbuf[5];		/* buffer containing "13:00" */
char   nowtime[5];		/* time string */

alertd_initialize()
{
    register int i;
    active =0;
    for(i=0; i< MAXUSERS; i++) {
	utab[i].name[0] = '\0';
	utab[i].no_ttys = 0;
	utab[i].being_watched = 0;
	utab[i].next = NULL;
    }
}

char errmsg[128];
free_uentry(name)
char *name;
{
    register struct uentry *temp = u_first;
    register struct uentry *prev = NULL;
    register int found = 0;

    while (temp != NULL)  {
	if(!strcmp(name, temp->name)) {
	    found++; break;
	}
	prev = temp;
	temp = temp->next;
    }
    if(!found) {
	sprintf(errmsg,"free_uentry/%s is not in the table\n", name);
	logerror(errmsg);
	return(-1);
    }
    *(char *)(temp->name) = '\0';
    temp->no_ttys = 0;
    if(temp == u_first) {
	if(u_first->next != NULL)
	  u_first = u_first->next;
    }
    else prev->next = u_first->next;
    temp->next = u_free;
    u_free = temp;
}


struct uentry *
get_uentry()
{
    struct uentry *item;
    item = NULL;
    /* free list not empty */
    if(u_free != NULL) {
	item = u_free;
	u_free = u_free->next;
	return(item);
    }
    else {
	if((active + 1) >= MAXUSERS) {
	    sprintf(errmsg,"Too many users. Increase MAXUSERS?\n");
	    logerror(errmsg);
	    return(NULL);
	}
	else {
	    item = (struct uentry *)&utab[active++];
	    return(item);
	}
    }
}

struct ttyentry *
find_ttyentry(ptr, dev)
register struct uentry *ptr;
char *dev;
{
    register struct ttyentry *temp;
    register int i;
    for(i=0; i < ptr->no_ttys; i++) {
	temp = &(ptr->ttys[i]);
	if(!strncmp(dev,temp->name,NMAX)) return(temp);
    }
    return(NULL);
}

struct uentry *
find_user(uname)
register char *uname;
{
    register struct uentry *temp = u_first;
    while(temp != NULL) {
	if(!strcmp(uname,temp->name)) return(temp);
	else temp= temp->next;
    }
    return(NULL);
}

#ifdef DEBUG
print_utab()
{
    register struct ttyentry *i;
    register int j,k;
    register struct uentry *temp = u_first;
    while(temp != NULL) {
	printf("\t %s is logged in at ", temp->name);
	for(j=0;j<temp->no_ttys; j++) {
	    i = &(temp->ttys[j]);
	    printf("%s ",i->name);
	    fflush(stdout);
	}
	temp = temp->next;
	printf("\n");
    }
    printf("\n");
}
#endif

touch_idlebits()
{
    register struct uentry *temp;
    register int i;
    temp = u_first;
    while(temp != NULL) {
	for(i=0; i< temp->no_ttys; i++) 
	    (temp->ttys[i]).idletime = -1;
	temp = temp->next;
    }
}

do_update()
{
    register FILE *fp;
    register int i,j,delflag;
    struct stat buf;
    struct utmp utmp;
    struct uentry *temp,*prev;
    register struct ttyentry *ttyp;
    register int no_entries;

    /* first time */
    if(force_update > 0) {
	if(stat("/etc/utmp", &buf)<0) {
	    sprintf(errmsg,"do_update/Couldn't stat utmp file?\n");
	    logerror(errmsg);
	    exit(0);
	}
	/* if it has been modified we need to do work*/
	if(!(buf.st_mtime > lastupdate.tv_sec)){
	    touch_idlebits(); /* this is to force recalculation of
				 idletimes regardless of utmp file */
	    gettimeofday(&lastupdate, (struct timezone *)&tzp);
	    strcpyn(nowtime,
		    (char *)(asctime(localtime(&(lastupdate.tv_sec))))+11,5);
	    nowtime[5] = '\0';
	    return(0);
	}
    }
    force_update = 1;
    if((fp = fopen("/etc/utmp", "r")) == NULL) {
	perror("/etc/utmp");
	exit(1);
    }


    fflush(stdout);
    /* reset touch bits */
    temp = u_first;
    while(temp != NULL) {
	for(i=0;i<temp->no_ttys; i++)
	    (temp->ttys[i]).touch = 0;
	temp = temp->next;
    }

    no_entries = 0;

    while(fread((char *)&utmp, sizeof(utmp), 1, fp) == 1) {
	if(utmp.ut_name[0] == '\0') continue;
	no_entries++;
	temp = u_first; 
	prev = NULL;
	while (temp != NULL) {
	    if(!strcmp(utmp.ut_name, temp->name)) break;
	    else {
		prev = temp;
		temp = temp->next;
	    }
	}
	if(temp == NULL) {
	    /* coudnt find him - allocate a new entry */
	    if((temp = get_uentry()) == NULL) {
		sprintf(errmsg,"do_update/uentry allocation failed\n");
		logerror(errmsg);
		continue;
	    }
	    if(prev != NULL && prev != temp) prev->next = temp;
	    temp->no_ttys = 0;
	    temp->next = NULL;
	    strncpy(temp->name, utmp.ut_name, NMAX);
	}

	if((ttyp=find_ttyentry(temp,utmp.ut_line)) != NULL) 
	  { /* he has already logged in on this line */
	      ttyp->touch = 1;	/* he still is logged on */
	      ttyp->idletime = -1;
	      continue;
	  }
	else {
	    if((temp->no_ttys + 1) >= NO_TTYS) {
		sprintf(errmsg,"do_update/couldnt allocate ttyentry\n");
		logerror(errmsg);
		continue;
	    }
	    ttyp = &(temp->ttys[temp->no_ttys]);
	    ttyp->touch = 1;
	    ttyp->idletime = -1;
	    strncpy(ttyp->name, utmp.ut_line, NMAX);
	    /* new login processing done here */
	    find_and_notify(temp->name, ttyp->name, LOGIN, temp->no_ttys,
			    0);
	    temp->no_ttys++;
	}
    }
    
    if (no_entries == 0) goto exit;
    /* do logout processing */
    temp = u_first;
    prev = NULL;
    while(temp != NULL) {
	delflag = 0;

	for(i=0; i < temp->no_ttys; i++) {
	    ttyp = &(temp->ttys[i]);
	    if(ttyp->touch == 0) {
		for(j=i;j<temp->no_ttys-1;j++) {
		    temp->ttys[j] = temp->ttys[j+1];
		}
		temp->no_ttys--;
		find_and_notify(temp->name, ttyp->name, LOGOUT, 
				i,temp->no_ttys);

		i--;		/* this is needed */
	    }
	    else {
		delflag = 1;
	    }
	}
	if(delflag == 0) {
	    /* 
	     * If this guy had been watching others we should delete 
	     * him since we dont want alert to persist over different
	     * sessions (or do we?)
             */
	    free_hunter(temp->name);
	    temp->name[0] = '\0';
	    temp->no_ttys = 0;
	    if(temp == u_first) {
		if(u_first->next != NULL) {
		    u_first = u_first->next;
		    temp->next = u_free;
		    u_free = temp;
		    temp = u_first;
		}
		else {
		    temp->next = u_free;
		    u_free = temp;
		    u_first = temp = NULL;
		}
	    }
	    else {
		prev->next = temp->next;
		temp->next = u_free;
		u_free = temp;
		temp = prev->next;
	    }
	}
	else {
	    prev = temp;
	    temp = temp->next;
	}
    }
  exit:
    gettimeofday(&lastupdate, (struct timezone *)&tzp);
    strcpyn(nowbuf,(char *)(asctime(localtime(&(lastupdate.tv_sec))))+11,5);
    nowbuf[5] = '\0';
    fclose(fp);
    return(0);
}

#ifdef DEBUG
main()
{
    for(;;) {
	printf("Doit? \n");
	getchar(); getchar();
	do_update();
	print_utab();
    }
}

#endif
