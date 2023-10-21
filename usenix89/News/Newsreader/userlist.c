#include <stdio.h>
#include "vn.h"

extern PAGE Page;

/*
	generate user list of articles - either article numbers
	are input directly (numeric list), or input is a search
	string - invoke regular expression library and examine titles
	search string "*" reserved for marked articles.  Strings may
	be prefixed with '!' for negation.
*/
userlist (list)
char *list;
{
	int i,j,anum[RECLEN/2],acount;
	char neg, *s, sbuf[MAX_C+1], *reg, *regex(), *regcmp(), *index(), *strtok();

	user_str (sbuf,"Articles or title search string : ");
	if (sbuf[0] == '!')
	{
		neg = '!';
		s = sbuf+1;
	}
	else
	{
		neg = '\0';
		s = sbuf;
	}
	for (i=0; s[i] != '\0'; ++i)
	{
		if (index(LIST_SEP,s[i]) == NULL)
		{
			if (s[i] < '0' || s[i] > '9')
				break;
		}
	}
	acount = 0;

	if (s[i] == '\0')
	{
		for (s = strtok(s,LIST_SEP); s != NULL; s = strtok(NULL,LIST_SEP))
		{
			anum[acount] = atoi(s);
			++acount;
		}
	}
	else
	{
		if (s[0] == ART_MARK)
		{
			for (i=0; i < Page.h.artnum; ++i)
			{
				if (Page.b[i].art_mark == ART_MARK)
				{
					anum[acount] = Page.b[i].art_id;
					++acount;
				}
			}
		}
		else
		{
			reg = regcmp(s,(char *) 0);
			if (reg != NULL)
			{
				for (i=0; i < Page.h.artnum; ++i)
				{
					if (regex(reg,Page.b[i].art_t) != NULL)
					{
						anum[acount] = Page.b[i].art_id;
						++acount;
					}
				}
				regfree (reg);
			}
			else
				preinfo ("bad regular expression syntax");
		}
	}

	/* algorithm is inefficient, but we're only handling a few numbers */
	*list = '\0';
	for (i=0; i < Page.h.artnum; ++i)
	{
		for (j=0; j < acount && anum[j] != Page.b[i].art_id; ++j)
			;
		if (neg == '!')
		{
			if (j < acount)
				continue;
		}
		else
		{
			if (j >= acount)
				continue;
		}
		sprintf (list,"%d ",Page.b[i].art_id);
		list += strlen(list);
	}
}
