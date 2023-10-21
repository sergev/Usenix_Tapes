#include <stdio.h>
#include <fcntl.h>

char *gets();
char rest[256];
char *rptr = NULL;
char pipeactive = 0;
currname = 0;
char *pipename[2] = {
	"shtmp1",
	"shtmp2"};
char *
getnextcmd(buf)
	char *buf;
{
	char *fgets(),*index(),*pipe,*semi;
	if (!rptr)
	{
		register char *c;
		if (0 == read(0,rest,sizeof(rest)))
			return NULL;
		c = rptr = rest;
		while (*c)
		{
			if (*c == '\r' || *c == '\n')
			{
				*c = '\0';
				break;
			}
			++c;
		}

	}
	pipe = index(rptr,'|');
	semi = index(rptr,';');
	if (pipe == NULL && semi == NULL)
	{
		strcpy(buf,rptr);
		if (pipeactive)
		{
			pipeactive = 0;
			strcat(buf," < ");
			strcat(buf,pipename[currname]);
		}
		rptr=NULL;
	}
	/* one or the other, or both are not NULL, so comparison is in order */
	else if (pipe && (!semi || (pipe < semi)))
	{
		*pipe = '\0';	/* terminate string */
		strcpy(buf,rptr); /* copy to buf */
		rptr = pipe+1;	/* set up rest */
		if (pipeactive++)
		{
			pipeactive = 1;
			strcat(buf," < ");
			strcat(buf,pipename[currname]);
		}
		strcat(buf," > ");
		currname ^= 1;	/* flip flop pipe names */
		strcat(buf,pipename[currname]);
	}
	else if (semi && (!pipe || (semi < pipe)))
	/* we have a semicolon to deal with */
	{
		*semi = '\0';
		strcpy(buf,rptr);
		rptr = semi+1;
		if (pipeactive)
		{
			pipeactive = 0;
			strcat(buf," < ");
			strcat(buf,pipename[currname]);
		}
	}
	return buf;
}
