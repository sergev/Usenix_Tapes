#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#include "defs.h"

/*GLOBAL*/
struct time_alarm *t_head = NULL;
int    no_alarms=0;
int    t_error;
char  *t_errmsgs[] = {
    0,
    "No alarm with that id no",
    "No predated alarms please",
    "Cant delete an alarm that belongs to someone else",
    0
};


free_time_alarm(user,id,override)
char *user;
int id;
int override;
{
    struct time_alarm *temp = t_head;
    struct time_alarm *prev = NULL;
    while(temp != NULL) {
	if(id == temp->id) break;
	prev = temp;
	temp = temp->next;
    }
    if(temp == NULL) {
	t_error = 1;
	return(-1);
    }
    else {
	if((strncmp(user,temp->name,NMAX) != 0) && !override) {
	    t_error = 3;
	    return(-1);
	}

	if(temp == t_head) {
	    t_head = t_head->next;
	    free(temp);

	}
	else {
	    prev->next = temp->next;
	    free(temp);
	}
    }
    return(1);
}

struct time_alarm *
find_time_alarm(id)
int id;
{
    register struct time_alarm *temp = t_head;
    while(temp != NULL) {
	if(id == temp->id) break;
	else temp = temp->next;
    }
    return(temp);
}

add_alarm(user,at,msg)
register char *user;
time_t at;
register char *msg;
{
    
    register struct time_alarm *temp = t_head;
    register struct time_alarm *prev = NULL;
    long now;
    
    time(&now);
    
    if(at < now) {
	t_error = 2;
	return(-1);
    }

    while(temp != NULL) {
	if(at < temp->alarmtime) break;
	else {
	    prev = temp;
	    temp = temp->next;
	}
    }
    if(temp == t_head) {
	temp = (struct time_alarm *) malloc(sizeof(struct time_alarm));
	temp->next = t_head;
	temp->id = (no_alarms > MAX_ALARMS) ? 0 : no_alarms++;
	temp->alarmtime = at;
	strcpy(temp->msg,msg);
	strncpy(temp->name, user, NMAX);
	t_head = temp;
    }
    else {
	temp = (struct time_alarm *) malloc(sizeof(struct time_alarm));
	if(temp == NULL) temp->next = NULL;
	else temp->next = prev->next;
	temp->id = (no_alarms > MAX_ALARMS) ? 0 : no_alarms++;
	temp->alarmtime = at;
	strcpy(temp->msg, msg);
	strncpy(temp->name,user, NMAX);
	prev->next = temp;
    }
}

do_timealarms()
{
    register struct time_alarm *temp = t_head;
    long     now;
    extern char nowbuf[];
    char     tempbuf[150];
    char     buf[30];
    if(temp == NULL) return;
    time(&now);
    while(temp != NULL) {
	if(temp->alarmtime < now) {
	    strncpy(buf,ctime(&(temp->alarmtime)),24);
	    sprintf(tempbuf,
		    "\r\n[alert: (%s) id no: %02d, set for %s]\n", 
		    nowbuf, temp->id, buf);
	    simple_notify(temp->name,tempbuf);
	    sprintf(tempbuf,"[Msg: %s]\n", temp->msg);
	    simple_notify(temp->name,tempbuf);
	    t_head = t_head->next;
	    if(t_head == NULL) no_alarms = 0;
	    free(temp);
	}
	else break;
	temp = t_head;
    }
}

#ifdef DEBUG			     
print_alarms()
{
    register struct time_alarm *temp = t_head;
    register int no=0;
    while(temp != NULL) {
	printf("%d Alarm at %s", temp->id, ctime(&(temp->alarmtime)));
	printf("\tFor user %8s saying %s\n",
	       temp->name, temp->msg);
	temp = temp->next;
	no++;
    }
    if(no == 0) printf("No alarms set.\n");
}

main()
{
    char buf[10];
    char msg[80];
    char user[20];
    char typ;int ret,del;
    long n;
    time(&n);
    printf("Time is now: %ld\n",n);
    for(;;) {
	printf("Alarm? ");
	gets(buf);
	switch(buf[0]) {
	  case 'a':
	    printf("User:  ");
	    gets(user);
	    printf("Time: ");
	    scanf("%ld", &n); getchar();
	    printf("Msg: ");
	    gets(msg);
	    if((ret = add_alarm(user,n,msg)) < 0) 
	      printf("Error: %s\n", t_errmsgs[t_error]);
	    break;
	    
	  case 'p':
	    print_alarms();
	    break;
	  case 'd':
	    printf("Id of alarm to delete: ");
	    scanf("%d", &del);
	    if((ret = free_time_alarm(del)) < 0) 
	      printf("Error: %s\n", t_errmsgs[t_error]);
	    break;
	  case 'e':
	    do_timealarms();
	    break;
	}
    }
}
#endif
		       
    
    
