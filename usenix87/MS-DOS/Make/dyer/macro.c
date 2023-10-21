#include <stdio.h>
#include "make.h"

/*
 * Macro processing
 */


/*
 * Perform macro substitution from 'orig' to 'dest'.
 * Return number of macro substitutions made.
 * A macro reference is in one of two forms:
 *		<MACCHAR>(macro-name)
 *  	or	<MACCHAR><single-character>
 *
 * "<MACCHAR><MACCHAR>" expands to a single '<MACCHAR>'
 */
mexpand(orig, dest, destsiz, macchar)
char *orig, *dest;
int destsiz;
char macchar;
{
	char *s, *d, mname[STRSIZ];
	int di, count;
	MACRO *m;

	di = count = 0;
	for(s=orig; *s;)
		if(*s == macchar)
		{
			if(*++s == macchar)
			{
				if(di < destsiz-1) dest[di++] = *s++;
				continue;
			}

			if(!*s) break;
			d = mname;
			if(*s != '(') *d++ = *s++;
			else
			{
				for(++s; *s && *s!=')';) *d++ = *s++;
				if(*s != ')') puts("Missed matching ')'");
				else ++s;
			}
			*d = 0;
			if((d = gmacro(mname)) == NULL)
				fprintf(stderr, "Undefined macro: %s\n", mname);
			else
			{
				while(*d && di < (destsiz - 1))
					dest[di++] = *d++;
				++count;
			}
		} else if(di < destsiz-1)
			dest[di++] = *s++;

	dest[di]=0;
	return count;
}


/*
 * Define a macro.
 * Give the macro called 'name' the string expansion 'def'.
 * Old macro-names are superseded, NOT replaced.
 * Return ERROR if can't define the macro.
 */
defmac(name, def)
char *name, *def;
{
	MACRO *m;

	if((m = (MACRO *)malloc(sizeof(MACRO))) == NULL) allerr();
	if((m->mname = (char *)malloc(strlen(name)+1)) == NULL) allerr();
	if((m->mvalue = (char *)malloc(strlen(def)+1)) == NULL) allerr();

	strcpy(m->mname, name);
	strcpy(m->mvalue, def);
	m->mnext = mroot;
	mroot = m;
}


/*
 * undefmac - undefine a macro.
 * Return 0 if macro was succesfully undefined, -1 if not found.
 */
undefmac(name)
char *name;
{
	MACRO *m = mroot;
	MACRO *prev = NULL;

	while(m != NULL && strcmp(name, m->mname))
	{
		prev = m;
		m = m->mnext;
	}

	if(m == NULL) return -1;
	if(prev == NULL) mroot = m->mnext;
	    else prev->mnext = m->mnext;

	free(m->mname);
	free(m->mvalue);
	free(m);
	return 0;
}


/*
 * Lookup a macro called 'name'.
 * Return a pointer to its definition,
 * or NULL if it does not exist.
 */
char *gmacro(name)
char *name;
{
	MACRO *m;

	for(m=mroot; m != NULL; m=m->mnext)
		if(!strcmp(name, m->mname)) return m->mvalue;
	return NULL;
}
