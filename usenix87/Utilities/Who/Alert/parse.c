/* 
 * parse.c parse user requests
 * 
 */
#include <stdio.h>
#include <signal.h>
#include "defs.h"

extern int daemon_granularity;
extern FILE *logfp;
extern FILE *sockin;
extern FILE *sockout;
extern char nowbuf[];

extern char *h_errmsgs[];
extern int  h_error;
extern char *t_errmsgs[];
extern int  t_error;
extern int  onalarm();

char errmsg[128];

#define PROTECT()                sigblock(1 << (SIGALRM - 1))
#define UNPROTECT()              sigsetmask(sigblock(0) & ~(1 <<(SIGALRM-1)))

parse_requests()
{
    unsigned char request;
    char buf[512];
    register char *temp = buf;
    char user[20];
    char msg[128];
    char *msgptr;
    char hunt[20];
    long time;
    int  quit = 0;
    int  itime = 0;
    int  newrate,retcode;
    
    while(!quit) {
	temp = buf;
	request = getc(sockin);
	fgets(buf, 512, sockin);

	switch(request) {
	  case CONTROL_QUIT_INTERACTIVE:
	    fprintf(sockout, "%s", buf);
	    fflush(sockout);
	    quit = 1;
	    break;

	  case CONTROL_QUIT:
	    quit = 1;
	    break; 
	    
	  case CONTROL_TUNE:
	    sscanf(temp, "%s", hunt);
	    temp += strlen(hunt);
	    sscanf(temp, "%d", &newrate);

	    sprintf(errmsg,"(%s) %s changed clk rate to %d secs\n",
		    nowbuf,user,newrate);
	    logerror(errmsg);
	    fprintf(sockout,"Setting new clock rate to %d secs.\n",
		    newrate);
	    fflush(sockout);
	    daemon_granularity = newrate;
	    break;

	  case SHOW_TIME_ALARMS:
	    sscanf(temp, "%s", user);
	    PROTECT();
	    print_alarms(user,0);
	    UNPROTECT();
	    break;

	  case SHOW_ALL_ALARMS:
	    sscanf(temp, "%s", user);
	    PROTECT();
	    print_alarms(user,1);
	    UNPROTECT();
	    break;

	  case ADD_TIME_ALARM:
	    msgptr = msg;
	    sscanf(temp, "%s", user);
	    temp += strlen(user)+1;
	    sscanf(temp, "%ld", &time);
	    while((*msgptr = fgetc(sockin)) != '\0') 
	      msgptr++;
	    PROTECT();
	    if((retcode = add_alarm(user,time,msg)) < 0)
	      fprintf(sockout, "Error: %s\n", t_errmsgs[t_error]);
	    else 
	      fprintf(sockout, "Added alarm for %s\n", user);
	    fflush(sockout);
	    UNPROTECT();
	    sprintf(errmsg,"(%s) %s added a time alarm\n",
		    nowbuf,user);
	    logerror(errmsg);

	    break;
		
	    
	  case DELETE_TIME_ALARM:
	    sscanf(temp, "%s", user);
	    temp += strlen(user)+1;
	    itime = -1;
	    sscanf(temp, "%d", &itime);

	    PROTECT();
	    if((retcode = free_time_alarm(user,itime,0)) < 0)
	      fprintf(sockout, "Error: %s\n", t_errmsgs[t_error]);
	    else
	      fprintf(sockout, "Deleted alarm %d\n", itime);
	    fflush(sockout);
	    UNPROTECT();
	    break;

	  case DELETE_ANY_ALARM:
	    sscanf(temp, "%s", user);
	    temp += strlen(user)+1;
	    itime = -1;
	    sscanf(temp, "%d", &itime);

	    PROTECT();
	    if((retcode = free_time_alarm(user,itime,1)) < 0)
	      fprintf(sockout, "Error: %s\n", t_errmsgs[t_error]);
	    else
	      fprintf(sockout, "Deleted alarm %d\n", itime);
	    fflush(sockout);
	    UNPROTECT();
	    break;
		

	  case FLUSH_USER:
	    sscanf(temp, "%s", hunt);
	    temp += strlen(hunt);
	    sscanf(temp, "%s", user);
	    temp += strlen(user);

	    PROTECT();
	    if((retcode = free_hunter(hunt)) < 0) 
	      fprintf(sockout,"Error: %s\n", h_errmsgs[h_error]);
	    else 
	      fprintf(sockout, "Flushed user %s\n", hunt);
	    fflush(sockout);
	    UNPROTECT();

	    if(retcode > 0) {
		sprintf(errmsg,"(%s) %s flushed %s\n",
			nowbuf,user,hunt);
		logerror(errmsg);
	    }
	    break;


	  case ADD_USER:
	    sscanf(temp,"%s", hunt);
	    temp += strlen(hunt);
	    sscanf(temp,"%s", user);
	    temp += strlen(user)+1;
	    sscanf(temp, "%d", &itime);
	    if(itime == 0) itime = 3;

	    PROTECT();
	    if((retcode = add_hunted(hunt,user,itime)) < 0) 
	      fprintf(sockout,"Error: %s\n", h_errmsgs[h_error]);
	    else 
	      fprintf(sockout,"Added %s for %s(%d)\n", hunt,user,itime);
	    fflush(sockout);
	    UNPROTECT();

	    sprintf(errmsg,"(%s) %s started watching %s(%d)\n",
		    nowbuf,user,hunt, itime);
	    logerror(errmsg);
	    break;

	  case DELETE_USER:
	    sscanf(temp,"%s", hunt);
	    temp += strlen(hunt);
	    sscanf(temp,"%s", user);
	    PROTECT();
	    if((retcode = delete_hunted(hunt,user)) < 0)
	      fprintf(sockout, "Error: %s\n", h_errmsgs[h_error]);
	    else
	      fprintf(sockout, "Deleted %s for %s\n", hunt, user);
	    fflush(sockout);
	    UNPROTECT();
	    sprintf(errmsg,"(%s) %s stopped watching %s\n",
		    nowbuf,user,hunt);
	    logerror(errmsg);
	    break;

	  case CONTROL_STATUS:
	    sscanf(temp,"%s", user);
	    PROTECT();
	    short_printout(user);
	    UNPROTECT();
	    sprintf(errmsg,"(%s) %s requested short status\n",
		    nowbuf,user);
	    logerror(errmsg);

	    break;

	  case OVERALL_STATUS:
	    sscanf(temp,"%s", user);
	    PROTECT();
	    printout(user);
	    UNPROTECT();
	    sprintf(errmsg,"(%s) WHOPR %s requested overall status\n",
		    nowbuf,user);
	    logerror(errmsg);

	    break;

	  default:
	    sprintf(errmsg,"Unk.Req. %o (oct).Buffer(%20s)\n", request,
		    buf);
	    logerror(errmsg);
	    quit =1;
	    break;
	}
    }
}

printout(user)
char *user;
{
    fprintf(sockout, "Last update at: %s, Daemon Rate: %d secs.\n",
	    nowbuf,daemon_granularity);
    fflush(sockout);
    print_utab();
    print_hunters(user,1);
    fprintf(sockout, "QUIT\n");
    fflush(sockout);
}

short_printout(user)
char *user;
{
    print_utab();
    print_hunters(user,0);
    fprintf(sockout, "QUIT\n");
    fflush(sockout);
}

extern struct uentry *u_first;

print_utab()
{
    register struct ttyentry *i;
    register int j,k;
    register struct uentry *temp = u_first;
    fprintf(sockout, "Login status: \n");
    while(temp != NULL) {
	fprintf(sockout, " %8s is logged in at ", temp->name);
	for(j=0;j < temp->no_ttys;j++) {
	    i = &(temp->ttys[j]);
	    fprintf(sockout, "%s ", i->name);
	}
	temp = temp->next;
	fprintf(sockout, "\n");
    }
    fprintf(sockout, "\n");
    fflush(sockout);
}

extern struct hunter *h_head;
extern struct hunter *h_tail;
print_hunters(user,allp)
char *user;
int allp;
{
    register int i,j=0;
    struct hunter *temp = h_head;
    fprintf(sockout, "Userwatch Status:\n");
    fflush(sockout);
    while(temp != NULL) {
	if(allp || (!strncmp(user,temp->name))) {
	    fprintf(sockout," %8s", temp->name);
	    if(temp->no_hunted != 0) fprintf(sockout," is watching ");
	    else fprintf(sockout," is not watching anyone.");
	    for(i=0; i< temp->no_hunted;i++) {
		fprintf(sockout,"%8s(%d) ", temp->hunted[i], 
			temp->idletime[i]);
	    }
	    fprintf(sockout,"\n");
	    j++;
	}
	temp = temp->next;
    }
    if(j == 0) fprintf(sockout, " No one is spying on anyone\n");
    fprintf(sockout,"\n");
    fflush(sockout);
}

extern struct time_alarm *t_head;
print_alarms(user,allp)
char *user;
int allp;
{
    register int no=0;
    register struct time_alarm *temp = t_head;
    while(temp != NULL) {
	if((!strncmp(user,temp->name,NMAX)) || allp) {
	    if(allp) fprintf(sockout,"User: %8s,",temp->name);
	    fprintf(sockout,"Alarm Id: %02d, Set for: %s",
		    temp->id, ctime(&temp->alarmtime));
	    fprintf(sockout,"\tMessage: %s\n",
		    temp->msg);
	    no++;
	}
	temp = temp->next;
    }
    if(no == 0) fprintf(sockout, "No alarms set\n");
    fflush(sockout);
    fprintf(sockout, "QUIT\n");
    fflush(sockout);
}

