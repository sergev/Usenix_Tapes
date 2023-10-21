/* -------------------------------------------------------------------------
		      Timer Queue Entry Processing Routines
			    filename = p_timque.c

   The routines in this module provide access to the Timer Queue.  Callers
   may start a timer, or end a timer.  Also in this module is the routine
   that occurs when a Timeout Event is indicated by the Timer Interrupt.

   set_time    accepts a timer interval, a subroutine address, and an
	       argument for the subroutine when invoked.  An entry is added
	       to the Timer Queue, such that when the interval is up, the
	       routine will be executed with the indicated parameter.
	       The timevent routine decrements the first timer queue entry
	       in terms of 18 ticks per second.  The caller expresses time
	       in terms of 6 time events per second.  Thus a caller wanting
	       a 1 second interval will assign (1*EVENT_PSEC).  This routine
	       converts that into (TICKS_PER_EVENT*(1*EVENT_PSEC)).  Routine
	       will return FALSE if it can't get a que entry for caller.
   end_time    accepts a subroutine address and a parameter value, and will
	       remove from the Timer Queue all entries that have a matching
	       address/parameter value.  The end_time routine will return
	       FALSE if it cannot find a matching que entry in the timer que.
	       If the subroutine address is 0, all entries matching the
	       'parameter' field will be deleted.
   end_cmd     accepts a queue header pointer, and a logical channel #.  The
	       queue list pointed to by the header is searched for entries
	       with a matching logical channel.  Those entries found are
	       deleted from the queue.  The routine returns FALSE if no
	       entries can be found.
   fid_cmd     accepts a queue header pointer, channel and a packet id #.
	       Queue list pointed to by the header is searched for entries
	       with a matching logical channel, and packet ID (if > 0).
	       The entry found is removed from the queue and returned.
	       The routine returns FALSE if no entries can be found.
   timevent    is called by the ASM routine that is invoked each timer tick.
	       The ASM routine compares a counter to the value it has for
	       TICKS_PER_EVENT, calling timevent when the counter = zero.
   -------------------------------------------------------------------------
	Date    Change   By   Reason
      01/28/84    00    curt  Initial generation of code.
      04/06/84    01    curt  Added end_cmd function to delete command entries.
      06/07/84    02    mike  Initial generation of 'fid_cmd' routine.
      06/14/84    03    curt  Initial conversion to PAD version of code.
      06/18/84    04    mike  end_time must not change prev when deleting curr
      11/29/84    05    curt  rewrite: ticks/event, events/second changes.
      09/26/86    06    Ed M  Reformat and recomment
   ------------------------------------------------------------------------- */
#include "driver.h"
#include "lcbccb.h"
#define     TICKS_PER_EVENT  3  /* 1 second = 18 ticks = 6 time events */
extern  QUE_ENTR quepool[];


/* -------------------------------------------------------------------------
   This routine puts a timer entry into the timer queue. The intrval is in
   terms of 'time events'.  Must convert to 'timer ticks'.  Formula is:
   (TICKS_PER_EVENT * (EVENTS_PSEC * intrval)).  Other arguments are put in
   the entry, no action is taken with them otherwise.  It returns TRUE if it
   can put the entry into the timer queue, or FALSE if not queue entries can
   be used from the available list.
   ------------------------------------------------------------------------- */
int     set_time (intrval, fnc_addr, fncparam)
int     intrval;
fncptr fnc_addr;
int     fncparam;
{
    extern QUE_PTR get_que();

    int     tsum;               /* running total of timeout entries.     */
    QUE_PTR tque,               /* entry to be added to list.            */
	curr,                   /* entry currently pointed at.           */
	prev;                   /* entry pointing to current entry.      */

    if (is_enb () || !fnc_addr || intrval < 1)
	abort ();               /* intr enabled or NOT valid params */

    tque = get_que (NO_BUFF);
    if (tque == NULL)
	return (FALSE);
    tque -> u.time.routine = fnc_addr;
    tque -> u.time.param = fncparam;

    intrval *= TICKS_PER_EVENT;
    tsum = 0;
    prev = NULL;
    curr = lcb.time_que.first;
 /* 
  Look through the list until pointing at NULL or until total units is less
  than or equal to interval desired.  When find pointing at NULL, did not
  find an entry that would timeout after the new one, so will add at end
  of list.      When find (sum + current) becomes more than interval, put new
  entry in front of current, that is, insert new in front of current because
  new has shorter timeout. NOTE if empty list, curr and prev both equal NULL.
  */
    while (curr != NULL) {
	if ((tsum + curr -> u.time.units) == intrval)
	    ++intrval;  
	if ((tsum + curr -> u.time.units) > intrval)
	    break;
	else {
	    tsum += curr -> u.time.units;
	    prev = curr;
	    curr = curr -> qnext;
	}
    }
 /* 
  Either hit curr=NULL, or found entry to insert in front of.  When loop goes
  through at least once, prev!=NULL, so the thing to do is insert the new
  entry between the two queue entries.  When looped zero times (curr=NULL,
  list is empty; or, first's timeout greater than new's), add to list front.
  */
    if (prev != NULL) { 
	tque -> qnext = prev -> qnext;
	prev -> qnext = tque;
    }
    else {
	lcb.time_que.first = tque;
	tque -> qnext = curr;
    }

 /* 
  Set up units as (timeout desired less timeout total to this entry).  The
  following entry's units must be decremented by the units of the new entry,
  to show how long after new times out that following entry should timeout.
  Only decrement following when new entry not added at the end of the list.
  */
    tque -> u.time.units = intrval - tsum;
    ++(lcb.time_que.ent_cnt);
    if (tque -> qnext == NULL)
	lcb.time_que.last = tque;
    else
	tque -> qnext -> u.time.units -= tque -> u.time.units;

    return (TRUE);
}



/* -------------------------------------------------------------------------
   This routine accepts a function address, and a parameter for the function.
   It searches the timer_que for an entry with matching values.  If it finds a
   matching entry(s), the entries are removed from the timer_que, TRUE returned.
   If the 'fnc_addr' parameter is non-NULL, the 1st matching entry is deleted.
   If the 'fnc_addr' parameter is NULL, all entries matching the 'fnc_param'
   value are deleted. If no matching entries are found, FALSE is returned.
   ------------------------------------------------------------------------- */
int     end_time (fnc_addr, fnc_param)
fncptr  fnc_addr;
int     fnc_param;
{
    extern void rel_que();

    QUE_PTR curr, prev, temp;
    int     ret_value;

    ret_value = FALSE;

    if (is_enb ())
	abort ();               /* abort, if interrupts are enabled */

    if (lcb.time_que.ent_cnt != 0) {
	curr = lcb.time_que.first;
	prev = NULL;
	/* 
	 Stay in loop until match/delete, or until curr is NULL (all checked).
	 */
	while (curr != NULL) {
	    if ((curr -> u.time.param == fnc_param)
		    && ((curr -> u.time.routine == fnc_addr)
		    || (fnc_addr == NULL))) {
		/* 
		 If prev is non-NULL, removing from within the list.  If prev is
		 NULL, removing the first list entry.  Point around old entry in
		 all cases.  If curr->qnext is NULL, removing last entry of the
		 list.  If curr->qnext is non-NULL, increment following entry's
		 timer 'delta' units (because that entry has to wait longer than
		 before did delete).
		 */
		--(lcb.time_que.ent_cnt);
		if (prev != NULL)
		    prev -> qnext = curr -> qnext;
		else
		    lcb.time_que.first = curr -> qnext;
		if (curr -> qnext == NULL)
		    lcb.time_que.last = prev;
		else
		    curr -> qnext -> u.time.units += curr -> u.time.units;
	    /* MIKE SAYS NEVER CHANGE PREV         */
		curr -> u.time.units = 0;/* MIKE SAYS ZERO FOR REL_QUE */
		temp = curr;    
		curr = curr -> qnext;
		rel_que (temp, NO_BUFF);
		ret_value = TRUE;
		if (fnc_addr != NULL)
		    break;
	    }
	    else {
		prev = curr;
		curr = curr -> qnext;
	    }
	}
    }
    return (ret_value);
}



/* -------------------------------------------------------------------------
   This routine will examine the queue list controlled by the queue header
   given it, looking for queue entries that have a logical channel # matching
   the log_chan parameter and a que p_id that is a lower priority than pktid.
   When an entry is found, it is deleted from the list, and TRUE is returned.
   When no entries are found, FALSE is returned.
   ------------------------------------------------------------------------- */
int     end_cmd (qhead, log_chan, pktid)
	QHEADPTR qhead;
int     log_chan,
	pktid;
{
#define RSETR_PKT 0x1B          /* will never change */
#define RSETC_PKT 0x1F          /* same as p_pktlyr.d */

    extern void rel_que();
    QUE_PTR curr, prev, temp;
    int     ret_value,
	    qpkt;


    if (is_enb ())
	abort ();               /* abort, if interrupts are enabled */

    ret_value = FALSE;
    if (qhead -> ent_cnt != 0) {
	curr = qhead -> first;
	prev = NULL;
	/* 
	 Stay in loop until match/delete, or until curr is NULL (all checked).
	 Restart erases packets < RSTRR_PKT, Reset erases packets < RSETR_PKT,
	 but Clear (pktid < 0) erases packets < CALLR_PKT or = RSETR_PKT
	 or = RSETC_PKT
	 */
	while (curr != NULL) {

	    if (((int) curr < (int) & quepool[0])
		    ||((int) curr > (int) & quepool[QUEPLCNT - 1]))
		abort ();

	    qpkt = curr -> u.cmd.p_id & 0xDF;
	    if (((curr -> u.cmd.p_gfilci & P_LCI) == log_chan)
		    && (((curr -> u.cmd.p_gfilci & C_BIT) == 0) ||
			(((pktid > 0) && (qpkt < pktid)) ||
			    ((pktid < 0) &&
				((qpkt < -pktid) || (qpkt == RSETR_PKT)
				    || (qpkt == RSETC_PKT)))))) {
		/*
		 If prev is non-NULL, remove from within the list.  If prev is
		 NULL, remove the first list entry.  Point around old entry in
		 all cases. If curr->qnext is NULL, removing last entry of the
		 list.
		 */
		--(qhead -> ent_cnt);
		if (prev != NULL)
		    prev -> qnext = curr -> qnext;
		else
		    qhead -> first = curr -> qnext;
		if (curr -> qnext == NULL)
		    qhead -> last = prev;

		temp = curr;    
		curr = curr -> qnext;
		rel_que (temp, YES_BUFF);
		ret_value = TRUE;
	    }
	    else {
		prev = curr;
		curr = curr -> qnext;
	    }
	}
    }
    return (ret_value);
}



/* -------------------------------------------------------------------------
   This routine will examine the queue list controlled by the queue header
   given it, looking for queue entries that have a logical channel # matching
   the log_chan parameter, and a packet ID # matching the pktid parameter.
   When an entry is found, it is deleted from the list, and returned to the
   caller.  When no entries are found, FALSE is returned.
   ------------------------------------------------------------------------- */
QUE_PTR fid_cmd (qhead, log_chan, pktid)
QHEADPTR qhead;
int     log_chan,
	pktid;
{
    QUE_PTR curr, prev, ret_value;


    if (is_enb ())
	abort ();               /* abort, if interrupts are enabled */

    ret_value = NULL;           /* default return, says not found */
    if (qhead -> ent_cnt != 0) {
	curr = qhead -> first;
	prev = NULL;
	/* 
	 Stay in loop until match/delete, or curr is NULL (item found, or end).
	 */
	do {

	    if (((int) curr < (int) & quepool[0])
		    ||((int) curr > (int) & quepool[QUEPLCNT - 1]))
		abort ();

	    if (((curr -> u.cmd.p_gfilci & P_LCI) == log_chan)
		    && ((pktid == 0) || (((curr -> u.cmd.p_gfilci & C_BIT) == C_BIT)
			    && (curr -> u.cmd.p_id == pktid)))) {
		/* 
		 If prev is non-NULL, removing from within the list.  If prev is
		 NULL, removing the first list entry.  Point around old entry in
		 all cases. If curr->qnext is NULL, removing last entry of the
		 list.
		 */
		if (prev != NULL)
		    prev -> qnext = curr -> qnext;
		else
		    qhead -> first = curr -> qnext;
		if (curr -> qnext == NULL)
		    qhead -> last = prev;

		--(qhead -> ent_cnt);
		curr -> qnext = NULL;
		ret_value = curr;
		curr = NULL;
	    }
	    else {
		prev = curr;
		curr = curr -> qnext;
	    }
	}
	while (curr);
    }
    return (ret_value);
}



/* -------------------------------------------------------------------------
   This routine is called by the Timer_tick interrupt-driven routine, each
   time a DOS Timer Tick occurs.  It looks at the first timer que entry (if
   there is one), and runs it if its delta count has become zero.  All counts
   are in terms of 1 second = 18 ticks, 'set_time' converts the 6 per second
   'time events' per second into the 18 'timer ticks' per second.
   INTERRUPTS ARE ENABLED, thru the magic of stack manipulation in timer_tick
   ------------------------------------------------------------------------- */
void timevent ()
{
    extern QUE_PTR deque();
    extern void    rel_que();

    fncptr troutine;            /* hold routine address temporarily */
    int     tparam;
 /*
  Decrement the event counter of the first entry.  If result is still more
  than zero, first event hasn't timed out.  When result is less than or
  equal to zero (may be less than when have two zero-valued entries in a row),
  the first entry time has expired, and needs to be serviced.
  */
    int_off ();
    if ((lcb.time_que.ent_cnt != 0)
	    && (--(lcb.time_que.first -> u.time.units) <= 0)) {
	 /*
	  Move fields needed out of the entry and release it.  This may help the
	  routine called have access to queue entries if it needs them. The last
	  line calls the routine in the variable 'troutine', and passes as the
	  only argument the contents of 'tparam'.
	  */
	lcb.time_que.first -> u.time.units = 0;
	troutine = lcb.time_que.first -> u.time.routine;
	tparam = lcb.time_que.first -> u.time.param;
	rel_que (deque (&lcb.time_que), NO_BUFF);
	int_on ();
	if (troutine)
	    (*troutine) (tparam);
	else
	    abort ();
    }
    int_on ();
}
