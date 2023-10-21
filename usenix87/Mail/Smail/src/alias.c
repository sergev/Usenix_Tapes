#ifndef lint
static char *sccsid = "@(#)alias.c	2.3 (smail) 1/28/87";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "defs.h"
#ifdef BSD
#include <strings.h>
#else
#include <string.h>
#endif

extern enum edebug debug;	/* verbose and debug modes		*/
extern char hostdomain[];
extern char hostname[];
extern char *aliasfile;

#ifdef SENDMAIL
char **
alias(pargc, argv)
int *pargc;
char **argv;
{
	int i;
/*
**  aliasing was done by sendmail - so don't do anything here
**  except to possibly truncate the argument count to MAXARGS.
*/
	if(MAXARGS < *pargc) {
		for(i = MAXARGS ; i < *pargc ; i++) {
			ADVISE("too many recipents - skipping %s\n", argv[i]);
		}
		*pargc = MAXARGS;
	}
	return(argv);
}
#else

/*
**
** Picture of the alias graph structure
**
**	head
**       |
**       v
**	maps -> mark -> gjm -> mel -> NNULL
**       |
**       v
**	sys ->  root -> ron -> NNULL
**       |       |
**       | +-----+
**       | |
**       v v
**	root -> mark -> chris -> lda -> NNULL
**       |
**       v
**      NNULL
*/

typedef struct alias_node node;

static struct alias_node {
	char *string;
	node *horz;
	node *vert;
};

static node aliases = {"", 0, 0}; /* this is the 'dummy header' */

/*
** lint free forms of NULL
*/

#define NNULL	((node  *) 0)
#define CNULL	('\0')

/*
** string parsing macros
*/
#define SKIPWORD(Z)  while(*Z!=' ' && *Z!='\t' && *Z!='\n' && *Z!=',') Z++;
#define SKIPSPACE(Z) while(*Z==' ' || *Z=='\t' || *Z=='\n' || *Z==',') Z++;

static int nargc = 0;
static char *nargv[MAXARGS];

void	add_horz();
void	load_alias(), strip_comments();
node	*vsearch(), *hsearch(), *pop();
int	recipients();
char	*tilde();

/* our horizontal linked list looks like a stack */
#define push		add_horz

#define escape(s)	((*s != '\\') ? (s) : (s+1))

char **
alias(pargc, argv)
int *pargc;
char **argv;
{
/*
**  alias the addresses
*/
	FILE	*fp;
	int	i, aliased;
	char	domain[SMLBUF], ubuf[SMLBUF], *user;
	char	*home, buf[SMLBUF];
	node	*addr, addrstk;
	node	*flist,  fliststk;
	node	*u, *a;
	struct	stat st;

	addr  = &addrstk;
	flist = &fliststk;
	user  = ubuf;

	addr->horz = NNULL;
	flist->horz  = NNULL;

	/*
	** push all of the addresses onto a stack
	*/
	for(i=0; i < *pargc; i++) {
		push(addr, argv[i]);
	}

	/*
	** for each adress, check for included files, aliases and .forward files
	*/
	while((nargc < MAXARGS) && ((u = pop(addr)) != NNULL)) {
		if(strncmpic(u->string, ":include:", 9) == 0) {
			/*
			** make sure it's a full path name
			** don't allow multiple sourcing
			** of a given include file
			*/
			char *p = u->string + 9;

			if((*p == '/')
			&& (hsearch(flist, p) == NULL)) {
				push(flist, p);
				if((stat(p, &st) >= 0)
				&&((st.st_mode & S_IFMT) == S_IFREG)
				&&((fp = fopen(p, "r")) != NULL)) {
					while(fgets(buf, sizeof buf, fp)) {
						recipients(addr, buf);
					}
					fclose(fp);
				}
			}
			continue;
		}

		/*
		** parse the arg to see if it's to be aliased
		*/

		if(islocal(u->string, domain, ubuf) == 0) {
			goto aliasing_complete;
		}

		/*
		** local form - try to alias user
		** aliases file takes precedence over ~user/.forward
		** since that's the way that sendmail does it.
		*/

		user = escape(ubuf);
		if((a = vsearch(user)) != NNULL) {
		/*
		** check for alias
		*/
			node *t = a;
			for(a = a->horz; a != NNULL; a=a->horz) {
				push(addr, a->string);
			}
			t->horz = NNULL;
			continue;
		}

		if((home = tilde(user)) != NULL) {
			/* don't allow multiple sourcing
			** of a given .forward file
			*/

			if((hsearch(flist, home) != NULL)) {
				continue;
			}
			push(flist, home);

			/*
			** check for ~user/.forward file
			** must be a regular, readable file
			*/

			sprintf(buf, "%s/%s", home, ".forward");
			if((stat(buf, &st) >= 0)
			&&((st.st_mode & S_IFMT) == S_IFREG)
			&&((fp = fopen(buf, "r")) != NULL)) {
				aliased = 0;
				while(fgets(buf, sizeof buf, fp)) {
					aliased |= recipients(addr, buf);
				}
				fclose(fp);
				if(aliased) {
					continue;
				}
			}
		}
aliasing_complete:
		user = escape(u->string);
		for(i=0; i < nargc; i++) {
			if(strcmpic(nargv[i], user) == 0) {
				break;
			}
		}

		if(i == nargc) {
			nargv[nargc++] = user;
		}
	}
	*pargc     = nargc;
	return(nargv);
}

/*
** vsearch
**	given an string, look for its alias in
**	the 'vertical' linked list of aliases.
*/
node *
vsearch(user)
char *user;
{
	node *head;
	node *a;
	static int loaded = 0;

	head = &aliases;
	if(loaded == 0) {
		load_alias(head, aliasfile);
		loaded = 1;
	}

	for(a = head->vert; a != NNULL; a = a->vert) {
#ifdef CASEALIAS
		if(strcmp(a->string, user) == 0)
#else
		if(strcmpic(a->string, user) == 0)
#endif
		{
			break;
		}
	}
	if((a == NNULL)			/* not in graph */
	|| (a->horz == NNULL)) {	/* null list (poss. prev. aliased) */
		return(NNULL);
	}
	return(a);
}

/*
** hsearch
**	given an string, look for it in
**	a 'horizontal' linked list of strings.
*/
node *
hsearch(head, str)
node *head;
char *str;
{
	node *a;
	for(a = head->horz; a != NNULL; a = a->horz) {
#ifdef CASEALIAS
		if(strcmp(a->string, str) == 0)
#else
		if(strcmpic(a->string, str) == 0)
#endif
		{
			break;
		}
	}
	return(a);
}

/*
** load_alias
**	parse an 'aliases' file and add the aliases to the alias graph.
**	Handle inclusion of other 'aliases' files.
*/

void
load_alias(head, filename)
node *head;
char *filename;
{
	FILE *fp;
	node *h, *add_vert();
	char domain[SMLBUF], user[SMLBUF];
	char *p, *b, buf[SMLBUF];

	if((fp = fopen(filename,"r")) == NULL) {
DEBUG("load_alias open('%s') failed\n", filename);
		return;
	}

	while(fgets(buf, sizeof buf, fp) != NULL) {
		p = buf;
		if((*p == '#') || (*p == '\n')) {
			continue;
		}

		/*
		** include another file of aliases
		*/

		if(strncmp(p, ":include:", 9) == 0) {
			char *nl;
			p += 9;
			if((nl = index(p, '\n')) != NULL) {
				*nl = CNULL;
			}
DEBUG("load_alias '%s' includes file '%s'\n", filename, p);
			load_alias(head, p);
			continue;
		}

		/*
		**  if the first char on the line is a space or tab
		**  then it's a continuation line.  Otherwise,
		**  we start a new alias.
		*/
		if(*p != ' ' && *p != '\t') {
			b = p;
			SKIPWORD(p);
			*p++ = CNULL;
			/*
			** be sure that the alias is in local form
			*/
			if(islocal(b, domain, user) == 0) {
				/*
				** non-local alias format - skip it
				*/
				continue;
			}
			/*
			** add the alias to the (vertical) list of aliases
			*/
			if((h = add_vert(head, user)) == NNULL) {
DEBUG("load_alias for '%s' failed\n", b);
				return;
			}
		}
		/*
		**  Next on the line is the list of recipents.
		**  Strip out each word and add it to the
		**  horizontal linked list.
		*/
		recipients(h, p);
	}
	(void) fclose(fp);
}

/*
** add each word in a string (*p) of recipients
** to the (horizontal) linked list associated with 'h'
*/

recipients(h, p)
node *h;
char *p;
{

	char *b;
	int ret = 0;

	strip_comments(p);	/* strip out stuff in ()'s */

	SKIPSPACE(p);		/* skip leading whitespace on line */

	while((*p != NULL) && (*p != '#')) {
		b = p;
		if(*b == '"') {
			if((p = index(++b, '"')) == NULL) {
				/* syntax error - no matching quote */
				/* skip the rest of the line */
				return(ret);
			}
		} else {
			SKIPWORD(p);
		}

		if(*p != CNULL) {
			*p++ = CNULL;
		}

		add_horz(h, b);
		ret = 1;
		SKIPSPACE(p);
	}
	return(ret);
}

/*
** some aliases may have comments on the line like:
**
** moderators	moderator@somehost.domain	(Moderator's Name)
**		moderator@anotherhost.domain	(Another Moderator's Name)
**
** strip out the stuff in ()'s
**
*/

void
strip_comments(p)
char *p;
{
	char *b;
	while((p = index(p, '(')) != NULL) {
		b = p++;	/*
				** save pointer to open parenthesis
				*/
		if((p = index(p, ')')) != NULL) {/* look for close paren */
			(void) strcpy(b, ++p);	 /* slide string left    */
		} else {
			*b = CNULL;	/* no paren, skip rest of line  */
			break;
		}
	}
}

/*
** add_vert - add a (vertical) link to the chain of aliases.
*/

node *
add_vert(head, str)
node *head;
char *str;
{
	char *p, *malloc();
	void free();
	node *new;

	/*
	** strip colons off the end of alias names
	*/
	if((p = index(str, ':')) != NULL) {
		*p = CNULL;
	}
	if((new = (node *) malloc(sizeof(node))) != NNULL) {
		if((new->string = malloc((unsigned) strlen(str)+1)) == NULL) {
			free(new);
			new = NNULL;
		} else {
			(void) strcpy(new->string, str);
			new->vert   = head->vert;
			new->horz   = NNULL;
			head->vert  = new;
/*DEBUG("add_vert %s->%s\n", head->string, new->string);/* */
		}
	}
	return(new);
}

/*
** add_horz - add a (horizontal) link to the chain of recipients.
*/

void
add_horz(head, str)
node *head;
char *str;
{
	char *malloc();
	node *new;

	if((new = (node *) malloc(sizeof(node))) != NNULL) {
		if((new->string = malloc((unsigned) strlen(str)+1)) == NULL) {
			free(new);
			new = NNULL;
		} else {
			(void) strcpy(new->string, str);
			new->horz  = head->horz;
			new->vert  = NNULL;
			head->horz = new;
		}
/*DEBUG("add_horz %s->%s\n", head->string, new->string);/* */
	}
}

char *
tilde(user)
char *user;
{
	struct passwd *getpwent(), *pw;
	static node pwdstk;
	static int pw_eof = 0;

	node *a, *pwd;

	char buf[SMLBUF];
	int i;

	pwd = &pwdstk;

	/*
	** check for previously cached user
	*/

	if((a = hsearch(pwd, user)) != NNULL) {
		return(a->string + strlen(a->string) + 1);
	}

	/*
	** cache previous username and home directory
	** this bit of code is quite implementation dependent.
	** The kludge here is that the string login:home is
	** pushed onto a linked list, and then the ':' is
	** replaced with a null character.  This lets us
	** search the list without the hassel of index()'ing
	** on the ':' and doing strncmp().
	*/

	while((pw_eof == 0) && ((pw = getpwent()) != NULL)) {
		strcpy(buf, pw->pw_name);
		i = strlen(buf);
		buf[i] = ':';
		strcpy(&buf[i+1], pw->pw_dir);
		push(pwd, buf);
		a = pwd->horz;
		a->string[i] = '\0';
		if(strcmp(user, pw->pw_name) == 0) {
			return(a->string + strlen(a->string) + 1);
		}
	}
	pw_eof = 1;
	return(NULL);
}

node *
pop(head)
node *head;
{
	node *ret = NNULL;

	if(head != NNULL) {
		ret = head->horz;
		if(ret != NNULL) {
			head->horz = ret->horz;
		}
	}
	return(ret);
}
#endif
