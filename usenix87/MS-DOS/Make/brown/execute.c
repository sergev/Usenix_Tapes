#include <errno.h>
#include <bdos.h>
#include <stdio.h>
#include <ctype.h>
#include "make.h"


#define FALSE 0
#define TRUE 1
#define BUFFSIZ 256

char *searchpath(), *getenv();
extern	int	_envseg;
static char cmd[128], param[128];
static char *cmds[] = {
	"break",
	"chdir",
	"cd",
	"cls",
	"copy",
	"ctty",
	"date",
	"del",
	"erase",
	"dir",
	"echo",
	"exit",
	"for",
	"goto",
	"if",
	"mkdir",
	"md",
	"path",
	"pause",
	"prompt",
	"rem",
	"ren",
	"rename",
	"rmdir",
	"rd",
	"set",
	"shift",
	"time",
	"type",
	"ver",
	"verify",
	"vol",
	0 };

execute (str, noexecflag)
char *str;
int noexecflag;
{
    char tmp[BUFFSIZ], buf[10];
    int index = 0, rval;
    extern int ignore_errors;
    
    if (noexecflag) {
        fputs(str, stdout);
	return 0;
    };
    while (*str != '\0') {
        if (*str == '\n') {
	    tmp[index] = '\0';
	    index = 0;
	    str++;
	    if ((rval = run(tmp)) != 0 && !ignore_errors) {
	        fputs("***Error Code ", stderr);
		itoa(rval, buf);
		fputs(buf, stderr);
		fputc('\n', stderr);
	        return rval;
	    }
	}
	else if (index == (BUFFSIZ - 1)) {
	    fputs("Command Too Long: ", stderr);
	    fputs(str, stderr);
	    fputs("\nShorten.\n", stderr);
	    return -1;
	} 
	else {
	    tmp[index++] = *str++;
	}    
    }
    return 0;
}
    
    

#ifdef TESTING
main()
{
        char temp[128];

	for (;;) {
		printf("Command: ");
		gets(temp);
	if (temp[0] == '\0') break;
		printf("        Execute: %d\n",run(temp));
	}
}
#endif

/* run(str)
 * char *str;
 *		returns the value of the executed command.  If the command is
 *		an MS-DOS resident command the command.com is executed else
 *		the program is invoked (looking down the PATH).
 *
 *		Written: Bradley N. Davis  University of Utah VCIS group
 *		Date: 4-Jan-84
 *
 */

static
run(str)
char *str;
{
	struct	execp	ep;
	struct	SREGS	segs;

	puts(str);
#ifdef NOREALEXECUTE
	return 0;
#else
	if (str[0] == '\0') {		/* Blank Line? push to subshell */
		strcpy(cmd, getenv("COMSPEC"));
		param[0] = '\0';
		segread(&segs);
		ep.ex_envseg = _envseg;
		ep.ex_cmdadd = (unsigned)param;
		ep.ex_cmdseg = segs.ss;
		ep.ex_fcb1ad = 0;
		ep.ex_fcb1sg = 0;
		ep.ex_fcb2ad = 0;
		ep.ex_fcb2sg = 0;
		return (exec(cmd, 0, &ep));
	}
	if (resident(str))
		return(system(str));
	return(program(cmd,param));
#endif
}

/*
 * resident(str)
 * char *str;
 *		returns true if the command in str is an MS-DOS resident
 *		command.
 *
 *		Written: Bradley N. Davis  University of Utah VCIS group
 *		Date: 4-Jan-84
 *
 */
#define iswhite(ch) ( ch == ' ' || ch == '\t' )
static
resident(str)
char *str;
{
	char **t, *strpbrk();
	int i, j;

	while (iswhite(*str)) str++;		/* trim blanks */
	if (str[1] == ':' && isalpha(str[0]))		/* look for x: */
	    return TRUE;
	if (strpbrk(str, "<>|") != NULL)	/* redirection? use system */
	    return TRUE;
	i = 0;
	while ( isalnum(*str))
		if (isupper(*str))
			cmd[i++] = *str++ - 'A' + 'a';
		else
			cmd[i++] = *str++;
	cmd[i] = '\0';
	for (t = cmds; *t; t++) {
		if (strcmp(*t,cmd) == 0) return TRUE;
	}
	strcat(cmd, ".bat");			/* Batch file? use system */
	if (searchpath(cmd) != 0) {
		cmd[i] = '\0';
		return TRUE;
	}
	cmd[i] = '\0';
	j = strlen(str);
	i = 1;
	while ((param[i++] = *str++) != 0);
	param[0] = j;
	param[j+1] = '\r';
	return FALSE;
}

static
program(cmd,param)
char	*cmd, *param;
{
	struct	execp	ep;
	struct	SREGS	segs;
	char	*pathp;
	int	len;

	len = strlen(cmd);
	strcat(cmd, ".com");
	pathp = searchpath(cmd);
	if (pathp == 0) {
		cmd[len] = '\0';
		strcat(cmd, ".exe");
		pathp = searchpath(cmd);
		if (pathp == 0) {
			cmd[len] = '\0';
			errno = ENOENT;
			return(-1);
		}
	}
	segread(&segs);
	ep.ex_envseg = _envseg;
	ep.ex_cmdadd = (unsigned)param;
	ep.ex_cmdseg = segs.ss;
	ep.ex_fcb1ad = 0;
	ep.ex_fcb1sg = 0;
	ep.ex_fcb2ad = 0;
	ep.ex_fcb2sg = 0;
	return (exec(pathp, 0, &ep));
}
