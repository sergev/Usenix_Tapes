#include "stdplt.h"

rmframe(afname)
  char *afname;
/*
 * Remove named frame and its children from tree structure,
 * and release the associated frame structures.
 * It is fatal to remove "." or "/".
 */
  {	register FRAME *fp, *p1, *p2;
	char *fname;

	fname = afname;
	fp = _find(&fname);
	if (*fname)
		_err("rmframe: %s not found\n", _cfname(afname));
	if (fp == &_root)
		_err("rmframe: cannot remove root\n");
	if (fp == _dotp)
		_err("rmframe: cannot remove current frame %s\n", _cfname("."));
/*
 * find the sibling to our left (or parent)
 */
	for (p1 = fp->_parent, p2 = p1->_child; p2 != fp; p2 = p2->_sibling)
		p1 = p2;
/*
 * unlink the named frame from the structure
 */
	if (p1 == fp->_parent)		/* we are leftmost child */
		p1->_child = fp->_sibling;
	else
		p1->_sibling = fp->_sibling;
/*
 * remove child subtree, then us
 */
	_rm(fp->_child);
	cfree(fp);
  }


_rm(fp)
  register FRAME *fp;
/*
 * remove the tree starting at fp.
 * walk the right subtree, removing left subtrees
 * and then the node being walked on.
 */
  {	register FRAME *p;

	while (p = fp)
	  {	if (p == _dotp)
			_err("rmframe: cannot remove current frame %s\n",
				_cfname("."));
		_rm(p->_child);
		fp = p->_sibling;
		cfree(p);
	  }
  }
