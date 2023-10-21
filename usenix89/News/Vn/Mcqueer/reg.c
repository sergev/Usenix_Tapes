/*
** vn news reader.
**
** reg.c - implementation of regex / regcmp on top of UCB library
**
** see copyright disclaimer / history in vn.c source file
*/

#include <stdio.h>

#define RGBLKSIZE 20

struct _regtab
{
	struct _regtab *link;
	char *regstr;
};

typedef struct _regtab REGTAB;

static REGTAB *Chain = NULL;
static REGTAB *Free = NULL;
static REGTAB *Compiled = NULL;

regfree(s)
char *s;
{
	REGTAB *ptr,*cmp,*old;

	cmp = (REGTAB *) s;
	old = NULL;

	for (ptr = Chain; ptr != NULL; ptr = (old = ptr)->link)
	{
		if (ptr == cmp)
		{
			if (old == NULL)
				Chain = Chain->link;
			else
				old->link = ptr->link;
			ptr->link = Free;
			Free = ptr;
			break;
		}
	}
}

char *regcmp(str)
char *str;
{
	int i;
	char *str_store();
	char *re_comp();

	if (re_comp(str) != NULL)
	{
		Compiled = NULL;	/* make sure we're OK */
		return(NULL);
	}

	if (Free == NULL)
	{
		Free = (REGTAB *) malloc(RGBLKSIZE * sizeof(REGTAB));
		if (Free == NULL)
			printex ("regcmp: memory allocation failure");
		for (i = 0; i < RGBLKSIZE - 1; ++i)
			Free[i].link = Free + i + 1;
		Free[i].link = NULL;
	}

	Compiled = Free;
	Free = Free->link;

	Compiled->link = Chain;
	Chain = Compiled;
	Compiled->regstr = str_store(str);

	return ((char *) Compiled);
}

char *regex(reg,str)
char *reg,*str;
{
	REGTAB *cmp;

	cmp = (REGTAB *) reg;

	if (cmp == Compiled)
	{
		if (re_exec(str))
			return(str);
		return (NULL);
	}

	for (Compiled = Chain; Compiled != NULL; Compiled = Compiled->link)
	{
		if (Compiled == cmp)
			break;
	}

	if (Compiled == NULL)
		printex ("regex: bad pointer");

	re_comp(Compiled->regstr);

	if (re_exec(str))
		return(str);

	return(NULL);
}
