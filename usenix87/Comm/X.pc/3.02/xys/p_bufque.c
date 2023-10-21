/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
		   X.PC Buffer and Queue Management Routines
			      p_bufque.c
   The routines in this module are used to manage linked lists of bufferlets
   and queue entries.  Routines referring to 'PROPER LISTs' require a list of
   1 or more entries, where the last entry's 'next' pointer is NULL.

			  Generic Queue Mgmt.
			  -------------------
   enque	accepts a queue header pointer and a queue entry pointer.  It
		appends the entry to the queue pointed to by the header.
   deque	accepts a queue header pointer and returns a pointer or NULL.
		A NULL value says: no entries available, non-NULL is firt entry.
   ins_list	accepts a queue header pointer, and a queue entry pointer that
		is assumed to head a PROPER LIST.  The list is put at the FRONT
		of the queue controlled by the queue header.
   last_que	accepts a 'from' que head pointer, finds the last queue entry.
		Returns the last entry of the 'from' list to the caller.

		       Available Bufferlet Pool *
		       --------------------------
   get_buff	returns a pointer to a bufferlet, or NULL.  A NULL pointer
		indicates no bufferlets available.  A non-NULL pointer points
		to the first bufferlet found, and has a 'bnext' pointer of NULL.
   rel_buff	accepts a bufferlet entry and puts that entry back into the
		available bufferlet pool, at the end of the queue.
   rel_blist	accepts a bufferlet pointer, assumes it points to a PROPER
		LIST, and adds the list to the available bufferlet pool.

			  Available Queue Pool *
			  --------------------
   get_que	returns a pointer to a queue entry from the available pool, or,
		returns NULL (no queue entries available).  The pointer is to
		the first available queue entry.  The entry's 'qnext' value is
		initialized to NULL.  If routine is called with 'buff_also'
		set to FALSE, no other fields are initialized.  If called with
		'buff_also' set TRUE, a bufferlet is retrieved with a call to
		get_buff.  If get_buff returns NULL, this routine returns NULL;
		otherwise the address returned is put into 'u.cmd.bfirst' field.
   rel_que	accepts a queue entry pointer and puts that queue entry back
		into the queue pool, at the end of the queue.  If the buff_also
		parameter is TRUE and 'u.cmd.bfirst' field is non-NULL, the
		PROPER LIST that is ASSUMED to exist is released as well.
   rel_qlist	accepts a queue entry pointer, assumes it points to the start
		of a PROPER LIST, and puts the list of entries back into the
		available queue entry pool.  If the BUFF_ALSO parameter is TRUE
		and 'u.cmd.bfirst' field is non-NULL, the buffer PROPER LIST that
		is ASSUMED to exist, for each queue entry, is released as well.
   rel_qhead	accepts a que header pointer, returns all to the available que.

   * NOTE that all of the available bufferlet/queue routines check to see if
	  the # of entries remaining are within the thresholds in 'p_config.d'.
   ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
	Date  Change  By      Reason
      06/12/84   01   curt    Initial generation of code, from 'x_bufque.c'.
      07/22/84   02   curt    Added 'low_rsrc', to return lcb.xpc_nrdy flag.
      08/06/84   03   curt    Add clear 'qnext' to deque
      08/09/84   04   mike    get_que clears queue-entry fields after bfirst
      10/03/84   05   curt    Changed size of QHEAD structure in 'p_bufque.s'.
      10/12/84   06   curt    Added 'rel_qhead' subroutine.
      09/04/86   07?  bobc    Rid B_RETRAN in cmd, due to
			      pack_start(retrans.)
      09/26/86   08   Ed M    Reformat and recomment
   ------------------------------------------------------------------------- */

#include "driver.h"
#include "lcbccb.h"

extern  BUF_ENTR buffpool[];
extern  QUE_ENTR quepool[];



/* -------------------------------------------------------------------------
   This routine merely returns the lcb's "xpc not ready" flag, which is non-
   zero when there is a shortage of queue entries or bufferlet entries.  It
   is a useful routine for those modules that don't have access to the lcb.
   ------------------------------------------------------------------------- */
int	low_rsrc ()
{
    return ((lcb.xpc_nrdy != 0) ? TRUE : FALSE);
}



/* -------------------------------------------------------------------------
   This routine accepts a pointer to a que head, release all que entries.
   ------------------------------------------------------------------------- */
void rel_qhead (qhead)
QHEADPTR qhead;
{

    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */

    if (qhead -> ent_cnt != 0)
	rel_qlist (qhead -> first, YES_BUFF);
    qhead -> first = qhead -> last = NULL;
    qhead -> ent_cnt = 0;
    return;
}



/* -------------------------------------------------------------------------
   This routine adds the queue entry at 'qentry' to the end of the list whose
   header is pointed to by 'qhead'.  No checks are made as to pointer validity.
   ------------------------------------------------------------------------- */
void enque (qhead, qentry)
QHEADPTR qhead;
QUE_PTR qentry;
{


    /*
     Die if interrupts are enabled or invalid queue pointer.
     */
    if (is_enb ())
	abort ();
    if (((int) qentry < (int)&buffpool[0])
    ||((int) qentry > (int) & quepool[QUEPLCNT - 1]))
	abort ();

    if (qhead -> ent_cnt != 0)
	qhead -> last -> qnext = qentry;
    else
	qhead -> first = qentry;

    qhead -> last = qentry;
    qentry -> qnext = NULL;
    ++(qhead -> ent_cnt);
    return;
}



/* -------------------------------------------------------------------------
   This routine takes the first entry from the 'qhead' pointed to, and returns
   the pointer to the first entry.  If no entries are present, returns NULL.
   ------------------------------------------------------------------------- */
QUE_PTR deque (qhead)
QHEADPTR qhead;
{
    QUE_PTR qentry;


    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */

    if (qhead -> ent_cnt != 0) {
	qentry = qhead -> first;

	if (((int) qentry < (int) & buffpool[0])
		||((int) qentry > (int) & quepool[QUEPLCNT - 1]))
	    abort ();

	qhead -> first = qentry -> qnext;
	--(qhead -> ent_cnt);
	if (!qhead -> ent_cnt)
	    qhead -> last = NULL;
	qentry -> qnext = NULL;
    }
    else
	qentry = NULL;
    return (qentry);
}



/* -------------------------------------------------------------------------
   This routine accepts a pointer to a queue header, and a pointer to the
   first entry in a list of queue entries (1 or more, NULL terminated).
   It inserts the queue list in the FRONT of the first que entry of the header.
   This routine MUST NOT be called for releasing to the available pools.
   ------------------------------------------------------------------------- */
void ins_list (qhead, qptr)
QHEADPTR qhead;
QUE_PTR qptr;
{
    QUE_PTR tque, prev, curr;
 /*
  Save off the first entry of the old list.  Make the first entry of the new
  list the new 'first' for the queue header.  Loop through new list to find
  end-of-list and count entries.  Once find last entry (the one with a NULL
  next pointer), continue on to fix up 'last' and connect new to old.
  */

    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */

    tque = qhead -> first;
    qhead -> first = curr = qptr;
    do {

	if (((int) curr < (int) & quepool[0])
		||((int) curr > (int) & quepool[QUEPLCNT - 1]))
	    abort ();
	/*
	 chg.07 bobc note:  next is a must since pack_start would re-use
	 old P(S) and thus leave an extra P(S) available.
	 */
	curr -> u.cmd.b_state = (curr -> u.cmd.b_state & ~(B_RETRAN));

	++qhead -> ent_cnt;
	prev = curr;
	curr = curr -> qnext;
    } while (curr);
 /*
  When tque is NULL, qhead->first was NULL (list empty).  In this case,
  must set up new last since created list.  When tque is not NULL, there
  was a list so need to point end of inserted list to start of old list.
  Since tque is NULL for empty list and qptr->qnext NULL, no need to assign
  old first to end.
  */
    if (tque == NULL)
	qhead -> last = prev;
    else {
	prev -> qnext = tque;

	if (((int) tque < (int) & quepool[0])
		||((int) tque > (int) & quepool[QUEPLCNT - 1]))
	    abort ();

    }
    return;
}



/* -------------------------------------------------------------------------
   This routine is to find the last entry in the 'from_head' (if there is one)
   and return it to the caller. Returns NULL if no entries in queue.
   ------------------------------------------------------------------------- */
QUE_PTR last_que (from_head)
QHEADPTR from_head;
{
    QUE_PTR prev, curr;


    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */

    curr = NULL;		/* return null if empty, drop out. */
    if (from_head -> ent_cnt != 0) {
	curr = from_head -> first;
	if (from_head -> ent_cnt == 1) {
	    from_head -> first = NULL;
	    from_head -> last = NULL;
	}
	else {
	    while (curr -> qnext != NULL) {

		if (((int) curr < (int) & quepool[0])
			||((int) curr > (int) & quepool[QUEPLCNT - 1]))
		    abort ();

		prev = curr;
		curr = prev -> qnext;
	    }
	    prev -> qnext = NULL;
	    from_head -> last = prev;
	}
	from_head -> ent_cnt--;
    }
    return (curr);
}



/* -------------------------------------------------------------------------
   This routine returns either NULL or a pointer to a bufferlet list entry.
   It returns NULL when there are no remaining entries.  The counter of bytes
   in the bufferlet is reset to zero, and its 'next' pointer is set to NULL.
   ------------------------------------------------------------------------- */
BUF_PTR get_buff ()
{
    BUF_PTR tbuff;
    extern  BHEAD avail_buff;
 /*
  Using the generic queue manipulation routine 'deque', get a bufferlet
  from the available queue.  Use type cast to fake out the compiler.
  If deque doesn't have a bufferlet, return the NULL value it returns.
  After deque, see if count is below LOW threshold: if so, set LCB flag.
  */

    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */

    tbuff = (BUF_PTR) deque ((QHEADPTR) & avail_buff);
    if (tbuff != NULL) {
	tbuff -> num_bytes = 0;
	if (avail_buff.ent_cnt <= BLO_THRES)
	    lcb.xpc_nrdy |= BUF_LOW;
    }
    return (tbuff);
}



/* -------------------------------------------------------------------------
   This routine accepts a bufferlet pointer and returns that bufferlet to
   the available bufferlet pool.  If the number of available bufferlets is
   greater than the high threshold, turn off BUF_LOW flag in LCB.
   ------------------------------------------------------------------------- */
void rel_buff (bentry)
BUF_PTR bentry;
{
    extern BHEAD avail_buff;

    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */
    if (((int) bentry < (int) & buffpool[0])
	    ||((int) bentry > (int) & buffpool[BUFPLCNT - 1]))
	abort ();

    enque ((QHEADPTR) & avail_buff, (QUE_PTR) bentry);
    if (avail_buff.ent_cnt >= BHI_THRES)
	lcb.xpc_nrdy &= ~BUF_LOW;
    return;
}



/* -------------------------------------------------------------------------
   This routine assumes that it is given a pointer to the first entry of a
   proper list of bufferlets.  It releases all of the entries in the list
   to the available bufferlet pool.  If the number of entries is greater than
   the high threshold, the BUF_LOW flag in LCB is set to zero.
   ------------------------------------------------------------------------- */
void rel_blist (first_buf)
BUF_PTR first_buf;
{
    BUF_PTR prev, curr;
    extern BHEAD avail_buff;

    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */

    curr = first_buf;
    if (avail_buff.ent_cnt != 0)
	avail_buff.last -> bnext = curr;
    else
	avail_buff.first = curr;
    do {

	if (((int) curr < (int) & buffpool[0])
		||((int) curr > (int) & buffpool[BUFPLCNT - 1]))
	    abort ();

	++(avail_buff.ent_cnt);
	prev = curr;
	curr = curr -> bnext;
    } while (curr);
    avail_buff.last = prev;
    if (avail_buff.ent_cnt >= BHI_THRES)
	lcb.xpc_nrdy &= ~BUF_LOW;
    return;
}



/* -------------------------------------------------------------------------
   This routine returns either NULL or a pointer to a queuelist entry.  It
   returns NULL when there are no remaining entries.  If buff_also = TRUE,
   a bufferlet is requested from get_buff.  If get_buff returns NULL, this
   routine returns NULL.  If get_buff returns non-NULL, the 'first buffer'
   pointer (in a 'command que entry' structure) is set to the value returned.
   If buff_also is FALSE, no fields are changed except 'next que' getting NULL.
   If entry count after deque is below LOW threshold: set LCB flag indicating.
   ------------------------------------------------------------------------- */
QUE_PTR get_que (buff_also)
int     buff_also;
{
    extern QHEAD avail_que;
    QUE_PTR tque;


    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */

    tque = deque (&avail_que);
    if (tque != NULL) {
	if (buff_also == TRUE) {
	    if ((tque -> u.cmd.bfirst = get_buff ()) == NULL) {
		enque (&avail_que, tque);
		tque = NULL;
		/* No threshold check, didn't affect counts of entries. */
	    }
	}
	else
	    tque -> u.cmd.bfirst = NULL;

	if (tque != NULL) {
	    tque -> qnext = NULL;
	    if (avail_que.ent_cnt <= QLO_THRES)
		lcb.xpc_nrdy |= QUE_LOW;
	    tque -> u.cmd.b_state = tque -> u.cmd.p_gfilci = 0;
	    tque -> u.cmd.p_seqenc = tque -> u.cmd.p_id = 0;
	}
    }
    return (tque);
}



/* -------------------------------------------------------------------------
   This routine accepts a que entry pointer and adds it to the end of the
   queue list available queue.  No return value is provided.  If the field
   'cmd.bfirst is non-NULL, the bufferlet list presumed to be pointed at
   is released to the available bufferlet list.
   ------------------------------------------------------------------------- */
void rel_que (qentry, buff_also)
QUE_PTR qentry;
int     buff_also;
{
    extern QHEAD avail_que;

    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */
    if (((int) qentry < (int) & quepool[0])
	    ||((int) qentry > (int) & quepool[QUEPLCNT - 1]))
	abort ();

    if (qentry -> u.cmd.bfirst)
	rel_blist (qentry -> u.cmd.bfirst);
    enque (&avail_que, qentry);
    if (avail_que.ent_cnt >= QHI_THRES)
	lcb.xpc_nrdy &= ~QUE_LOW;
    return;
}



/* -------------------------------------------------------------------------
   This routine assumes that it is given a pointer to the first entry of a
   proper list of queue entries.  It releases each entry in the list to the
   available queue pool.  If the buff_also parameter is TRUE, and if any of
   the queue entries has a 'cmd.bfirst' field that is non-NULL, the bufferlet
   list pointed (ASSUMED) to is released as well.
   ------------------------------------------------------------------------- */
void rel_qlist (first_que, buff_also)
QUE_PTR first_que;
int     buff_also;
{
    extern QHEAD avail_que;
    QUE_PTR prev, curr;
    curr = first_que;

    if (is_enb ())
	abort ();		/* abort, if interrupts are enabled */

    if (avail_que.ent_cnt != 0)
	avail_que.last -> qnext = curr;
    else
	avail_que.first = curr;
    do {

	if (((int) curr < (int) & quepool[0])
		||((int) curr > (int) & quepool[QUEPLCNT - 1]))
	    abort ();

	if (curr -> u.cmd.bfirst)
	    rel_blist (curr -> u.cmd.bfirst);
	++(avail_que.ent_cnt);
	prev = curr;
	curr = curr -> qnext;
    } while (curr);
    avail_que.last = prev;
    if (avail_que.ent_cnt >= QHI_THRES)
	lcb.xpc_nrdy &= ~QUE_LOW;
    return;
}
