#include <stdio.h>

#define ENVSIZE 4000
extern int _PSP;
void *calloc();
char *environment=NULL, *str_upper();
int env_paragraph;
char *next_env;
#define envlimit(a) &a[ENVSIZE-1]

#ifdef MAIN
main
#else
set
#endif
(argc,argv)
	char *argv[];
{
	if (!environment)
		init_env();
	if (argc == 1)
	{
		show_env();
		return 0;
	}
	while(--argc)
	{
		add_env(str_upper(*(++argv)));
	}
	return 0;
}

char *
str_upper(c)
	register char *c;
{
	register char *save = c;
	while(*c)
	{
		*c = toupper(*c);
		c++;
	}
	return save;
}

char *
str_lower(c)
	register char *c;
{
	register char *save = c;
	while(*c)
	{
		*c = tolower(*c);
		c++;
	}
	return save;
}
init_env()
{
	extern unsigned _dsval;	/* current data segment register value */
	long fudgefactor;
	register int c;
	unsigned envseg;
	unsigned offset = 0;
	envseg = peekw(0x2c,_PSP);
	environment = calloc(1,ENVSIZE+16);
	fudgefactor = (long)_dsval << 4;	/* convert to absolute paragraph */
	fudgefactor += (unsigned)environment + 16;
	fudgefactor &= 0xFFFF0L;
	env_paragraph = (int)((fudgefactor >>4) & 0xFFFF);
	environment = (char *) (fudgefactor - (long)(_dsval << 4));
	next_env = environment;
	while (c = peekb(offset,envseg))
	{
		while (c = peekb(offset++,envseg))
		{
			*next_env++ = c;
		}
		*next_env++ = '\0';
	}
}

show_env()
{
	register char *env;
	char c;
	for (env = environment;*env;)
	{
		while (c = *env++)
			write(1,&c,1);
		crlf();
	}
}

static char *enverr = "No more environment space\r\n";
static char *enverr2 = "Improper environment string format!!\r\n";

add_env(string)
	char *string;

{
	char *env_copy, *new, *index();
	char *old = environment;
	char *name_end,*new_name_end;
	int added = 0;
	int namelen;

	if (NULL == (env_copy = new = calloc(1,ENVSIZE)))
	{
		write(2,enverr,strlen(enverr));
		return -1;
	}

	while (*old)
	{
		if ( NULL == (name_end = index(old,'=')) || 
			NULL == (new_name_end = index(string,'=')) 
		)
		{
			write(2,enverr2,strlen(enverr2));
			free(env_copy);
			return -1;
		}
		namelen = (int)(name_end - old);
		if (!strncmp(old,string,namelen))
		{
			if (new_name_end[1])
			{
				/* if we don't have a string of the form name= */
				/* copy new string instead of old string */
				strcpy(new,string);
			}
			else
			/* if we have a set name= with no string then we want
			   to remove the string from the environment
			 */
				;
			added++;
		}
		else
		{
			strcpy(new,old);
		}
		new = &new[strlen(new)+1];
		old = &old[strlen(old)+1];
		if (new >= envlimit(new))
		{
			write(2,enverr,strlen(enverr));
			free(env_copy);
			return -1;
		}
	}
	if (!added)
	{
		strcpy(new,string);
	}
	new = &new[strlen(new)+1];
	/* copy the copy back to the environment */
	movmem(env_copy,environment,(int)(new-env_copy)+2);
	free(env_copy);
	return 0;
}
