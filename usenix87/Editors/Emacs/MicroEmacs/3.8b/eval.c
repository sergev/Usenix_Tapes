/*	EVAL.C:	Expresion evaluation functions for
		MicroEMACS

	written 1986 by Daniel Lawrence				*/

#include	<stdio.h>
#include	"estruct.h"
#include	"edef.h"
#include	"evar.h"

#if	MEGAMAX & ST520
overlay	"eval"
#endif

char value[80];		/* buffer to return value in */

varinit()		/* initialize the user variable list */

{
	register int i;

	for (i=0; i < MAXVARS; i++)
		uv[i].u_name[0] = 0;
}

char *gtfun(fname)	/* evaluate a function */

char *fname;		/* name of function to evaluate */

{
	register int fnum;		/* index to function to eval */
	register int status;		/* return status */
	char arg1[NSTRING];		/* value of first argument */
	char arg2[NSTRING];		/* value of second argument */
	char arg3[NSTRING];		/* value of third argument */
	static char result[2 * NSTRING];	/* string result */

	/* look the function up in the function table */
	fname[3] = 0;	/* only first 3 chars significant */
	for (fnum = 0; fnum < NFUNCS; fnum++)
		if (strcmp(fname, funcs[fnum].f_name) == 0)
			break;

	/* return errorm on a bad reference */
	if (fnum == NFUNCS)
		return(errorm);

	/* retrieve the first argument */
	if ((status = macarg(arg1)) != TRUE)
		return(errorm);

	/* if needed, retrieve the second argument */
	if (funcs[fnum].f_type >= DYNAMIC) {
		if ((status = macarg(arg2)) != TRUE)
			return(errorm);

		/* if needed, retrieve the third argument */
		if (funcs[fnum].f_type >= TRINAMIC)
			if ((status = macarg(arg3)) != TRUE)
				return(errorm);
	}

	/* and now evaluate it! */
	switch (fnum) {
		case UFADD:	return(itoa(atoi(arg1) + atoi(arg2)));
		case UFSUB:	return(itoa(atoi(arg1) - atoi(arg2)));
		case UFTIMES:	return(itoa(atoi(arg1) * atoi(arg2)));
		case UFDIV:	return(itoa(atoi(arg1) / atoi(arg2)));
		case UFMOD:	return(itoa(atoi(arg1) % atoi(arg2)));
		case UFNEG:	return(itoa(-atoi(arg1)));
		case UFCAT:	strcpy(result, arg1);
				return(strcat(result, arg2));
		case UFLEFT:	return(strncpy(result, arg1, atoi(arg2)));
		case UFRIGHT:	return(strcpy(result, &arg1[atoi(arg2)-1]));
		case UFMID:	return(strncpy(result, &arg1[atoi(arg2)-1],
					atoi(arg3)));
		case UFNOT:	return(ltos(stol(arg1) == FALSE));
		case UFEQUAL:	return(ltos(atoi(arg1) == atoi(arg2)));
		case UFLESS:	return(ltos(atoi(arg1) < atoi(arg2)));
		case UFGREATER:	return(ltos(atoi(arg1) > atoi(arg2)));
		case UFSEQUAL:	return(ltos(strcmp(arg1, arg2) == 0));
		case UFSLESS:	return(ltos(strcmp(arg1, arg2) < 0));
		case UFSGREAT:	return(ltos(strcmp(arg1, arg2) > 0));
		case UFIND:	return(getval(arg1));
	}

	exit(-11);	/* never should get here */
}

char *gtusr(vname)	/* look up a user var's value */

char *vname;		/* name of user variable to fetch */

{

	register int vnum;	/* ordinal number of user var */

	/* scan the list looking for the user var name */
	for (vnum = 0; vnum < MAXVARS; vnum++)
		if (strcmp(vname, uv[vnum].u_name) == 0)
			break;

	/* return errorm on a bad reference */
	if (vnum == MAXVARS)
		return(errorm);

	return(uv[vnum].u_value);
}

char *gtenv(vname)

char *vname;		/* name of environment variable to retrieve */

{
	register int vnum;	/* ordinal number of var refrenced */

	/* scan the list, looking for the referenced name */
	for (vnum = 0; vnum < NEVARS; vnum++)
		if (strcmp(vname, envars[vnum]) == 0)
			break;

	/* return errorm on a bad reference */
	if (vnum == NEVARS)
		return(errorm);

	/* otherwise, fetch the appropriate value */
	switch (vnum) {
		case EVFILLCOL:	return(itoa(fillcol));
		case EVPAGELEN:	return(itoa(term.t_nrow + 1));
		case EVCURCOL:	return(itoa(getccol(FALSE)));
		case EVCURLINE: return(itoa(getcline()));
		case EVRAM:	return(itoa((int)(envram / 1024l)));
		case EVFLICKER:	return(ltos(flickcode));
		case EVCURWIDTH:return(itoa(term.t_nrow));
		case EVCBUFNAME:return(curbp->b_bname);
		case EVCFNAME:	return(curbp->b_fname);
		case EVSRES:	return(sres);
		case EVDEBUG:	return(ltos(macbug));
		case EVSTATUS:	return(ltos(cmdstatus));
	}
}

int setvar(f, n)		/* set a variable */

int f;		/* default flag */
int n;		/* numeric arg (can overide prompted value) */

{
	register int vnum;	/* ordinal number of var refrenced */
	register int status;	/* status return */
	register int vtype;	/* type of variable to set */
	register char * sp;	/* scratch string pointer */
	char var[NVSIZE+1];	/* name of variable to fetch */
	char value[NSTRING];	/* value to set variable to */

	/* first get the variable to set.. */
	if (clexec == FALSE) {
		status = mlreply("Variable to set: ", &var[0], NVSIZE);
		if (status != TRUE)
			return(status);
	} else {	/* macro line argument */
		/* grab token and skip it */
		execstr = token(execstr, var);
	}

	/* check the legality and find the var */
sv01:	vtype = -1;
	switch (var[0]) {

		case '$': /* check for legal enviromnent var */
			for (vnum = 0; vnum < NEVARS; vnum++)
				if (strcmp(&var[1], envars[vnum]) == 0) {
					vtype = TKENV;
					break;
				}
			break;

		case '%': /* check for existing legal user variable */
			for (vnum = 0; vnum < MAXVARS; vnum++)
				if (strcmp(&var[1], uv[vnum].u_name) == 0) {
					vtype = TKVAR;
					break;
				}
			if (vnum < MAXVARS)
				break;

			/* create a new one??? */
			for (vnum = 0; vnum < MAXVARS; vnum++)
				if (uv[vnum].u_name[0] == 0) {
					vtype = TKVAR;
					strcpy(uv[vnum].u_name, &var[1]);
					break;
				}
			break;

		case '&':	/* indirect operator? */
			var[4] = 0;
			if (strcmp(&var[1], "ind") == 0) {
				/* grab token, and eval it */
				execstr = token(execstr, var);
				strcpy(var, getval(var));
				goto sv01;
			}
	}

	/* if its not legal....bitch */
	if (vtype == -1) {
		mlwrite("%%No such variable");
		return(FALSE);
	}

	/* get the value for that variable */
	if (f == TRUE)
		strcpy(value, itoa(n));
	else {
		status = mlreply("Value: ", &value[0], NSTRING);
		if (status != TRUE)
			return(status);
	}

	/* and set the appropriate value */
	status = TRUE;
	switch (vtype) {
	case TKVAR: /* set a user variable */
		if (uv[vnum].u_value != NULL)
			free(uv[vnum].u_value);
		sp = malloc(strlen(value) + 1);
		if (sp == NULL)
			return(FALSE);
		strcpy(sp, value);
		uv[vnum].u_value = sp;
		break;

	case TKENV: /* set an environment variable */
		status = TRUE;	/* by default */
		switch (vnum) {
		case EVFILLCOL:	fillcol = atoi(value);
				break;
		case EVPAGELEN:	status = newsize(TRUE, atoi(value));
				break;
		case EVCURCOL:	status = setccol(atoi(value));
				break;
		case EVCURLINE:	status = gotoline(TRUE, atoi(value));
				break;
		case EVRAM:	break;
		case EVFLICKER:	flickcode = stol(value);
				break;
		case EVCURWIDTH:status = newwidth(TRUE, atoi(value));
				break;
		case EVCBUFNAME:strcpy(curbp->b_bname, value);
				curwp->w_flag |= WFMODE;
				break;
		case EVCFNAME:	strcpy(curbp->b_fname, value);
				curwp->w_flag |= WFMODE;
				break;
		case EVSRES:	status = TTrez(value);
				break;
		case EVDEBUG:	macbug = stol(value);
				break;
		case EVSTATUS:	cmdstatus = stol(value);
				break;
		}
		break;
	}
	return(status);
}

/*	atoi:	ascii string to integer......This is too
		inconsistant to use the system's	*/

atoi(st)

char *st;

{
	int result;	/* resulting number */
	int sign;	/* sign of resulting number */
	char c;		/* current char being examined */

	result = 0;
	sign = 1;
	while ((c = *st++)) {
		if (c == '-')
			sign *= -1;
		if (c >= '0' && c <= '9')
			result = result * 10 + c - '0';
	}

	return(result * sign);
}

/*	itoa:	integer to ascii string.......... This is too
		inconsistant to use the system's	*/

char *itoa(i)

int i;	/* integer to translate to a string */

{
	register int digit;		/* current digit being used */
	register char *sp;		/* pointer into result */
	register int sign;		/* sign of resulting number */
	static char result[INTWIDTH+1];	/* resulting string */

	/* eliminate the trivial 0 */
	if (i == 0)
		return("0");

	/* record the sign...*/
	sign = 1;
	if (i < 0) {
		sign = -1;
		i = -i;
	}

	/* and build the string (backwards!) */
	sp = result + INTWIDTH;
	*sp = 0;
	while (i) {
		digit = i % 10;
		*(--sp) = '0' + digit;	/* and install the new digit */
		i = i / 10;
	}

	/* and fix the sign */
	if (sign == -1) {
		*(--sp) = '-';	/* and install the minus sign */
	}

	return(sp);
}

int gettyp(token)	/* find the type of a passed token */

char *token;	/* token to analyze */

{
	register char c;	/* first char in token */

	/* grab the first char (this is all we need) */
	c = *token;

	/* no blanks!!! */
	if (c == 0)
		return(TKNUL);

	/* a numeric literal? */
	if (c >= '0' && c <= '9')
		return(TKLIT);

	switch (c) {
		case '"':	return(TKSTR);

		case '!':	return(TKDIR);
		case '@':	return(TKARG);
		case '#':	return(TKBUF);
		case '$':	return(TKENV);
		case '%':	return(TKVAR);
		case '&':	return(TKFUN);
		case '*':	return(TKLBL);

		default:	return(TKCMD);
	}
}

char *getval(token)	/* find the value of a token */

char *token;		/* token to evaluate */

{
	register int status;	/* error return */
	register BUFFER *bp;	/* temp buffer pointer */
	register int blen;	/* length of buffer argument */
	char buf[NSTRING];	/* string buffer for some returns */

	switch (gettyp(token)) {
		case TKNUL:	return("");

		case TKARG:	/* interactive argument */
				status = getstring(getval(&token[1]),
					   buf, NSTRING, ctoec('\n'));
				if (status == ABORT)
					return(errorm);
				return(buf);

		case TKBUF:	/* buffer contents fetch */

				/* grab the right buffer */
				bp = bfind(getval(&token[1]), FALSE, 0);
				if (bp == NULL)
					return(errorm);
		
				/* make sure we are not at the end */
				if (bp->b_linep == bp->b_dotp)
					return(errorm);
		
				/* grab the line as an argument */
				blen = bp->b_dotp->l_used;
				if (blen > NSTRING)
					blen = NSTRING;
				strncpy(buf, bp->b_dotp->l_text, blen);
				buf[blen] = 0;
		
				/* and step the buffer's line ptr ahead a line */
				bp->b_dotp = bp->b_dotp->l_fp;
				bp->b_doto = 0;

				/* and return the spoils */
				return(buf);		

		case TKVAR:	return(gtusr(token+1));
		case TKENV:	return(gtenv(token+1));
		case TKFUN:	return(gtfun(token+1));
		case TKDIR:	return(errorm);
		case TKLBL:	return(itoa(gtlbl(token)));
		case TKLIT:	return(token);
		case TKSTR:	return(token+1);
		case TKCMD:	return(token);
	}
}

gtlbl(token)	/* find the line number of the given label */

char *token;	/* label name to find */

{
	return(1);
}

int stol(val)	/* convert a string to a numeric logical */

char *val;	/* value to check for stol */

{
	/* check for logical values */
	if (val[0] == 'F')
		return(FALSE);
	if (val[0] == 'T')
		return(TRUE);

	/* check for numeric truth (!= 0) */
	return((atoi(val) != 0));
}

char *ltos(val)		/* numeric logical to string logical */

int val;	/* value to translate */

{
	if (val)
		return(truem);
	else
		return(falsem);
}
