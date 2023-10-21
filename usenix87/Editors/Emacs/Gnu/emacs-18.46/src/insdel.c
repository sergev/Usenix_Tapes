/* Buffer insertion/deletion and gap motion for GNU Emacs.
   Copyright (C) 1985, 1986 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the GNU Emacs General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
GNU Emacs, but only under the conditions described in the
GNU Emacs General Public License.   A copy of this license is
supposed to have been given to you along with GNU Emacs so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */


#include "config.h"
#include "lisp.h"
#include "buffer.h"
#include "window.h"

/* Move gap to position `pos'.
   Note that this can quit!  */

move_gap (pos)
     int pos;
{
  if (bf_p2 != bf_gap + bf_p1)
    abort ();

  if (pos <= bf_s1)
    gap_left (pos);
  else if (pos > bf_s1 + 1)
    gap_right (pos);
}

gap_left (pos)
     register int pos;
{
  register unsigned char *to, *from;
  register int i;
  int new_s1;

  pos--;

  if (unchanged_modified == bf_modified)
    {
      beg_unchanged = pos;
      end_unchanged = bf_s1 + bf_s2 - pos;
    }
  else
    {
      if (bf_s2 < end_unchanged)
	end_unchanged = bf_s2;
      if (pos < beg_unchanged)
	beg_unchanged = pos;
    }

  i = bf_s1 + 1;
  to = &bf_p2[i];
  from = &bf_p1[i];
  new_s1 = bf_s1;

  /* Now copy the characters.  To move the gap down,
     copy characters up.  */

  while (1)
    {
      /* I gets number of characters left to copy.  */
      i = new_s1 - pos;
      if (i == 0)
	break;
      /* If a quit is requested, stop copying now.
	 Change POS to be where we have actually moved the gap to.  */
      if (QUITP)
	{
	  pos = new_s1;
	  break;
	}
      /* Move at most 32000 chars before checking again for a quit.  */
      if (i > 32000)
	i = 32000;
      new_s1 -= i;
      while (--i >= 0)
	*--to = *--from;
    }

  /* Adjust markers, and buffer data structure, to put the gap at POS.
     POS is where the loop above stopped, which may be what was specified
     or may be where a quit was detected.  */
  adjust_markers (pos + 1, bf_s1 + 1, bf_gap);
  bf_s2 += bf_s1 - pos;
  bf_s1 = pos;
  QUIT;
}

gap_right (pos)
     register int pos;
{
  register unsigned char *to, *from;
  register int i;
  int new_s1;

  pos--;

  if (unchanged_modified == bf_modified)
    {
      beg_unchanged = pos;
      end_unchanged = bf_s1 + bf_s2 - pos;
    }
  else
    {
      if (bf_s1 + bf_s2 - pos < end_unchanged)
	end_unchanged = bf_s1 + bf_s2 - pos;
      if (bf_s1 < beg_unchanged)
	beg_unchanged = bf_s1;
    }

  i = bf_s1 + 1;
  from = &bf_p2[i];
  to = &bf_p1[i];
  new_s1 = bf_s1;

  /* Now copy the characters.  To move the gap up,
     copy characters down.  */

  while (1)
    {
      /* I gets number of characters left to copy.  */
      i = pos - new_s1;
      if (i == 0)
	break;
      /* If a quit is requested, stop copying now.
	 Change POS to be where we have actually moved the gap to.  */
      if (QUITP)
	{
	  pos = new_s1;
	  break;
	}
      /* Move at most 32000 chars before checking again for a quit.  */
      if (i > 32000)
	i = 32000;
      new_s1 += i;
      while (--i >= 0)
	*to++ = *from++;
    }

  adjust_markers (bf_s1 + bf_gap + 1, pos + bf_gap + 1, - bf_gap);
  bf_s2 += bf_s1 - pos;
  bf_s1 = pos;
  QUIT;
}

/* Add `amount' to the position of every marker in the current buffer
   whose current position is between `from' (exclusive) and `to' (inclusive).
   Also, any markers past the outside of that interval, in the direction
   of adjustment, are first moved back to the near end of the interval
   and then adjusted by `amount'.  */

adjust_markers (from, to, amount)
     register int from, to, amount;
{
  Lisp_Object marker;
  register struct Lisp_Marker *m;
  register int mpos;

  marker = bf_cur->markers;

  while (!NULL (marker))
    {
      m = XMARKER (marker);
      mpos = m->bufpos;
      if (amount > 0)
	{
	  if (mpos > to && mpos < to + amount)
	    mpos = to + amount;
	}
      else
	{
	  if (mpos > from + amount && mpos <= from)
	    mpos = from + amount;
	}
      if (mpos > from && mpos <= to)
	mpos += amount;
      if (m->bufpos != mpos)
	m->bufpos = mpos, m->modified++;
      marker = m->chain;
    }
}

/* make sure that the gap in the current buffer is at least k
   characters wide */

make_gap (k)
     int k;
{
  register unsigned char *p1, *p2, *lim;

  if (bf_gap >= k)
    return;

  k += 2000;			/* Get more than just enough */

  p1 = (unsigned char *) realloc (bf_p1 + 1, bf_s1 + bf_s2 + k);
  if (p1 == 0)
    memory_full ();

  k -= bf_gap;			/* Amount of increase.  */

  /* Record new location of text */
  bf_p1 = p1 - 1;

  /* Transfer the new free space from the end to the gap
     by shifting the second segment upward */
  p2 = bf_p1 + 1 + bf_s1 + bf_s2 + bf_gap;
  p1 = p2 + k;
  lim = p2 - bf_s2;
  while (lim < p2)
    *--p1 = *--p2;

  /* Finish updating text location data */
  bf_gap += k;
  bf_p2 = bf_p1 + bf_gap;

  /* Don't wait for next SetBfp; make it permanent now. */
  bf_cur->text = bf_text;

  /* adjust markers */
  adjust_markers (bf_s1 + 1, bf_s1 + bf_s2 + bf_gap + 1, k);
}

/* Insert the character c before point */

insert_char (c)
     unsigned char c;
{
  InsCStr (&c, 1);
}

/* Insert the null-terminated string s before point */

InsStr (s)
     char *s;
{
  InsCStr (s, strlen (s));
}

/* Insert a string of specified length before point */

InsCStr (string, length)
     register unsigned char *string;
     register length;
{
  if (length<1)
    return;

  prepare_to_modify_buffer ();

  if (point != bf_s1 + 1)
    move_gap (point);
  if (bf_gap < length)
    make_gap (length);

  record_insert (point, length);
  bf_modified++;

  bcopy (string, bf_p1 + point, length);

  bf_gap -= length;
  bf_p2 -= length;
  bf_s1 += length;
  point += length;
}

/* like InsCStr except that all markers pointing at the place where
   the insertion happens are adjusted to point after it.  */

insert_before_markers (string, length)
     unsigned char *string;
     register int length;
{
  register int opoint = point;
  InsCStr (string, length);
  adjust_markers (opoint - 1, opoint, length);
}

/* Delete characters in current buffer
  from `from' up to (but not incl) `to' */

del_range (from, to)
     register int from, to;
{
  register int numdel;

  /* Make args be valid */
  if (from < FirstCharacter)
    from = FirstCharacter;
  if (to > NumCharacters)
    to = NumCharacters + 1;

  if ((numdel = to - from) <= 0)
    return;

  /* Make sure the gap is somewhere in or next to what we are deleting */
  if (from - 1 > bf_s1)
    gap_right (from);
  if (to - 1 < bf_s1)
    gap_left (to);

  prepare_to_modify_buffer ();
  record_delete (from, numdel);
  bf_modified++;

  /* Relocate point as if it were a marker.  */
  if (from < point)
    {
      if (point < to)
	point = from;
      else
	point -= numdel;
    }

  /* Relocate all markers pointing into the new, larger gap
     to point at the end of the text before the gap.  */
  adjust_markers (to + bf_gap, to + bf_gap, - numdel - bf_gap);

  bf_gap += numdel;
  bf_p2 += numdel;
  bf_s2 -= to - 1 - bf_s1;
  bf_s1 = from - 1;

  if (bf_s1 < beg_unchanged)
    beg_unchanged = bf_s1;
  if (bf_s2 < end_unchanged)
    end_unchanged = bf_s2;
}

modify_region (start, end)
     int start, end;
{
  prepare_to_modify_buffer ();
  if (start - 1 < beg_unchanged || unchanged_modified == bf_modified)
    beg_unchanged = start - 1;
  if (bf_s1 + bf_s2 + 1 - end < end_unchanged
      || unchanged_modified == bf_modified)
    end_unchanged = bf_s1 + bf_s2 + 1 - end;
  bf_modified++;
}

prepare_to_modify_buffer ()
{
  if (!NULL (bf_cur->read_only))
    Fbarf_if_buffer_read_only();

#ifdef CLASH_DETECTION
  if (!NULL (bf_cur->filename)
      && bf_cur->save_modified >= bf_modified)
    lock_file (bf_cur->filename);
#endif /* CLASH_DETECTION */
}
