/*
** vn news reader.
**
** storage.c - storage allocation routines
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>
#include "vn.h"

extern char *malloc();

extern int L_allow;

extern PAGE Page;
/*
	Storage allocaters.  One more call to malloc in entry_order routine.
*/

char *str_store (s)
char *s;
{
	static unsigned av_len = 0;	/* current storage available */
	static char *avail;
	int len;

	if (s == NULL)
		s = "";

	if ((len = strlen(s)+1) > av_len)
	{
		if (len > STRBLKSIZE)
			av_len = len;
		else
			av_len = STRBLKSIZE;
		if ((avail = malloc(av_len)) == NULL)
			printex ("can't allocate memory for string storage");
	}
	strcpy (avail,s);
	s = avail;
	avail += len;
	av_len -= len;
	return (s);
}

/*
** called after number of terminal lines (L_allow) is known, to set
** up storage for Page.
*/
page_alloc ()
{
	char *body;

	if ((body = malloc(L_allow*sizeof(BODY))) == NULL)
		printex ("can't allocate memory for display storage");

	Page.b = (BODY *) body;
}

NODE
*node_store()
{
	static int nd_avail = 0;
	static NODE *nd;
	NODE *ret;

	if (nd_avail <= 0)
	{
		if ((nd = (NODE *) malloc(sizeof(NODE)*NDBLKSIZE)) == NULL)
			printex ("can't allocate memory for newsgroup table");
		nd_avail = NDBLKSIZE;
	}
	--nd_avail;
	ret = nd;
	++nd;
	return(ret);
}
