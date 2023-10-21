#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "defs.h"

/*EXTERN*/
extern struct uentry *find_user(); /* this function is in utils.c */
extern int force_update;	   /* if eq to 0 causes utmp toberead */
int compute_idletime();

/*GLOBALS*/
struct hunter *h_head=NULL;	/* start of the hunter list */
struct hunter *h_tail=NULL;	/* end of the list of hunters */
int    h_error;

char  *h_errmsgs[] = {
   0,
   "Malloc failure",
   "Already added",
   "Watching too many users already",
   "Unknown hunter name",
   "Unknown user name",
   0 };

/*
 * Global deallocator of an entry from the list of hunters. This is used 
 * by the control msg FLUSH_USER. It is also used to clean up when a user
 * logs out. (see comment in utils.c)
 */
free_hunter(name)
char *name;
{
    struct hunter *temp=h_head;
    struct hunter *previous = NULL;
    while(temp != NULL) {
	if(!strncmp(temp->name,name,NMAX)) break;
	previous = temp;
	temp = temp->next;
    }				
    if(temp == NULL) {		
	h_error = 4;
	return(-1);
    }
    else {
	if(temp == h_head) {
	    if(temp == h_tail) {
		free(temp);
		h_head = h_tail = NULL;
	    }
	    else {
		h_head = h_head->next;
		free(temp);
	    }
	}
	else {
	    if(temp == h_tail) {
		h_tail = previous;
		h_tail->next = NULL;
		free(temp);
	    }
	    else {
		previous->next = temp->next;
		free(temp);
	    }
	}
    }
    force_update = 0;
    return(1);
}
	
/*
 * Given a name this returns a pointer to a struct of type hunter in the
 * global hunter list, or NULL
 */

struct hunter *
find_hunter(name)
char *name;
{
    register struct hunter *temp = h_head;
    while(temp != NULL) {
	if(!strncmp(temp->name, name, NMAX)) break;
	else temp = temp->next;
    }
    return(temp);
}

/*
 * Like find_hunter, but allocates a new structure if one doesnt already
 * exist.
 */

struct hunter *
get_hunter(name)
char *name;
{
    register struct hunter *temp = h_head;
    while(temp != NULL) {
	if(!strncmp(temp->name,name,NMAX)) break;
	else temp= temp->next;
    }
    if(temp != NULL) {
	return(temp);
    }

    if(h_head == NULL) {
	h_head = (struct hunter *)malloc(sizeof(struct hunter));
	if(h_head == NULL) return(NULL);
	h_tail = h_head;
	strncpy(h_head->name,name,NMAX);
	h_head->no_hunted = 0;
	h_head->next = NULL;
	return(h_head);
    }
    else {
	h_tail->next = (struct hunter *)malloc(sizeof(struct hunter));
	if(h_tail->next == NULL) return(NULL);
	h_tail = h_tail->next;
	strncpy(h_tail->name,name,NMAX);
	h_tail->no_hunted = 0;
	h_tail->next = NULL;
	return(h_tail);
    }
}

/* 
 * Top level user interface function to add a hunter and hunted to the
 * global list of hunters. Minimum idle time is 1 minute.
 * Returns -1, and sets h_error if an error occurs in the process
 */

add_hunted(user, hunter, idle)
register char *user;
register char *hunter;
int  idle;
{
    register struct hunter *temp;
    register int i;
    if((temp = get_hunter(hunter)) == NULL) {
	h_error = 1;
	return(-1);
    }
    else {
	for(i=0;i<temp->no_hunted; i++) {
	    if(!strncmp(user,temp->hunted[i], NMAX)) {
		h_error = 2;
		return(-1);
	    }
	    else continue;
	}
	if(i < MAX_WATCH) {
	    strncpy(temp->hunted[temp->no_hunted], user, NMAX);
	    if(idle < 1) idle = 1;
	    temp->idletime[temp->no_hunted] = idle;
	    temp->no_hunted++;
	    force_update = 0;
	    return(0);
	}
	else h_error = 3;
	return(-1);
    }
}

/*
 * Top level user callable function that deletes a given name that
 * a given hunter is watching. 
 */
delete_hunted(user, hunter)
char *user;
char *hunter;
{
    register struct hunter *temp;
    register int i,j;
    if((temp = find_hunter(hunter)) == NULL) {
	h_error = 4;
	return(-1);
    }
    else {
	for(i=0;i < temp->no_hunted; i++) {
	    if(!strncmp(temp->hunted[i], user, NMAX)) {
		for(; i < temp->no_hunted-1; i++) {
		    strncpy(temp->hunted[i],temp->hunted[i+1], NMAX);
		    temp->idletime[i] = temp->idletime[i+1];
		    for(j=0;j<MAX_WATCH; j++){
			temp->imagic[i][j] = temp->imagic[i+1][j];
			temp->hmagic[i][j] = temp->hmagic[i+1][j];
		    }
		}
		i++;
		temp->hunted[i][0] = '\0';
		temp->no_hunted--;
		force_update = 0;
		return(1);
	    }
	    else continue;
	}
	h_error = 5;
	return(-1);
    }
}

/* 
 * Voila - where all the work is done (called from onalarm)
 */

hunt()
{
    /* first we update the user table structures correctly */
    do_update();
    
    /* compute idletimes and do notifications */
    do_idletimes();

    /* check the time alarms */
    do_timealarms();
}


/* 
 * This function calculates idletimes of all the tty devices that
 * other users are interested in, and sends notifications to all
 * the terminals that those users may be logged in.
 */

do_idletimes()
{
    register struct hunter *temp = h_head;
    register char *watch;
    register int  i,idle,k;
    register struct uentry *thisguy;
    register struct ttyentry *ttyp;
    while(temp != NULL) {
	for(i=0;i<temp->no_hunted;i++) {
	    if((thisguy = find_user(temp->hunted[i])) == NULL) continue;
	    else {
		for(k=0;k<thisguy->no_ttys; k++) {
		    ttyp = &(thisguy->ttys[k]);
		    if((idle = ttyp->idletime) < 0) {
			ttyp->idletime = compute_idletime(ttyp->name);
			idle = ttyp->idletime;
		    }
		    if(idle > temp->idletime[i]) {
			if(temp->imagic[i][k] == 0) {
			    notify(temp->name, thisguy->name,
				   ttyp->name,IDLE);
			    temp->imagic[i][k] = 1;
			    temp->hmagic[i][k] = 0;
			}
		    }
		    else {
			if(temp->hmagic[i][k] == 0) {
			    notify(temp->name,thisguy->name,
				   ttyp->name,ACTIVE);
			    temp->hmagic[i][k] = 1;
			    temp->imagic[i][k] = 0;
			}
		    }
		}
	    }
	}
	temp = temp->next;
    }
}

find_and_notify(name, tty, opcode,idx,max)
char *name,*tty;
{
    register struct hunter *temp = h_head;
    register int i,j;
    while(temp != NULL) {
	for(i=0; i< temp->no_hunted;i++) {
	    if(!strncmp(name, temp->hunted[i], NMAX)) {
		switch(opcode) {
		  case LOGIN:
		    notify(temp->name, name, tty, opcode);
		    /* initialize imagic and hmagic flags */
		    temp->imagic[i][idx] = 0;
		    temp->hmagic[i][idx] = 1;
				
		    break;
		  case LOGOUT:	
		    notify(temp->name, name, tty, opcode);
		    for(j=i;j<max;j++) {
			temp->imagic[i][j] = temp->imagic[i][j+1];
			temp->hmagic[i][j] = temp->hmagic[i][j+1];
		    }
		    break;
		}
	    }
	    else continue;
	}
	temp = temp->next;
    }
}

/* 
 * Simple_notify takes a username and a message buffer
 * finds out all the terminals that the user is logged in and dumps
 * the message on all of those terminals 
 */
simple_notify(user, msg)
register char *user,*msg;
{
    register struct uentry *temp;
    register struct ttyentry *ttyp;
    FILE *ttyfp;
    register int i;
    char errmsg[127];
    char device[20];

    if((temp = find_user(user)) == NULL) {
	sprintf(errmsg,"fatal error inside notify?\n");
	logerror(errmsg);
	exit(1);
    }
    
    for(i=0;i<temp->no_ttys; i++) {
	ttyp = &(temp->ttys[i]);
	strcpy(device,"/dev/");
	strcat(device,ttyp->name);
	ttyfp = NULL;
	if((ttyfp = fopen(device,"w")) == NULL) {
	    sprintf(errmsg,"%s is protected?\n", device);
	    logerror(errmsg);
	    continue;
	}
	
	fprintf(ttyfp,"%s", msg);
	fflush(ttyfp);
	fclose(ttyfp);
    }
}
	
/* 
 * Notify takes an opcode and conses up a message to write to
 * the user. It uses the function above this to do the actual work
 * should we ring the bell?
 */
notify(user, hunted, tty, opcode)
char *user,*hunted,*tty;
{
    char msg[128];
    extern   char nowbuf[];
    extern   char nowtime[];

    switch(opcode) {
      case ACTIVE:
	sprintf(msg, "\r\n[alert: (%5s/%5s) %s on %s is hacking again]\n",
		nowtime, nowbuf, hunted, tty);
	break;
      case IDLE:
	sprintf(msg, "\r\n[alert: (%5s/%5s) %s on %s is now idle]\n",
		nowtime,nowbuf, hunted, tty);
	break;
      case LOGIN:
	sprintf(msg, "\r\n[alert: (%5s/%5s) %s is logging in on %s]\n",
		nowtime,nowbuf, hunted,tty);
	break;
      case LOGOUT:
	sprintf(msg, "\r\n[alert: (%5s/%5s) %s is logging out of %s]\n",
		nowtime,nowbuf, hunted, tty);
	break;
    }
    simple_notify(user,msg);
}
    
/* final util reoutine in this file */
compute_idletime(tty)
char *tty;
{
	struct stat stbuf;
	long lastaction, diff;
	char ttyname[20];
	time_t now;

	strcpy(ttyname, "/dev/");
	strcatn(ttyname, tty, NMAX);
	stat(ttyname, &stbuf);
	time(&now);
	lastaction = stbuf.st_atime;
	diff = now - lastaction;
	diff = (diff+30)/60;
	if (diff < 0) diff = 0;
	return(diff);
}
	
/* to test */
#ifdef DEBUG
print_hunters()
{
    register int i;
    struct hunter *temp = h_head;
    while(temp != NULL) {
	printf(" %8s", temp->name);
	if(temp->no_hunted != 0) printf(" is watching ");
	else printf(" is not watching anyone.");
	for(i=0; i< temp->no_hunted;i++) {
	    printf("%s ", temp->hunted[i]);
	}
	printf("\n");
	temp = temp->next;
    }
}

main()
{
    char name[20];
    register int i;
    char buf[10];
    char user[20];
    for(;;) {
	hunt();
	printf("command> ");
	gets(buf);
	switch(buf[0]) {
	  case 'a':
	    printf("Hunter: ");
	    gets(name);
	    printf("Hunted: ");
	    gets(user);
	    if((i=add_hunted(user, name, 3)) < 0) 
	      printf("Error: %s\n", h_errmsgs[h_error]);
	    else printf("Done.\n");
	    break;

	  case 'f':
	    printf("Hunter: ");
	    gets(name);
	    if(free_hunter(name)<0) printf("Free: failure\n");
	    break;

	  case 'd':
	    printf("Hunter: ");
	    gets(name);
	    printf("Hunted: ");
	    gets(user);
	    if((i = delete_hunted(user, name)) < 0)
	      printf("Error: %s\n", h_errmsgs[h_error]);
	    else printf("Done.\n");
	    break;

	  case 'p':
	    print_hunters();
	    break;
	}
    }
}

#endif /*DEBUG*/

