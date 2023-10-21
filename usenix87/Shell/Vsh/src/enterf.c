#include "hd.h"
#include "mydir.h"
#include "classify.h"

/* Enterfile selects its parameter fname;  Fname may either be a single
   element file name (no slashes) or a full path name.  Enterfile
   can handle files which look like ".", "..", "file.c", or "/a/b".
   For files which could look like "/a/b/", "a/b", or "./../../a" use
   the procedure "file" (file expects an argument in argv format).
*/
enterfile (fname) register char *fname; 
{
    register char *s;
    register int flag;
    int i;

    flag = classify(fname);
    /* Protect user against entering binary files */
    if (flag == CL_UNKPLN) {
	putmsg(" Binary file, enter ?");
	i = getch();
	if (i == 'y' || i == 'Y')
		flag = CL_TEXT;
    }
    switch (flag) 
    {

      case CL_NULL: 
      case CL_PROTPLN:
	myperror (fname); return NOREPLOT;

      case CL_DIR:
	return enterdir (fname);

      case CL_CORE:
	f_exec (DEBUGGER, DEBUGGER, fname, 0); return REPLOT;

      case CL_AOUT:
	p_exec ("nm", "nm", fname, 0); return REPLOT;

      case CL_AR:
	p_exec ("ar", "ar", "tv", fname, 0); return REPLOT;

      case CL_CPIO:
	p_exec (0, fname, "cpio", "cpio", "-ivt", 0); return REPLOT;

      case CL_PACK:
	p_exec ("pcat", "pcat", fname, 0); return REPLOT;

      case CL_COMPACT:
	p_exec (0, fname, "uncompact", "uncompact", 0); return REPLOT;

      case CL_COMPRESS:
	p_exec (0, fname, "uncompress", "uncompress", 0); return REPLOT;

      case CL_TEXT:
	flag = REPLOT;
	if (strcmp(ENTERTEXT, ENTEREDIT) == 0)
		f_exec (EDITOR, EDITOR, fname, 0);
	else if (strcmp(ENTERTEXT, ENTERDISP) == 0)
		flag = help(fname);
	else {
		s = ENTERTEXT;
		if (*s == ';')
			s++;
		f_exec (s, s, fname, 0);
		if (*ENTERTEXT != ';')
			getrtn();
	}
	return flag;


      default:
	putmsg (fname); printf (":  Vsh cannot handle this file");
    }
    return NOREPLOT;
}
home () 
{			/* enter home directory */

    return file (&HOME);
}
