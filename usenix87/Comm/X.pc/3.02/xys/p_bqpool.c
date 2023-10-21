/*****************************************************************************
*     This program is the sole property and confidential information of      *
*     Tymnet, Inc., and may not be copied in whole or part, or disclosed     *
*     to any third party, without the written prior consent of Tymnet.       *
*****************************************************************************/
/* -------------------------------------------------------------------------
	     X.PC Bufferlet and Queue Pool Initialization Routines
			  filename = p_bqpool.c

   The routines in this module initialize the data areas pointed to by the
   arrays 'buffpool' and 'quepool' into linked lists.  Each entry in the list
   points to a following entry, except the last entry which points to NULL.
   Also included are routines to zero the memory area used for the pools.

			      Module contents
			      ---------------

   mak_bufpool    creates a linked list of bufferlets, leaving the address of
		  the first entry in 'buffpool' in the 'avail_buff.first' list
		  header variable, and the 'avail_buff.ent_cnt' set to the
		  number of entries declared by the array declaration.

   mak_quepool    creates a linked list of queue entries, leaving the address
		  of the first entry in 'quepool' in the 'avail_que.first' list
		  header variable, and the 'avail_que.ent_cnt' set to the
		  number of entries declared by the array declaration.
   -------------------------------------------------------------------------
	Date  Change  By      Reason
      05/27/84   01   curt    Initial Generation of Code.
      11/13/84   02   curt    Consolidate routines into 'make' routines only.
      09/26/86   03   Ed M    Reformat and recomment
   ------------------------------------------------------------------------- */

#include "driver.h"

/* -------------------------------------------------------------------------
   The arrays are used to allocate the memory needed (at compile time). Each
   list has a 'header', which is used to control and manage the lists.  The
   array structures should be hidden from users of the available bufferlet and
   queue entry management routines in 'x_bufque.c'.  The bufferlet/queue list
   headers may be referenced through the declarations in 'x_bqpool.e'.
   ------------------------------------------------------------------------- */

BUF_ENTR buffpool[BUFPLCNT];
QUE_ENTR quepool[QUEPLCNT];

BHEAD avail_buff;
QHEAD avail_que;



/* -------------------------------------------------------------------------
   This routine puts zeros in all of the memory in the bufferlet pool area,
   then it creates a linked list of bufferlets.  It uses the area allocated
   by the 'buffpool' array declaration, and pointed to by the first element
   of the array.  Assume BUFPLCNT is > 0.
   INTERRUPTS MUST BE OFF!
   ------------------------------------------------------------------------- */
void mak_bufpool ()
{
    BUF_PTR pool_addr;

    pool_addr = &buffpool[0];
    zero_mem (pool_addr, (sizeof (BUF_ENTR) * BUFPLCNT));
    avail_buff.siz_chk = sizeof (BUF_ENTR);
    avail_buff.first = pool_addr;
    avail_buff.last = NULL;
    avail_buff.ent_cnt = 0;
    while (avail_buff.ent_cnt < BUFPLCNT) {
	if (avail_buff.last != NULL)
	    avail_buff.last -> bnext = pool_addr;
	avail_buff.last = pool_addr;

	++avail_buff.ent_cnt;
	++pool_addr;
    }
    avail_buff.last -> bnext = NULL;
    return;
}



/* -------------------------------------------------------------------------
   This routine puts zeros in all of the queue pool data area, then it creates
   a linked list of queue entries to build the pool. It uses the area that is
   allocated by the 'quepool' array declaration, and pointed to by the first
   entry of the array.  Assume QUEPLCNT is > 0.
   INTERRUPTS MUST BE OFF!
   ------------------------------------------------------------------------- */
void mak_quepool () {
    QUE_PTR pool_addr;

    pool_addr = &quepool[0];
    zero_mem (pool_addr, (sizeof (QUE_ENTR) * QUEPLCNT));
    avail_que.siz_chk = sizeof (QUE_ENTR);
    avail_que.first = pool_addr;
    avail_que.last = NULL;
    avail_que.ent_cnt = 0;
    while (avail_que.ent_cnt < QUEPLCNT) {
	if (avail_que.last != NULL)
	    avail_que.last -> qnext = pool_addr;
	avail_que.last = pool_addr;

	++avail_que.ent_cnt;
	++pool_addr;
    }
    avail_que.last -> qnext = NULL;
    return;
}
