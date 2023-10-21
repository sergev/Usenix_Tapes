#
#include	"ext.h"
/*
Name:
	syntax

Function:
	Start syntactical check of command line and build compile tree.

Algorithm:
	Scan for null statement (plain or fancy).  Otherwise, go to first level
	syntax scan.
	Tree structure will be set up as follows:

	Basic unit:
		0	DTYP	Command type (*)
		1	DLEF	Pointer to left side sprig of command list tree (cmd&cmd cmd;cmd cmd|cmd)
				or pointer to redirected std input file name
		2	DRIT	Pointer to right side sprig of command list tree (cmd&cmd cmd;cmd cmd|cmd)
				or pointer to redirected std output file name
		3	DERR	Pointer to redirected std error output file name
		4	DFLG	Descriptive flag (**)
		5	DSPR	Pointer to next tree 'sprig'
		6	DCOM	Pointer to command name

	* Types for DCOM:
		TCOM	Bottom level command
		TPAR	Start of paren expression
		TFIL	Piped commands (need further processing by 'execute()')
		TLST	List of commands (separated by &, ;, and \n)

	** Flags for DFLG:
		FAND	Command is ampersanded off into background
		FCAT	Appended redirection of std output (>>)
		FCTE	Appended redirection of std error output (^^)
		FPAR	End of paren expression
		FINT	Interrups should be masked
		FPRS	Print process id (command was ampersanded off)
		FPIN	Piped input (Set by 'execute()')
		FPOU	Piped output (Set by 'execute()')

Parameters:
	Pointer to list of arguments
	Pointer to end of arguments

Returns:
	Pointer to tree position.
	0 = error

Files and Programs:
	None.


*/
/*
 * syntax
 *      empty
 *      syn1
 */

syntax(p1, p2)
char **p1, **p2;
{

        while(p1 != p2) {
                if(any(**p1, ";&\n"))		/* if fancy null statement */
                        p1++;
		else
                        return(syn1(p1, p2));
        }
        return(0);			/* if null statement */
}

/*

Name:
	syn1

Function:
	Perform first level syntactical scan of command line.

Algorithm:
	Scan for lower level delimiters (parens) and set level balance variable.
	Scan for line terminators (\n&;).  If found, 1) reserve tree space, 2)
	analyze rest of line (left side), and 3) analyze right side of line terminator
	from the top.  Otherwise, anaylze like left side.

Parameters:
	Pointer to list of arguments.
	Pointer to end of arguments.

Returns:
	Pointer to tree position.

Files and Programs:
	None.


*/
/*
 * syn1
 *      syn2
 *      syn2 & syntax
 *      syn2 ; syntax
 */

syn1(p1, p2)
char **p1, **p2;
{
        register char **p;
        register *t, *t1;
        int l;

        l = 0;
        for(p=p1; p!=p2; p++)
        switch(**p) {

        case '(':
                l++;
                continue;

        case ')':
                l--;
                if(l < 0)
                        error++;
                continue;

        case '&':
        case ';':
        case '\n':
                if(l == 0) {
                        l = **p;
                        t = tree(DCOM-1);
                        t[DTYP] = TLST;
                        t[DLEF] = syn2(p1, p);
                        t[DFLG] = 0;
                        if(l == '&') {
                                t1 = t[DLEF];
                                t1[DFLG] =| FAND|FPRS|FINT;
                        }
                        t[DRIT] = syntax(p+1, p2);
                        return(t);
                }
        }
        if(l == 0)
                return(syn2(p1, p2));
        error++;
}

/*

Name:
	syn2

Function:
	Perform second level syntactical scan of command line.

Algorithm:
	Scan for lower level delimiters (parens).  Set level balance variable.
	Scan for command separator (pipe).  If found, 1) finish analysis of left
	side and 2) rescan right hand side from the top.  Otherwise, finish analysis
	on line.

Parameters:
	Pointer to list of arguments.
	Pointer to end of arguments.

Returns:
	Pointer to tree position.

Files and Programs:
	None.


*/
/*
 * syn2
 *      syn3
 *      syn3 | syn2
 */

syn2(p1, p2)
char **p1, **p2;
{
        register char **p;
        register int l, *t;

        l = 0;
        for(p=p1; p!=p2; p++)
        switch(**p) {

        case '(':
                l++;
                continue;

        case ')':
                l--;
                continue;

        case '|':
                if(l == 0) {
                        t = tree(DCOM-1);
                        t[DTYP] = TFIL;
                        t[DLEF] = syn3(p1, p);	/* analyze from begin to hre */
                        t[DRIT] = syn2(p+1, p2);	/* analyze from here to end */
                        t[DFLG] = 0;
                        return(t);
                }
        }
        return(syn3(p1, p2));			/* analyze from here to end */
}

/*

Name:
	syn3

Function:
	Finish syntactical scan of command line.

Algorithm:
	Scan for I/O redirection and check for correct numbers.  If inside parens,
	reserve room on tree and rescan command line within parens.  Otherwise,
	reserve room on the tree and fill in tree entries.

Parameters:
	Pointer to list of arguemnts.
	Pointer to end of arguments.

Returns:
	Pointer to tree position.

Files and Programs:
	None.


*/
/*
 * syn3
 *      ( syn1 ) [ < in  ] [ > out ]
 *      word word* [ < in ] [ > out ]
 */

syn3(p1, p2)
char **p1, **p2;
{
        register char **p;
        char **lp, **rp;
        register *t;
        int n, l, i, o, c, flg, e;

        flg = 0;
        if(**p2 == ')')
                flg =| FPAR;
        lp = 0;
        rp = 0;
        i = 0;
        o = 0;
	e = 0;
        n = 0;
        l = 0;
        for(p=p1; p!=p2; p++)
        switch(c = **p) {

        case '(':
                if(l == 0) {
                        if(lp != 0)
                                error++;
                        lp = p+1;
                }
                l++;
                continue;

        case ')':
                l--;
                if(l == 0)
                        rp = p;
                continue;

        case '>':			/* redirect standard output */
                p++;			/* check next arg */
                if(p!=p2 && **p=='>')		/* if >>, set concatenate (append) flag */
                        flg =| FCAT;
		else				/* otherwise, back up to > */
                        p--;
		goto redio;
	case '^':			/* redirect error output */
		p++;				/* check next arg */
		if(p != p2 && **p == '^')		/* if ^^, set concatenate (append) flag */
			flg =| FCTE;
		else			/* otherwise, back up to ^ */
			p--;
	redio:
        case '<':				/* redirect standard input */
                if(l == 0) {		/* if at top level of parens */
                        p++;
                        if(p == p2) {	/* if no further args, we're missing filename */
                                error++;
                                p--;
                        }
                        if(any(**p, "^<>("))	/* if more redirection ahead, it's too many */
                                error++;
			switch(c) {
			case '<':
				if(i != 0)	/* ought to have only one so far */
					error++;
				i = *p;
				break;
			case '>':
				if(o != 0)	/* ought to have only one so far */
					error++;
				o = *p;		/* set output */
				break;
			case '^':
				if(e != 0)	/* ought to have only one so far */
					error++;
				e = *p;
				break;
			}
                }
                continue;

        default:
                if(l == 0)
                        p1[n++] = *p;
        }
        if(lp != 0) {
                if(n != 0)
                        error++;
                t = tree(DCOM);
                t[DTYP] = TPAR;
                t[DSPR] = syn1(lp, rp);
                goto out;
        }
        if(n == 0)
                error++;
        p1[n++] = 0;
        t = tree(n+DCOM);
        t[DTYP] = TCOM;
        for(l=0; l<n; l++)
                t[l+DCOM] = p1[l];
out:
        t[DFLG] = flg;
        t[DLEF] = i;
        t[DRIT] = o;
	t[DERR] = e;
        return(t);
}

/*

Name:
	scan

Function:
	Scan arguments and perform action on each one.

Algorithm:
	Invoke scanning function for each argument in the tree.

Parameters:
	Tree pointer.
	Function to be used for scanning.

Returns:
	None.

Files and Programs:
	None.


*/
scan(at, f)
int *at;
int (*f)();
{
        register char *p, c;
        register *t;

        t = at+DCOM;
        while(p = *t++)
                while(c = *p)
                        *p++ = (*f)(c);
}

/*

Name:
	tglob

Function:
	Test for global characters.

Algorithm:
	If character matches global character set, set global flag.

Parameters:
	Character to check.

Returns:
	Character being checked.

Files and Programs:
	None.


*/
tglob(c)
int c;
{

        if(any(c, "[?*"))
                gflg = 1;
        return(c);
}

/*

Name:
	trim

Function:
	Strip 8th bit off characters.

Algorithm:
	AND off all but the least significant 7 bits.

Parameters:
	Character to be stripped.

Returns:
	Stripped character.

Files and Programs:
	None.


*/
trim(c)
int c;
{

        return(c&0177);
}

