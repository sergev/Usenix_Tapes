#ifdef PUCC
#include <sys/types.h>
#include <rld.h>
#endif PUCC
#include <sys/file.h>
#include "untamo.h"
#include "y.tab.h"

extern char *malloc(), *strcpy(), *ctime();
extern strlen();
time_t time();
long lseek();

/*
 * addlist -- adds a record to the list "list".  Uses the 
 *	      insque(3) call for speed.
 *	      list -- which list to add the rule to (rules or exempt)
 *	      type -- what kind of rule (LOGIN, GROUP, etc)
 *	      name -- who it applies to (ajw, console, etc)
 *	      num --  who it applies to (1505 (rld), 5 (group staff) )
 *	      flag -- idle time for a "rule" rule, or exemption type
 *		      for "exempt" rule: IDLE, MULTIPLE, etc.
 */
addlist(list, type, name, num, flag)
struct qelem *list;
char *name;
int type, flag, num;
{
	struct qelem *new_node;
	struct qelem *ptr;
	struct item  *new_data;

	/* 
	 * make all the new structures
	 */
	new_node = (struct qelem *) malloc( sizeof(struct qelem) );
	new_data = (struct item  *) malloc( sizeof(struct item ) );
	new_data->name_t = type;
	new_data->name = name;
	new_data->num = num;
	new_data->flag = flag;
	new_node->q_item = new_data; 
	/*
	 * find where to insert it in the list
	 */
	ptr = list->q_forw;
	while (ptr != list)	{
		if (ptr->q_item->name_t <= new_data->name_t)
			break;
		ptr = ptr->q_forw;
	}
	/*
	 * and insert it
	 */
	(void) insque(new_node,ptr->q_back);
}


/*
 * freelist -- frees up the space in the list pointed to 
 *	       by ptr.  Uses free.
 */
freelist(ptr)
struct qelem *ptr;
{
	struct qelem *dead;
	struct qelem *start;

	start = ptr;
	ptr = ptr->q_forw;
	while ( ptr != start )   {
		dead = ptr;
		ptr = ptr->q_forw;
		(void) free( (char *) dead->q_item);  	/* kill the data */
		(void) free( (char *) dead );		/* now get the node */
	}
	start->q_forw = start;		/* reset pointers for a null list */
	start->q_back = start;
}


/*
 * setlimits -- looks through the rules list and uses the most
 *		specific rule for users[i], then looks through
 *		the exceptions and uses the (again) most specific
 *		exemption for the user.
 */

setlimits(i)
int i;
{
	struct qelem *ptr;
	int rule;
	time_t tempus;

	/*
	 * look down the rules list and set the
	 * most specific rule that applies to 
	 * this user
	 */
	rule = 0;
	(void) time(&tempus);
	ptr = rules->q_back;		/* start at the end of the list */
	users[i].warned = 0;		/* clear his warning and exempt flag */
	users[i].exempt = 0;		/* to avoid granularity problems */

	users[i].next = tempus;		/* next time is set to now, so */
					/* the new rules (or exemptions) */
					/* take affect immediately */
	/* 
	 * while we haven't looked through the whole list,
	 * and we haven't found a rule...
	 */
	while ( (ptr != rules) && (!rule) )	{
		if ( (rule = find(ptr, i)) )  {
			users[i].idle = (ptr->q_item)->flag * 60;
			users[i].exempt &= ~IS_IDLE;
		} else
			ptr = ptr->q_back;		/* move back one */
	}

	rule = 0;
	ptr = session->q_back;
	while ( (ptr != session) && (!rule) ){
		if ( (rule = find(ptr, i)) )  {
			users[i].session = (ptr->q_item)->flag * 60;
			users[i].exempt &= ~IS_LIMIT;
		} else
			ptr = ptr->q_back;		/* move back one */
	}
	/*
	 * now look down the exemptions list and
	 * set the first exemptions for this user.
	 */
	rule = 0;
	ptr = exmpt->q_back;		/* start at the end of the list */
	while ( (ptr != exmpt) && (!rule) )	{
		if ( (rule = find(ptr, i)) )  {
			if ( (ptr->q_item)->flag == ALL)
				users[i].exempt |= IS_IDLE|IS_MULT|IS_LIMIT;
			else if ( (ptr->q_item)->flag == IDLE)
				users[i].exempt |= IS_IDLE;
			else if ( (ptr->q_item)->flag == MULTIPLE)
				users[i].exempt |= IS_MULT;
			else if ( (ptr->q_item)->flag == SESSION)
				users[i].exempt |= IS_LIMIT;
		} else
			ptr = ptr->q_back;		/* move back one */
	}
}


/*
 * find -- given a rule (or exemption) and a users, see if it applies.
 *	   Example: If the rule is a LOGIN type rule, compare the name with that
 *	   of the user to see if it applies.  If we find a match, return 1,
 *	   else return 0.
 */
find(ptr,i)
int i;
struct qelem *ptr;
{
	switch ( (ptr->q_item)->name_t )	{
	case LOGIN: 
		if (!strcmp( (ptr->q_item)->name, users[i].uid) )
			return(1);
		break;
	case TTY:
		if (!strcmp( (ptr->q_item)->name, (users[i].line+5) ) ) 
			return(1);
		break;
	case GROUP:
		if ( (ptr->q_item)->num == users[i].ugroup ) 
			return(1);
		break;
#ifdef PUCC
	case CLUSTER:
		if (!strcmp( (ptr->q_item)->name, users[i].clust) )
			return(1);
		break;
	case RLD:
		if ( (ptr->q_item)->num == users[i].rld ) 
			return(1);
		break;
#endif PUCC
	case DEFAULT:
		return(1);
		break;
	}
	return(0);
}

#ifdef PUCC

#define makestr(Z)	(strcpy(malloc( (unsigned) strlen(Z)+1),Z))

/*
 * findcluster -- passes an rld number, it returns a char *
 *		  to the name of the cluster associated with it
 */
char *
findcluster(rld)
int rld;
{
	int fd;
	static struct rld_data data;

#ifdef BSD2_9
	if ( (fd = open(RLD_FILE,O_RDONLY) ) < 0 )  {
#else BSD2_9
	if ( (fd = open(RLD_FILE,O_RDONLY,0) ) < 0 )  {
#endif BSD2_9
		(void) error("Can't open rld file.");
		exit(1);
	}

	/* 
	 * seek to the proper rld and read it.
	 */
	(void) lseek(fd, rld*sizeof(data), 0);

	(void) read(fd, (char *) &data, sizeof(data) );

	(void) close(fd);

	/*
	 * return the cluster name, if null, return "public"
	 */
	if (data.rld_cluster)
		return(makestr(data.rld_cluster));
	else
		return(makestr("public"));
}
#endif PUCC
