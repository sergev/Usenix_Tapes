/* Copyright (C) 1981,1982, 1983 by Manx Software Systems */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#ifndef NULL
#define NULL ((void *)0)
#endif

char *get_first(), *get_next(), *sbrk();
char *wilderr = "No wild cards in command names!!\r\n";

/* noexpand can be set by routines before calling _Croot */
int noexpand = 0;

#define ARGMAX 256
static char *Argv[ARGMAX];
static int Argc;
static char curr_path[128];

_Croot(cp,cmd)
register char *cp;
int (*cmd)();
{
	int returnval = 0;
	char *startbuf,*endbuf;
	register char *cp2;
	char *wild_match;
	char *index(),*rindex(), *save_str();
	char *path,*copy; int j;
	char *quote;
	int k,omode; char *fname;
	int in = -1, out = -1;
	/* lets try not to free things not allocated by malloc */
	startbuf = cp; endbuf = &cp[strlen(cp)];
	if (!noexpand)
	{
		/* ls is a special case !!! */
		if(0 == strncmp(cp,"ls",2) || 0 == strncmp(cp,"dir",3))
			noexpand++;
	}

	/* loop through arguments */
	for (Argc = 0;;) 
	{
		/* skip blanks */
		while (*cp == ' ' || *cp == '\t')
			++cp;

		/* if you're at the end of command line, you're done */
		if (*cp == 0)
			break;
		/* handle redirection . . . */
		if (*cp == '>')
		{
			k = 1;
			if (cp[1] == '>')
			{
				++cp;
				omode = O_CREAT | O_WRONLY | O_APPEND; 
			}
			else
				omode = O_CREAT | O_WRONLY | O_TRUNC;
			goto redirect;
		} else if (*cp == '<')
		{
			k = 0;
	redirect:
			while (*++cp == ' ' || *cp == '\t')
				;
			fname = cp;
			while(*++cp)
				if (*cp == ' ' || *cp == '\t')
				{
					*cp++ = 0;
					break;
				}
			close(k);
			if (k)
				out = k = open(fname,omode);
			else
				in = k = open(fname,O_RDONLY);
			if (k == -1)
			{
				perror("redirection");
				return -1;
			}
			/* go back for next argument */
			continue;
		}
		/* find beginning of next argument */
		cp2 = cp;	/* save original pointer to the string */
		while (*++cp2)
		{
			/* if you hit a space char - stick a null in to terminate last
			   argument
			 */
			if (*cp2 == ' ' || *cp2 == '\t') 
			{
				*cp2++ = 0;
				break;
			}
		}

		/* if no wild card characters, do it the old fashioned way */
		if (index(cp,'*') == NULL && index(cp,'?') == NULL)
		{
notranslate:
			if (*cp == '\'')
			/* pass through untranslated, with quotes stripped */
			{
				cp++;	/* point past quote */
				if ( NULL == (quote = rindex(cp,'\'')))
				{
					write(2,"sh - no close quotes on command line\r\n",38);
					goto free_args;
				}
				*quote = '\0';
			}
			/* update the next argv pointer */
			Argv[Argc] = cp;
			/* bump the argument count */
			if (++Argc == ARGMAX)
				abort();
		}
		else
		{
			if (*cp == '"' || *cp == '\'')
				goto notranslate;
			if (noexpand)
				goto notranslate;
			/* wild cards not permitted on first run thru */
			if (Argc == 0)
			{
				write(2,wilderr,strlen(wilderr));
				return -1;
			}
			/* if there is a path included, save it off */
			if ((path = rindex(cp,'\\')) || (path = rindex(cp,'/')))
			{
				copy = cp;
				/* copy to curr_path, mapping / to \ */
				for (j = 0; j < sizeof(curr_path) && copy != path+1; copy++,j++)
					curr_path[j] = (*copy == '/' ? '\\' : *copy);
				/* terminate string */
				curr_path[j] = '\0';
			}
			else if (cp[1] == ':')
			{
				copy = cp;
				for (j = 0; j < 2; j++)
					curr_path[j] = *copy++;
				curr_path[j] = '\0';
			} else
			/* null path */
				curr_path[0] = 0;
			if (wild_match = get_first(cp))
			{
				/* update the next argv pointer */
				Argv[Argc]= save_str(wild_match);
				/* bump the argument count */
				if (++Argc == ARGMAX)
					abort();
				/* get the rest of the matching file names */
				while (wild_match = get_next())
				{
					
					/* update the next argv pointer */
					Argv[Argc] = save_str(wild_match);
					/* bump the argument count */
					if (++Argc == ARGMAX)
						abort();
				}
			}
		}
		cp = cp2;	/* point to beginning of next argument */
	}
	Argv[Argc] = NULL;	
	returnval=(*cmd)(Argc,Argv);
	if (in != -1)
		close(in);
	if (out != -1)
		close(out);
	/* free anything not dynamically allocated */
free_args:
	if (!noexpand)
	{
		for(j = 1;j < Argc; j++)
			if (
			!(Argv[j] >= startbuf && Argv[j] <= endbuf)	/* not in cmd line */
			&& Argv[j]									/* Not NULL			*/
		   		)
			free(Argv[j]);
	}
	noexpand = 0;
	return returnval;
}

char *
save_str(s)
	register char *s;
{
	register char *r,*malloc();
	int pathlen;
	/* squirrel away matched file name */
	if (NULL == (r = malloc(strlen(s)+(pathlen = strlen(curr_path))+1)))
		abort();
	strcat(curr_path,s);
	strcpy(r,curr_path);
	curr_path[pathlen] = '\0';
	return r;
}

abort()
{
	write(2, "Too many args.", 14);
	_exit(200);
}


typedef struct
{
	char dos_reserved[21];
	char attribute;
	unsigned file_time;
	unsigned file_date;
	long file_size;
	char file_name[13];
} fcb;
fcb wildcard;

char *get_first(fname)
	char *fname;
{
	register int result;
	/* set the disk transfer address */
	bdos(0x1A,&wildcard);
	result = bdos(0x4E,fname,0);
	/* make the find first call */
	if(2 == result || 18 == result)
		return NULL;
	return &(wildcard.file_name[0]);
}

char *get_next()
{
	register int result;
	/* set the disk transfer address */
	bdos(0x1A,&wildcard);
	result = bdos(0x4f,0,0);
	/* make the find next call */
	if (18 == result)
		return NULL;
	return &(wildcard.file_name[0]);
}
