#include "stat.h"
char exists__[] "~|^`exists.c:	2.1";
/*
	Returns "mode" word if file exists; otherwise 0.
*/

exists(f)
char f[];
{
	struct inode sb;

	if(stat(f,&sb) == -1) return(0);
	else return(sb.i_mode);
}

