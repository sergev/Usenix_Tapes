#include "stdplt.h"

char * _path(fp, buf)
  register FRAME *fp;
  register char *buf;
/*
 * Place full pathname of frame fp in buf.
 */
  {
	if (fp == &_root)
		*buf = '\0';
	else
	  {	_path(fp->_parent, buf);
		if (fp->_parent != &_root)
			strcat(buf, "/");
	  }
	return(strcat(buf, fp->_fname));
  }
