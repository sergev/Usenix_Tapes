/* Buffer manipulation primitives for GNU Emacs.
   Copyright (C) 1985, 1986, 1987 Free Software Foundation, Inc.

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


#include <sys/param.h>

#ifndef MAXPATHLEN
/* in 4.1, param.h fails to define this. */
#define MAXPATHLEN 1024
#endif /* not MAXPATHLEN */

#ifdef NULL
#undef NULL
#endif
#include "config.h"
#include "lisp.h"
#include "window.h"
#include "commands.h"
#include "buffer.h"
#include "syntax.h"

struct buffer *bf_cur;		/* the current buffer */

/* This structure contains data describing the text of the current buffer.
 Switching buffers swaps their text data in and out of here */

struct buffer_text bf_text;

/* First buffer in chain of all buffers (in reverse order of creation).
   Threaded through ->next.  */

struct buffer *all_buffers;

/* This structure holds the default values of the buffer-local variables
   defined with DEFVAR_PER_BUFFER, that have special slots in each buffer.
   The default value occupies the same slot in this structure
   as an individual buffer's value occupies in that buffer.
   Setting the default value also goes through the alist of buffers
   and stores into each buffer that does not say it has a local value.  */

struct buffer buffer_defaults;
/* A Lisp_Object pointer to the above, used for staticpro */
static Lisp_Object Vbuffer_defaults;

/* This structure marks which slots in a buffer have corresponding
   default values in buffer_defaults.
   Each such slot has a nonzero value in this structure.
   The value has only one nonzero bit.

   When a buffer has its own local value for a slot,
   the bit for that slot (found in the same slot in this structure)
   is turned on in the buffer's local_var_flags slot.

   If a slot in this structure is negative, then even though there may
   be a DEFVAR_PER_BUFFER for the slot, there is no default value for it;
   and the corresponding slot in buffer_defaults is not used.

   If a slot in this structure corresponding to a DEFVAR_PER_BUFFER is
   zero, that is a bug */

struct buffer buffer_local_flags;

/* This structure holds the names of symbols whose values may be
   buffer-local.  It is indexed and accessed in the same way as the above. */

struct buffer buffer_local_symbols;
/* A Lisp_Object pointer to the above, used for staticpro */
static Lisp_Object Vbuffer_local_symbols;

Lisp_Object Fset_buffer ();

/* Alist of all buffer names vs the buffers. */
/* This used to be a variable, but is no longer,
 to prevent lossage due to user rplac'ing this alist or its elements.  */
Lisp_Object Vbuffer_alist;

Lisp_Object Qfundamental_mode, Qmode_class;

Lisp_Object QSFundamental;	/* A string "Fundamental" */

/* For debugging; temporary.  See SetBfp.  */
/* Lisp_Object Qlisp_mode, Vcheck_symbol; */

nsberror (spec)
     Lisp_Object spec;
{
  if (XTYPE (spec) == Lisp_String)
    error ("No buffer named %s", XSTRING (spec)->data);
  error ("Invalid buffer argument");
}

DEFUN ("buffer-list", Fbuffer_list, Sbuffer_list, 0, 0, 0,
  "Return a list of all buffers.")
  ()
{
  return Fmapcar (Qcdr, Vbuffer_alist);
}

DEFUN ("get-buffer", Fget_buffer, Sget_buffer, 1, 1, 0,
  "Return the buffer named NAME (a string).\n\
It is found by looking up NAME in  buffer-alist.\n\
If there is no buffer named NAME, nil is returned.\n\
NAME may also be a buffer; it is returned.")
  (name)
     register Lisp_Object name;
{
  if (XTYPE (name) == Lisp_Buffer)
    return name;
  CHECK_STRING (name, 0);

  return Fcdr (Fassoc (name, Vbuffer_alist));
}

DEFUN ("get-file-buffer", Fget_file_buffer, Sget_file_buffer, 1, 1, 0,
  "Return the buffer visiting file FILENAME (a string).\n\
If there is no such buffer, nil is returned.")
  (filename)
     register Lisp_Object filename;
{
  register Lisp_Object tail, buf, tem;
  CHECK_STRING (filename, 0);
  filename = Fexpand_file_name (filename, Qnil);

  for (tail = Vbuffer_alist; CONSP (tail); tail = XCONS (tail)->cdr)
    {
      buf = Fcdr (XCONS (tail)->car);
      if (XTYPE (buf) != Lisp_Buffer) continue;
      if (XTYPE (XBUFFER (buf)->filename) != Lisp_String) continue;
      tem = Fstring_equal (XBUFFER (buf)->filename, filename);
      if (!NULL (tem))
	return buf;
    }
  return Qnil;
}

/* Incremented for each buffer created, to assign the buffer number. */
int buffer_count;

DEFUN ("get-buffer-create", Fget_buffer_create, Sget_buffer_create, 1, 1, 0,
  "Like get-buffer but creates a buffer named NAME and returns it if none already exists.")
  (name)
     register Lisp_Object name;
{
  register Lisp_Object buf, function, tem;
  int count = specpdl_ptr - specpdl;
  register struct buffer *b;
  /* register struct buffer *bx; */
  register unsigned char *data;

  buf = Fget_buffer (name);
  if (!NULL (buf)) return buf;

  b = (struct buffer *) malloc (sizeof (struct buffer));
  if (!b) memory_full ();

  data = (unsigned char *) malloc (b->text.gap = 20);
  if (!data) memory_full ();
  b->text.p1 = data - 1;
  b->text.p2 = data - 1 + b->text.gap;
  b->text.size1 = b->text.size2 = 0;
  b->text.modified = 1;
  b->text.pointloc = 1;
  b->text.head_clip = 1;
  b->text.tail_clip = 0;

  b->next = all_buffers;
  all_buffers = b;

  b->save_length = make_number (0);
  b->last_window_start = 1;
  b->markers = Qnil;
  b->mark = Fmake_marker ();
  /*b->number = make_number (++buffer_count);*/
  b->name = name;
  if (XSTRING (name)->data[0] != ' ')
    make_undo_records (b);
  else
    b->undodata = 0;

  reset_buffer (b);

  XSET (buf, Lisp_Buffer, b);
#if 0
  XSETTYPE (buf, Lisp_Buffer);
  bx = b;			/* Use of bx avoids compiler bug on Sun */
  XSETBUFFER (buf, bx);
#endif
  Vbuffer_alist = nconc2 (Vbuffer_alist, Fcons (Fcons (name, buf), Qnil));

  function = buffer_defaults.major_mode;
  if (NULL (function))
    {
      tem = Fget (bf_cur->major_mode, Qmode_class);
      if (!EQ (tem, Qnil))
	function = bf_cur->major_mode;
    }

  if (NULL (function) || EQ (function, Qfundamental_mode))
    return buf;

  /* To select a nonfundamental mode,
     select the buffer temporarily and then call the mode function. */

  record_unwind_protect (save_excursion_restore, save_excursion_save ());

  Fset_buffer (buf);
  call0 (function);

  unbind_to (count);
  return buf;
}

void
reset_buffer (b)
     register struct buffer *b;
{
  b->filename = Qnil;
  b->directory = (bf_cur) ? bf_cur->directory : Qnil;
  b->modtime = 0;
  b->save_modified = 1;
  b->backed_up = Qnil;
  b->auto_save_modified = 0;
  b->auto_save_file_name = Qnil;
  b->read_only = Qnil;
  reset_buffer_local_variables(b);
}

reset_buffer_local_variables(b)
     register struct buffer *b;
{
  register int offset;

  /* Reset the major mode to Fundamental, together with all the
     things that depend on the major mode.
     default-major-mode is handled at a higher level.
     We ignore it here.  */
  b->major_mode = Qfundamental_mode;
  b->keymap = Qnil;
  b->abbrev_table = Vfundamental_mode_abbrev_table;
  b->mode_name = QSFundamental;
  b->minor_modes = Qnil;
  b->syntax_table_v = XVECTOR (Vstandard_syntax_table);

  /* Reset all per-buffer variables to their defaults.  */
  b->local_var_alist = Qnil;
  b->local_var_flags = 0;

  /* For each slot that has a default value,
     copy that into the slot.  */

  for (offset = (char *)&buffer_local_flags.name - (char *)&buffer_local_flags;
       offset < sizeof (struct buffer);
       offset += sizeof (Lisp_Object)) /* sizeof int == sizeof Lisp_Object */
    if (*(int *)(offset + (char *) &buffer_local_flags) > 0)
      *(Lisp_Object *)(offset + (char *)b) =
		*(Lisp_Object *)(offset + (char *)&buffer_defaults);
}

DEFUN ("generate-new-buffer", Fgenerate_new_buffer, Sgenerate_new_buffer,
  1, 1, 0,
  "Creates and returns a buffer named NAME if one does not already exist,\n\
else tries adding successive suffixes to NAME until a new buffer-name is\n\
formed, then creates and returns a new buffer with that new name.")
 (name)
     register Lisp_Object name;
{
  register Lisp_Object gentemp, tem;
  int count;
  char number[10];

  CHECK_STRING (name, 0);

  tem = Fget_buffer (name);
  if (NULL (tem))
    return Fget_buffer_create (name);

  count = 1;
  while (1)
    {
      sprintf (number, "<%d>", ++count);
      gentemp = concat2 (name, build_string (number));
      tem = Fget_buffer (gentemp);
      if (NULL (tem))
	return Fget_buffer_create (gentemp);
    }
}


DEFUN ("buffer-name", Fbuffer_name, Sbuffer_name, 0, 1, 0,
  "Return the name of BUFFER, as a string.\n\
No arg means return name of current buffer.")
  (buffer)
     register Lisp_Object buffer;
{
  if (NULL (buffer))
    return bf_cur->name;
  CHECK_BUFFER (buffer, 0);
  return XBUFFER (buffer)->name;
}

#ifdef NOTDEF /* Useless. If you need this, you should be using `eq'
  DEFUN ("buffer-number", Fbuffer_number, Sbuffer_number, 0, 1, 0,
    "Return the number of BUFFER.\n\
  No arg means return number of current buffer.")
    (buffer)
       Lisp_Object buffer;
  {
    if (NULL (buffer))
      return bf_cur->number;
    CHECK_BUFFER (buffer, 0);
    return XBUFFER (buffer)->number;
  }
 */
#endif NOTDEF

DEFUN ("buffer-file-name", Fbuffer_file_name, Sbuffer_file_name, 0, 1, 0,
  "Return name of file BUFFER is visiting, or NIL if none.\n\
No argument means use current buffer as BUFFER.")
  (buffer)
     register Lisp_Object buffer;
{
  if (NULL (buffer))
    return bf_cur->filename;
  CHECK_BUFFER (buffer, 0);
  return XBUFFER (buffer)->filename;
}

DEFUN ("buffer-local-variables", Fbuffer_local_variables,
  Sbuffer_local_variables, 0, 1, 0,
  "Return alist of variables that are buffer-local in BUFFER.\n\
No argument means use current buffer as BUFFER.\n\
Each element of the value looks like (SYMBOL . VALUE).\n\
Note that storing new VALUEs in these elements\n\
does not change the local values.")
  (buffer)
     register Lisp_Object buffer;
{
  register struct buffer *buf;
  register Lisp_Object val;

  if (NULL (buffer))
    buf = bf_cur;
  else
    {
      CHECK_BUFFER (buffer, 0);
      buf = XBUFFER (buffer);
    }

  val = Fcopy_alist (buf->local_var_alist);
  if (buf == bf_cur)
    {
      /* get latest values from `cache'  (would be updated on setbuffer) */
      register Lisp_Object tem;
      for (tem = val; CONSP (tem); tem = XCONS (tem)->cdr)
	XCONS (XCONS (tem)->car)->cdr =
			Fsymbol_value (XCONS (XCONS (tem)->car)->car);
    }
  {
    register int offset, mask;

    for (offset = (char *)&buffer_local_symbols.name - (char *)&buffer_local_symbols;
	 offset < sizeof (struct buffer);
	 offset += (sizeof (int))) /* sizeof int == sizeof Lisp_Object */
      {
	mask = *(int *)(offset + (char *) &buffer_local_flags);
	if (mask < 0 || (buf->local_var_flags & mask))
	  val = Fcons (Fcons (*(Lisp_Object *)(offset + (char *)&buffer_local_symbols),
			      *(Lisp_Object *)(offset + (char *)buf)),
		       val);
      }
  }
  return (val);
}


DEFUN ("buffer-modified-p", Fbuffer_modified_p, Sbuffer_modified_p,
  0, 1, 0,
  "Return t if BUFFER is modified since file last read in or saved.\n\
No argument means use current buffer as BUFFER.")
  (buffer)
     register Lisp_Object buffer;
{
  register struct buffer *buf;
  if (NULL (buffer))
    buf = bf_cur;
  else
    {
      CHECK_BUFFER (buffer, 0);
      buf = XBUFFER (buffer);
    }

  bf_cur->text.modified = bf_modified;
  return buf->save_modified < buf->text.modified ? Qt : Qnil;
}

DEFUN ("set-buffer-modified-p", Fset_buffer_modified_p, Sset_buffer_modified_p,
  1, 1, 0,
  "Mark current buffer as modified or unmodified according to FLAG.")
  (flag)
     register Lisp_Object flag;
{
  register int already;
  register Lisp_Object fn;

#ifdef CLASH_DETECTION
  /* If buffer becoming modified, lock the file.
     If buffer becoming unmodified, unlock the file.  */

  fn = bf_cur->filename;
  if (!NULL (fn))
    {
      already = bf_cur->save_modified < bf_modified;
      if (!already && !NULL (flag))
	lock_file (fn);
      else if (already && NULL (flag))
	unlock_file (fn);
    }
#endif /* CLASH_DETECTION */

  bf_cur->save_modified = NULL (flag) ? bf_modified : 0;
  RedoModes++;
  return flag;
}

/* Return number of modified buffers that exist now. */

int
ModExist ()
{
  register Lisp_Object tail, buf;
  register struct buffer *b;
  register int modcount = 0;

  bf_cur->text.modified = bf_modified;

  for (tail = Vbuffer_alist; !NULL (tail); tail = Fcdr (tail))
    {
      buf = Fcdr (Fcar (tail));
      b = XBUFFER (buf);
      if (!NULL (b->filename) && b->save_modified < b->text.modified)
	modcount++;
    }

  return modcount;
}

DEFUN ("rename-buffer", Frename_buffer, Srename_buffer, 1, 1,
       "sRename buffer (to new name): ",
  "Change current buffer's name to NEWNAME (a string).")
  (name)
     register Lisp_Object name;
{
  register Lisp_Object tem, buf;

  CHECK_STRING (name, 0);
  tem = Fget_buffer (name);
  if (!NULL (tem))
    error("Buffer \"%s\" already exists", XSTRING (name)->data);

  bf_cur->name = name;
  XSET (buf, Lisp_Buffer, bf_cur);
  Fsetcar (Frassq (buf, Vbuffer_alist), name);
  if (NULL (bf_cur->filename) && !NULL (bf_cur->auto_save_file_name))
    call0 (intern ("rename-auto-save-file"));
  return Qnil;
}

DEFUN ("other-buffer", Fother_buffer, Sother_buffer, 0, 1, 0,
  "Return most recently selected buffer other than BUFFER.\n\
Buffers not visible in windows are preferred to visible buffers.\n\
If no other exists, the buffer *scratch* is returned.\n\
If BUFFER is omitted or nil, some interesting buffer is returned.")
  (buffer)
     register Lisp_Object buffer;
{
  register Lisp_Object tail, buf, notsogood, tem;
  notsogood = Qnil;

  for (tail = Vbuffer_alist; !NULL (tail); tail = Fcdr (tail))
    {
      buf = Fcdr (Fcar (tail));
      if (EQ (buf, buffer))
	continue;
      if (XSTRING (XBUFFER (buf)->name)->data[0] == ' ')
	continue;
      tem = Fget_buffer_window (buf);
      if (NULL (tem))
	return buf;
      if (NULL (notsogood))
	notsogood = buf;
    }
  if (!NULL (notsogood))
    return notsogood;
  return Fget_buffer_create (build_string ("*scratch*"));
}

DEFUN ("buffer-flush-undo", Fbuffer_flush_undo, Sbuffer_flush_undo, 1, 1, 0,
  "Make BUFFER stop keeping undo information.")
  (buf)
     register Lisp_Object buf;
{
  CHECK_BUFFER (buf, 0);
  if (XBUFFER (buf)->undodata)
    free_undo_records (XBUFFER (buf));
  XBUFFER (buf)->undodata = 0;
  return Qnil;
}

DEFUN ("buffer-enable-undo", Fbuffer_enable_undo, Sbuffer_enable_undo,
       0, 1, "",
  "Start keeping undo information for buffer BUFFER (default is current buffer).")
  (buf)
     register Lisp_Object buf;
{
  register struct buffer *b;
  register Lisp_Object buf1;

  if (NULL (buf))
    b = bf_cur;
  else
    {
      buf1 = Fget_buffer (buf);
      if (NULL (buf1)) nsberror (buf);
      b = XBUFFER (buf1);
    }
  if (b->undodata == 0)
    make_undo_records (b);
  return Qnil;
}

DEFUN ("kill-buffer", Fkill_buffer, Skill_buffer, 1, 1, "bKill buffer: ",
  "One arg, a string or a buffer.  Get rid of the specified buffer.")
  (bufname)
     register Lisp_Object bufname;
{
  register Lisp_Object buf;
  register struct buffer *b;
  register Lisp_Object tem;
  register struct Lisp_Marker *m;

  if (NULL (bufname))
    buf = Fcurrent_buffer ();
  else
    buf = Fget_buffer (bufname);
  if (NULL (buf))
    nsberror (bufname);

  b = XBUFFER (buf);

  /* Make sure b->text.modified is correct
     even if buffer is selected now.  */
  bf_cur->text.modified = bf_modified;

  if (INTERACTIVE && !NULL (b->filename)
      && b->text.modified > b->save_modified)
    {
      tem = Fyes_or_no_p (format1 ("Buffer %s modified; kill anyway? ",
				   XSTRING (b->name)->data));
      if (NULL (tem))
	return Qnil;
    }

  /* We have no more questions to ask.  Verify that it is valid
     to kill the buffer.  This must be done after the questions
     since anything can happen within Fyes_or_no_p.  */

  /* Don't kill the minibuffer now current.  */
  if (EQ (buf, XWINDOW (minibuf_window)->buffer))
    return Qnil;

  if (NULL (b->name))
    return Qnil;

  /* Make this buffer not be current.
     In the process, notice if this is the sole visible buffer
     and give up if so.  */
  if (b == bf_cur)
    {
      tem = Fother_buffer (buf);
      Fset_buffer (tem);
      if (b == bf_cur)
	return Qnil;
    }

  /* Now there is no question: we can kill the buffer.  */

#ifdef CLASH_DETECTION
  /* Unlock this buffer's file, if it is locked.  */
  unlock_buffer (b);
#endif /* CLASH_DETECTION */

#ifdef subprocesses
  kill_buffer_processes (buf);
#endif subprocesses

  tem = Vinhibit_quit;
  Vinhibit_quit = Qt;
  Vbuffer_alist = Fdelq (Frassq (buf, Vbuffer_alist), Vbuffer_alist);
  Freplace_buffer_in_windows (buf);
  Vinhibit_quit = tem;

  /* Unchain all markers of this buffer
     and leave them pointing nowhere.  */
  for (tem = b->markers; !EQ (tem, Qnil); )
    {
      m = XMARKER (tem);
      m->buffer = 0;
      tem = m->chain;
      m->chain = Qnil;
    }
  b->markers = Qnil;

  b->name = Qnil;
  free (b->text.p1 + 1);
  if (b->undodata)
    free_undo_records (b);

  return Qnil;
}

/* Put the element for buffer `buf' at the front of buffer-alist.
 This is done when a buffer is selected "visibly".
 It keeps buffer-alist in the order of recency of selection
 so that other_buffer will return something nice.  */

record_buffer (buf)
     Lisp_Object buf;
{
  register Lisp_Object link, prev;

  prev = Qnil;
  for (link = Vbuffer_alist; CONSP (link); link = XCONS (link)->cdr)
    {
      if (EQ (XCONS (XCONS (link)->car)->cdr, buf))
	break;
      prev = link;
    }

  /* Effectively do Vbuffer_alist = Fdelq (link, Vbuffer_alist)
     but cannot use Fdelq here it that allows quitting.  */

  if (NULL (prev))
    Vbuffer_alist = XCONS (Vbuffer_alist)->cdr;
  else
    XCONS (prev)->cdr = XCONS (XCONS (prev)->cdr)->cdr;
	
  XCONS(link)->cdr = Vbuffer_alist;
  Vbuffer_alist = link;
}

DEFUN ("switch-to-buffer", Fswitch_to_buffer, Sswitch_to_buffer, 1, 2, "BSwitch to buffer: ",
  "Select buffer BUFFER in the current window.\n\
BUFFER may be a buffer or a buffer name.\n\
Optional second arg NORECORD non-nil means\n\
do not put this buffer at the front of the list of recently selected ones.\n\
\n\
WARNING: This is NOT the way to work on another buffer temporarily\n\
within a Lisp program!  Use `set-buffer' instead.  That avoids messing with\n\
the window-buffer correspondances.")
  (bufname, norecord)
     Lisp_Object bufname, norecord;
{
  register Lisp_Object buf;
  if (NULL (bufname))
    buf = Fother_buffer (Fcurrent_buffer ());
  else
    buf = Fget_buffer_create (bufname);
  Fset_buffer (buf);
  if (NULL (norecord))
    record_buffer (buf);

  Fset_window_buffer (EQ (selected_window, minibuf_window)
		      ? Fnext_window (minibuf_window, Qnil) : selected_window,
		      buf);

  return Qnil;
}

DEFUN ("pop-to-buffer", Fpop_to_buffer, Spop_to_buffer, 1, 2, 0,
  "Select buffer BUFFER in some window, preferably a different one.\n\
If  pop-up-windows  is non-nil, windows can be split to do this.\n\
If second arg  OTHER-WINDOW is non-nil, insist on finding another\n\
window even if BUFFER is already visible in the selected window.")
  (bufname, other)
     Lisp_Object bufname, other;
{
  register Lisp_Object buf;
  if (NULL (bufname))
    buf = Fother_buffer (Fcurrent_buffer ());
  else
    buf = Fget_buffer_create (bufname);
  Fset_buffer (buf);
  record_buffer (buf);
  Fselect_window (Fdisplay_buffer (buf, other));
  return Qnil;
}

DEFUN ("current-buffer", Fcurrent_buffer, Scurrent_buffer, 0, 0, 0,
  "Return the current buffer as a Lisp buffer object.")
  ()
{
  register Lisp_Object buf;
  XSET (buf, Lisp_Buffer, bf_cur);
  return buf;
}

DEFUN ("set-buffer", Fset_buffer, Sset_buffer, 1, 1, 0,
  "Set the current buffer to the buffer or buffer name supplied as argument.\n\
That buffer will then be the default for editing operations and printing.\n\
This function's effect can't last past end of current command\n\
because returning to command level\n\
selects the chosen buffer of the current window,\n\
and this function has no effect on what buffer that is.\n\
See also `save-excursion' when you want to select a buffer temporarily.\n\
Use `switch-to-buffer' or `pop-to-buffer' for interactive buffer selection.")
  (bufname)
     register Lisp_Object bufname;
{
  register Lisp_Object buffer;
  buffer = Fget_buffer (bufname);
  if (NULL (buffer))
    nsberror (bufname);
  SetBfp (XBUFFER (buffer));
  return buffer;
}

DEFUN ("barf-if-buffer-read-only", Fbarf_if_buffer_read_only,
				   Sbarf_if_buffer_read_only, 0, 0, 0,
  "Signal a  buffer-read-only  error if the current buffer is read-only.")
  ()
{
  while (!NULL (bf_cur->read_only))
    Fsignal (Qbuffer_read_only, (Fcons (Fcurrent_buffer (), Qnil)));
  return Qnil;
}

DEFUN ("bury-buffer", Fbury_buffer, Sbury_buffer, 0, 1, "",
  "Put BUFFER at the end of the list of all buffers.\n\
There it is the least likely candidate for other-buffer to return;\n\
thus, the least likely buffer for \\[switch-to-buffer] to select by default.")
  (buf)
     register Lisp_Object buf;
{
  register Lisp_Object aelt, link;

  if (NULL (buf))
    {
      XSET (buf, Lisp_Buffer, bf_cur);
      Fswitch_to_buffer (Fother_buffer (buf), Qnil);
    }
  else
    {
      Lisp_Object buf1 = Fget_buffer (buf);
      if (NULL (buf1))
	nsberror (buf);
      buf = buf1;
    }	  

  aelt = Frassq (buf, Vbuffer_alist);
  link = Fmemq (aelt, Vbuffer_alist);
  Vbuffer_alist = Fdelq (aelt, Vbuffer_alist);
  XCONS (link)->cdr = Qnil;
  Vbuffer_alist = nconc2 (Vbuffer_alist, link);
  return Qnil;
}

extern int last_known_column_point;

/* set the current buffer to p */
SetBfp (p)
     register struct buffer *p;
{
  register struct buffer *c = bf_cur;
  register struct window *w = XWINDOW (selected_window);
  register struct buffer *swb;
  register Lisp_Object tail, valcontents;
  enum Lisp_Type tem;

  if (c == p)
    return;

  if (w)
    swb = NULL (selected_window) ? 0 : XBUFFER (w->buffer);

  if (p && NULL (p->name))
    error ("Selecting deleted buffer");
  windows_or_buffers_changed = 1;

  if (c)
    {
      if (c == swb)
	Fset_marker (w->pointm, make_number (point), w->buffer);

      if (point < FirstCharacter || point > NumCharacters + 1)
	abort ();

      c->text = bf_text;
    }
  bf_cur = p;
  bf_text = p->text;
  if (p == swb)
    {
      SetPoint (marker_position (w->pointm));
      if (point < FirstCharacter)
	point = FirstCharacter;
      if (point > NumCharacters + 1)
	point = NumCharacters + 1;
    }
  last_known_column_point = -1;   /* invalidate indentation cache */

  /* Vcheck_symbol is set up to the symbol paragraph-start
     in order to check for the bug that clobbers it.  */
/* if (c && EQ (c->major_mode, Qlisp_mode)
      && XFASTINT (Vcheck_symbol) != 0
      && !NULL (Vcheck_symbol))
    {
      valcontents = XSYMBOL (Vcheck_symbol)->value;
      if (XTYPE (valcontents) != Lisp_Some_Buffer_Local_Value)
	abort ();
      if (c == XBUFFER (XCONS (XCONS (valcontents)->cdr)->car)
	  && (XTYPE (XCONS (valcontents)->car) != Lisp_String
	      || XSTRING (XCONS (valcontents)->car)->size != 6))
	abort ();
    }
*/

  /* Look down buffer's list of local Lisp variables
     to find and update any that forward into C variables. */

  for (tail = p->local_var_alist; !NULL (tail); tail = XCONS (tail)->cdr)
    {
      valcontents = XSYMBOL (XCONS (XCONS (tail)->car)->car)->value;
      if ((XTYPE (valcontents) == Lisp_Buffer_Local_Value
	   || XTYPE (valcontents) == Lisp_Some_Buffer_Local_Value)
	  && (tem = XTYPE (XCONS (valcontents)->car),
	      (tem == Lisp_Boolfwd || tem == Lisp_Intfwd
	       || tem == Lisp_Objfwd)))
	/* Just reference the variable
	     to cause it to become set for this buffer.  */
	Fsymbol_value (XCONS (XCONS (tail)->car)->car);
    }

  /* Do the same with any others that were local to the previous buffer */

  if (c)
    for (tail = c->local_var_alist; !NULL (tail); tail = XCONS (tail)->cdr)
      {
	valcontents = XSYMBOL (XCONS (XCONS (tail)->car)->car)->value;
	if ((XTYPE (valcontents) == Lisp_Buffer_Local_Value
	     || XTYPE (valcontents) == Lisp_Some_Buffer_Local_Value)
	    && (tem = XTYPE (XCONS (valcontents)->car),
		(tem == Lisp_Boolfwd || tem == Lisp_Intfwd
		 || tem == Lisp_Objfwd)))
	  /* Just reference the variable
               to cause it to become set for this buffer.  */
	  Fsymbol_value (XCONS (XCONS (tail)->car)->car);
      }
  /* Vcheck_symbol is set up to the symbol paragraph-start
     in order to check for the bug that clobbers it.  */
/*if (EQ (p->major_mode, Qlisp_mode)
      && Vcheck_symbol
      && !NULL (Vcheck_symbol))
    {
      valcontents = XSYMBOL (Vcheck_symbol)->value;
      if (XTYPE (valcontents) != Lisp_Some_Buffer_Local_Value)
	abort ();
      if (p == XBUFFER (XCONS (XCONS (valcontents)->cdr)->car)
	  && (XTYPE (XCONS (valcontents)->car) != Lisp_String
	      || XSTRING (XCONS (valcontents)->car)->size != 6))
	abort ();
      Fsymbol_value (Vcheck_symbol);
      valcontents = XSYMBOL (Vcheck_symbol)->value;
      if (p != XBUFFER (XCONS (XCONS (valcontents)->cdr)->car)
	  || XTYPE (XCONS (valcontents)->car) != Lisp_String
	  || XSTRING (XCONS (valcontents)->car)->size != 6)
	abort ();
    }
 */
}

/* set the current buffer to p "just for redisplay" */
SetBfx (p)
     register struct buffer *p;
{
  if (bf_cur == p)
    return;

  bf_cur->text = bf_text;
  bf_cur = p;
  bf_text = p->text;
}

DEFUN ("erase-buffer", Ferase_buffer, Serase_buffer, 0, 0, 0,
  "Delete the entire contents of the current buffer.")
  ()
{
  Fwiden ();
  del_range (1, NumCharacters + 1);
  bf_cur->last_window_start = 1;
  /* Prevent warnings, or suspension of auto saving, that would happen
     if future size is less than past size.  Use of erase-buffer
     implies that the future text is not really related to the past text.  */
  XFASTINT (bf_cur->save_length) = 0;
  return Qnil;
}

validate_region (b, e)
     register Lisp_Object *b, *e;
{
  register int i;

  CHECK_NUMBER_COERCE_MARKER (*b, 0);
  CHECK_NUMBER_COERCE_MARKER (*e, 1);

  if (XINT (*b) > XINT (*e))
    {
      i = XFASTINT (*b);	/* This is legit even if *b is < 0 */
      *b = *e;
      XFASTINT (*e) = i;	/* because this is all we do with i.  */
    }

  if (!(FirstCharacter <= XINT (*b) && XINT (*b) <= XINT (*e)
        && XINT (*e) <= 1 + NumCharacters))
    args_out_of_range (*b, *e);
}

Lisp_Object
list_buffers_1 (files)
     Lisp_Object files;
{
  register Lisp_Object tail, tem, buf;
  Lisp_Object col1, col2, col3, minspace;
  register struct buffer *old = bf_cur, *b;
  int desired_point = 0;

  bf_cur->text.modified = bf_modified;

  XFASTINT (col1) = 19;
  XFASTINT (col2) = 25;
  XFASTINT (col3) = 40;
  XFASTINT (minspace) = 1;

  SetBfp (XBUFFER (Vstandard_output));

  tail = intern ("Buffer-menu-mode");
  if (!EQ (tail, bf_cur->major_mode)
      && (tem = Ffboundp (tail), !NULL (tem)))
    call0 (tail);
  Fbuffer_flush_undo (Vstandard_output);
  bf_cur->read_only = Qnil;

  write_string ("\
 MR Buffer         Size  Mode           File\n\
 -- ------         ----  ----           ----\n", -1);

  for (tail = Vbuffer_alist; !NULL (tail); tail = Fcdr (tail))
    {
      buf = Fcdr (Fcar (tail));
      b = XBUFFER (buf);
      /* Don't mention the minibuffers. */
      if (XSTRING (b->name)->data[0] == ' ')
	continue;
      /* Optionally don't mention buffers that lack files. */
      if (!NULL (files) && NULL (b->filename))
	continue;
      /* Identify the current buffer. */
      if (b == old)
	desired_point = point;
      write_string (b == old ? "." : " ", -1);
      /* Identify modified buffers */
      write_string (b->text.modified > b->save_modified ? "*" : " ", -1);
      write_string (NULL (b->read_only) ? "  " : "% ", -1);
      Fprinc (b->name, Qnil);
      Findent_to (col1, make_number (2));
      XFASTINT (tem) = b->text.size1 + b->text.size2;
      Fprin1 (tem, Qnil);
      Findent_to (col2, minspace);
      Fprinc (b->mode_name, Qnil);
      Findent_to (col3, minspace);
      if (!NULL (b->filename))
	Fprinc (b->filename, Qnil);
      write_string ("\n", -1);
    }

  bf_cur->read_only = Qt;
  SetBfp (old);
/* Foo.  This doesn't work since temp_output_buffer_show sets point to 1
  if (desired_point)
    XBUFFER (Vstandard_output)->text.pointloc = desired_point;
 */
  return Qnil;
}

DEFUN ("list-buffers", Flist_buffers, Slist_buffers, 0, 1, "",
  "Display a list of names of existing buffers.\n\
Inserts it in buffer *Buffer List* and displays that.\n\
Note that buffers with names starting with spaces are omitted.\n\
Non-null optional arg FILES-ONLY means mention only file buffers.\n\
\n\
The M column contains a * for buffers that are modified.\n\
The R column contains a % for buffers that are read-only.")
  (files)
     Lisp_Object files;
{
  internal_with_output_to_temp_buffer ("*Buffer List*",
				       list_buffers_1, files);
  return Qnil;
}

DEFUN ("kill-all-local-variables", Fkill_all_local_variables, Skill_all_local_variables,
  0, 0, 0,
  "Eliminate all the buffer-local variable values of the current buffer.\n\
This buffer will then see the default values of all variables.")
  ()
{
  register Lisp_Object alist, sym, tem;

  for (alist = bf_cur->local_var_alist; !NULL (alist); alist = XCONS (alist)->cdr)
    {
      sym = XCONS (XCONS (alist)->car)->car;

      /* Need not do anything if some other buffer's binding is now encached.  */
      tem = XCONS (XCONS (XSYMBOL (sym)->value)->cdr)->car;
      if (XBUFFER (tem) == bf_cur)
	{
	  /* Symbol is set up for this buffer's old local value.
	     Set it up for the current buffer with the default value.  */

	  tem = XCONS (XCONS (XSYMBOL (sym)->value)->cdr)->cdr;
	  XCONS (tem)->car = tem;
	  XCONS (XCONS (XSYMBOL (sym)->value)->cdr)->car = Fcurrent_buffer ();
	  store_symval_forwarding (sym, XCONS (XSYMBOL (sym)->value)->car,
				   XCONS (tem)->cdr);
	}
    }

  reset_buffer_local_variables (bf_cur);
  return Qnil;
}

extern Lisp_Object Vprin1_to_string_buffer;	/* in print.c */
init_buffer_once ()
{
  register Lisp_Object tem;

  /* Make sure all markable slots in buffer_defaults
     are initialized reasonably, so mark_buffer won't choke.  */
  reset_buffer (&buffer_defaults);
  reset_buffer (&buffer_local_symbols);
  XSET (Vbuffer_defaults, Lisp_Buffer, &buffer_defaults);
  XSET (Vbuffer_local_symbols, Lisp_Buffer, &buffer_local_symbols);

  /* Set up the default values of various buffer slots.  */
  /* Must do these before making the first buffer! */

  /* real setup is done in loaddefs.el */
  buffer_defaults.mode_line_format = build_string ("%-");
  buffer_defaults.abbrev_mode = Qnil;
  buffer_defaults.overwrite_mode = Qnil;
  buffer_defaults.case_fold_search = Qt;
  buffer_defaults.auto_fill_hook = Qnil;
  buffer_defaults.selective_display = Qnil;
  buffer_defaults.selective_display_ellipses = Qt;
  buffer_defaults.abbrev_table = Qnil;

  XFASTINT (buffer_defaults.tab_width) = 8;
  buffer_defaults.truncate_lines = Qnil;
  buffer_defaults.ctl_arrow = Qt;

  XFASTINT (buffer_defaults.fill_column) = 70;
  XFASTINT (buffer_defaults.left_margin) = 0;

  /* Assign the local-flags to the slots that have default values.
     The local flag is a bit that is used in the buffer
     to say that it has its own local value for the slot.
     The local flag bits are in the local_var_flags slot of the buffer.  */

  /* Nothing can work if this isn't true */
  if (sizeof (int) != sizeof (Lisp_Object)) abort ();

  /* 0 means not a lisp var, -1 means always local, else mask */
  bzero (&buffer_local_flags, sizeof buffer_local_flags);
  XFASTINT (buffer_local_flags.filename) = -1;
  XFASTINT (buffer_local_flags.directory) = -1;
  XFASTINT (buffer_local_flags.backed_up) = -1;
  XFASTINT (buffer_local_flags.save_length) = -1;
  XFASTINT (buffer_local_flags.auto_save_file_name) = -1;
  XFASTINT (buffer_local_flags.read_only) = -1;
  XFASTINT (buffer_local_flags.major_mode) = -1;
  XFASTINT (buffer_local_flags.mode_name) = -1;
  
  XFASTINT (buffer_local_flags.mode_line_format) = 1;
  XFASTINT (buffer_local_flags.abbrev_mode) = 2;
  XFASTINT (buffer_local_flags.overwrite_mode) = 4;
  XFASTINT (buffer_local_flags.case_fold_search) = 8;
  XFASTINT (buffer_local_flags.auto_fill_hook) = 0x10;
  XFASTINT (buffer_local_flags.selective_display) = 0x20;
  XFASTINT (buffer_local_flags.selective_display_ellipses) = 0x40;
  XFASTINT (buffer_local_flags.tab_width) = 0x80;
  XFASTINT (buffer_local_flags.truncate_lines) = 0x100;
  XFASTINT (buffer_local_flags.ctl_arrow) = 0x200;
  XFASTINT (buffer_local_flags.fill_column) = 0x400;
  XFASTINT (buffer_local_flags.left_margin) = 0x800;
  XFASTINT (buffer_local_flags.abbrev_table) = 0x1000;

  Vbuffer_alist = Qnil;
  bf_cur = 0;
  all_buffers = 0;

  QSFundamental = build_string ("Fundamental");

  Qfundamental_mode = intern ("fundamental-mode");
  buffer_defaults.major_mode = Qfundamental_mode;

  Qmode_class = intern ("mode-class");

  Vprin1_to_string_buffer = Fget_buffer_create (build_string (" prin1"));
  /* super-magic invisible buffer */
  Vbuffer_alist = Qnil;

  tem = Fset_buffer (Fget_buffer_create (build_string ("*scratch*")));
  /* Want no undo records for *scratch*
     until after Emacs is dumped */
  Fbuffer_flush_undo (tem);
}

init_buffer ()
{
  char buf[MAXPATHLEN+1];

  Fset_buffer (Fget_buffer_create (build_string ("*scratch*")));
  getwd (buf);
#ifndef VMS
  /* Maybe this should really use some standard subroutine
     whose definition is filename syntax dependent.  */
  if (buf[strlen (buf) - 1] != '/')
    strcat (buf, "/");
#endif /* not VMS */
  bf_cur->directory = build_string (buf);
  if (NULL (Vpurify_flag))
    make_undo_records (bf_cur);
}

/* initialize the buffer routines */
syms_of_buffer ()
{
  staticpro (&Vbuffer_defaults);
  staticpro (&Vbuffer_local_symbols);
  staticpro (&Qfundamental_mode);
  staticpro (&Qmode_class);
  staticpro (&QSFundamental);
  staticpro (&Vbuffer_alist);

/*staticpro (&Qlisp_mode);
  Qlisp_mode = intern ("lisp-mode");
*/

  /* All these use DEFVAR_LISP_NOPRO because the slots in
     buffer_defaults will all be marked via Vbuffer_defaults.  */

  DEFVAR_LISP_NOPRO ("default-mode-line-format",
	      &buffer_defaults.mode_line_format,
    "Default mode-line-format for buffers that do not override it.\n\
This is the same as (default-value 'mode-line-format).");

  DEFVAR_LISP_NOPRO ("default-abbrev-mode",
	      &buffer_defaults.abbrev_mode,
    "Default abbrev-mode for buffers that do not override it.\n\
This is the same as (default-value 'abbrev-mode).");

  DEFVAR_LISP_NOPRO ("default-ctl-arrow",
	      &buffer_defaults.ctl_arrow,
    "Default ctl-arrow for buffers that do not override it.\n\
This is the same as (default-value 'ctl-arrow).");

  DEFVAR_LISP_NOPRO ("default-truncate-lines",
	      &buffer_defaults.truncate_lines,
    "Default truncate-lines for buffers that do not override it.\n\
This is the same as (default-value 'truncate-lines).");

  DEFVAR_LISP_NOPRO ("default-fill-column",
	      &buffer_defaults.fill_column,
    "Default fill-column for buffers that do not override it.\n\
This is the same as (default-value 'fill-column).");

  DEFVAR_LISP_NOPRO ("default-left-margin",
	      &buffer_defaults.left_margin,
    "Default left-margin for buffers that do not override it.\n\
This is the same as (default-value 'left-margin).");

  DEFVAR_LISP_NOPRO ("default-tab-width",
	      &buffer_defaults.tab_width,
    "Default tab-width for buffers that do not override it.\n\
This is the same as (default-value 'tab-width).");

  DEFVAR_LISP_NOPRO ("default-case-fold-search",
	      &buffer_defaults.case_fold_search,
    "Default case-fold-search for buffers that do not override it.\n\
This is the same as (default-value 'case-fold-search).");

  DEFVAR_PER_BUFFER ("mode-line-format", &bf_cur->mode_line_format, 0);

/* This doc string is too long for cpp; cpp dies.
  DEFVAR_PER_BUFFER ("mode-line-format", &bf_cur->mode_line_format,
    "Template for displaying mode line for current buffer.\n\
Each buffer has its own value of this variable.\n\
Value may be a string, a symbol or a list or cons cell.\n\
For a symbol, its value is used (but it is ignored if t or nil).\n\
 A string appearing directly as the value of a symbol is processed verbatim\n\
 in that the %-constructs below are not recognized.\n\
For a list whose car is a symbol, the symbol's value is taken,\n\
 and if that is non-nil, the cadr of the list is processed recursively.\n\
 Otherwise, the caddr of the list (if there is one) is processed.\n\
For a list whose car is a string or list, each element is processed\n\
 recursively and the results are effectively concatenated.\n\
For a list whose car is an integer, the cdr of the list is processed\n\
  and padded (if the number is positive) or truncated (if negative)\n\
  to the width specified by that number.\n\
A string is printed verbatim in the mode line except for %-constructs:\n\
  (%-constructs are allowed when the string is the entire mode-line-format\n\
   or when it is found in a cons-cell or a list)\n\
  %b -- print buffer name.      %f -- print visited file name.\n\
  %* -- print *, % or hyphen.   %m -- print value of mode-name (obsolete).\n\
  %s -- print process status.   %M -- print value of global-mode-string. (obs)\n\
  %p -- print percent of buffer above top of window, or top, bot or all.\n\
  %n -- print Narrow if appropriate.\n\
  %[ -- print one [ for each recursive editing level.  %] similar.\n\
  %% -- print %.   %- -- print infinitely many dashes.\n\
Decimal digits after the % specify field width to which to pad.");
*/

  DEFVAR_LISP_NOPRO ("default-major-mode", &buffer_defaults.major_mode,
    "*Major mode for new buffers.  Defaults to fundamental-mode.\n\
nil here means use current buffer's major mode.");

  DEFVAR_PER_BUFFER ("major-mode", &bf_cur->major_mode,
    "Symbol for current buffer's major mode.");

  DEFVAR_PER_BUFFER ("abbrev-mode", &bf_cur->abbrev_mode,
    "Non-nil turns on automatic expansion of abbrevs when inserted.\n\
Automatically becomes local when set in any fashion.");

  DEFVAR_PER_BUFFER ("case-fold-search", &bf_cur->case_fold_search,
    "*Non-nil if searches should ignore case.\n\
Automatically becomes local when set in any fashion.");

  DEFVAR_PER_BUFFER ("mode-name", &bf_cur->mode_name,
    "Pretty name of current buffer's major mode (a string).");

  DEFVAR_PER_BUFFER ("fill-column", &bf_cur->fill_column,
    "*Column beyond which automatic line-wrapping should happen.\n\
Automatically becomes local when set in any fashion.");

  DEFVAR_PER_BUFFER ("left-margin", &bf_cur->left_margin,
    "*Column for the default indent-line-function to indent to.\n\
Linefeed indents to this column in Fundamental mode.\n\
Automatically becomes local when set in any fashion.");

  DEFVAR_PER_BUFFER ("tab-width", &bf_cur->tab_width,
    "*Distance between tab stops (for display of tab characters), in columns.\n\
Automatically becomes local when set in any fashion.");

  DEFVAR_PER_BUFFER ("ctl-arrow", &bf_cur->ctl_arrow,
    "*Non-nil means display control chars with uparrow.\n\
Nil means use backslash and octal digits.\n\
Automatically becomes local when set in any fashion.");

  DEFVAR_PER_BUFFER ("truncate-lines", &bf_cur->truncate_lines,
    "*Non-nil means do not display continuation lines;\n\
give each line of text one screen line.\n\
Automatically becomes local when set in any fashion.\n\
\n\
Note that this is overridden by the variable\n\
truncate-partial-width-windows if that variable is non-nil\n\
and this buffer is not full-screen width.");

  DEFVAR_PER_BUFFER ("default-directory", &bf_cur->directory,
    "Name of default directory of current buffer.  Should end with slash.");

  DEFVAR_PER_BUFFER ("auto-fill-hook", &bf_cur->auto_fill_hook,
    "Function called (if non-nil) after self-inserting a space at column beyond fill-column");

  DEFVAR_PER_BUFFER ("buffer-file-name", &bf_cur->filename,
    "Name of file visited in current buffer, or nil if not visiting a file.");

  DEFVAR_PER_BUFFER ("buffer-auto-save-file-name",
		    &bf_cur->auto_save_file_name,
    "Name of file for auto-saving current buffer,\n\
or nil if buffer should not be auto-saved.");

  DEFVAR_PER_BUFFER ("buffer-read-only", &bf_cur->read_only,
    "Non-nil if this buffer is read-only.");

  DEFVAR_PER_BUFFER ("buffer-backed-up", &bf_cur->backed_up,
    "Non-nil if this buffer's file has been backed up.\n\
Backing up is done before the first time the file is saved.");

  DEFVAR_PER_BUFFER ("buffer-saved-size", &bf_cur->save_length,
    "Length of current buffer when last read in, saved or auto-saved.\n\
0 initially.");

  DEFVAR_PER_BUFFER ("selective-display", &bf_cur->selective_display,
    "t enables selective display:\n\
 after a ^M, all the rest of the line is invisible.\n\
 ^M's in the file are written into files as newlines.\n\
Integer n as value means display only lines\n\
 that start with less than n columns of space.\n\
Automatically becomes local when set in any fashion.");

  DEFVAR_PER_BUFFER ("selective-display-ellipses",
		    &bf_cur->selective_display_ellipses,
    "t means display ... on previous line when a line is invisible.\n\
Automatically becomes local when set in any fashion.");

  DEFVAR_PER_BUFFER ("overwrite-mode", &bf_cur->overwrite_mode,
    "Non-nil if self-insertion should replace existing text.\n\
Automatically becomes local when set in any fashion.");

/*DEFVAR_LISP ("debug-check-symbol", &Vcheck_symbol,
    "Don't ask.");
*/
  defsubr (&Sbuffer_list);
  defsubr (&Sget_buffer);
  defsubr (&Sget_file_buffer);
  defsubr (&Sget_buffer_create);
  defsubr (&Sgenerate_new_buffer);
  defsubr (&Sbuffer_name);
/*defsubr (&Sbuffer_number);*/
  defsubr (&Sbuffer_file_name);
  defsubr (&Sbuffer_local_variables);
  defsubr (&Sbuffer_modified_p);
  defsubr (&Sset_buffer_modified_p);
  defsubr (&Srename_buffer);
  defsubr (&Sother_buffer);
  defsubr (&Sbuffer_flush_undo);
  defsubr (&Sbuffer_enable_undo);
  defsubr (&Skill_buffer);
  defsubr (&Serase_buffer);
  defsubr (&Sswitch_to_buffer);
  defsubr (&Spop_to_buffer);
  defsubr (&Scurrent_buffer);
  defsubr (&Sset_buffer);
  defsubr (&Sbarf_if_buffer_read_only);
  defsubr (&Sbury_buffer);
  defsubr (&Slist_buffers);
  defsubr (&Skill_all_local_variables);
}

keys_of_buffer ()
{
  defkey (CtlXmap, 'b', "switch-to-buffer");
  defkey (CtlXmap, 'k', "kill-buffer");
  defkey (CtlXmap, Ctl ('B'), "list-buffers");
}
