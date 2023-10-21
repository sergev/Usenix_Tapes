#
#include	"ext.h"
/*

Name:

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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
			flg =| FCAT;
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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
trim(c)
int c;
{

        return(c&0177);
}

