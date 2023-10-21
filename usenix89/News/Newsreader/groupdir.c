#include <stdio.h>
#include "config.h"
#include "vn.h"

extern PAGE Page;

/*
	g_dir converts newsgroup name to directory string
*/
g_dir(s,t)
char *s,*t;
{
	char *ptr, *index();
	sprintf (t,"%s/%s",SPOOLDIR,s);
	for (ptr=t+strlen(SPOOLDIR)+1; (ptr = index(ptr,'.')) != NULL; *ptr = '/')
		;
}


/*
	change directory to group
*/
cd_group ()
{
	char dbuf [RECLEN];
	g_dir ((Page.h.group)->nd_name,dbuf);
	if (chdir(dbuf) < 0)
	{
		Page.h.artnum = 1;
		Page.b[0].art_id = 0;
		strcpy (Page.b[0].art_t, "CANNOT FIND NEWSGROUP");
	}
}
