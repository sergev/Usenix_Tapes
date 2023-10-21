#
#include	"ext.h"
main(c, av)
int c;
char **av;
{
        register f;
        register char **v;
	int tvec[2];
	int imain;
	int nrcmds;

	argv = av;
	gtty(0,ttys);
        setdflt();
        newbin();
        for(f=3; f<15; f++)
                close(f);
        dolc = getpid();
        for(f=4; f>=0; f--) {
                dolc = ldiv(0, dolc, 10);
                pidp[f] = ldivr+'0';
        }
        v = av;
/*
 *	get project number and opr for accounting
 *		use kludgy ptrs to save space
 */
	if(c>3 && v[1][0]=='-' && v[1][1]=='p') {
		projno = &projbuf;
		oprno = v[2];
		while(*projno++ = *oprno++);
		oprno = &oprbuf;
		projno = v[3];
		while(*oprno++ = *projno++);
		projno = &projbuf;
		oprno = &oprbuf;
		v[3] = v[2] = v[1] = v[0];
		v =+ 3;
		c =- 3;
	}
/*
 * special case for -e
 */
        if(c > 1 && v[1][0] == '-' && v[1][1] == 'e') {
                v[1] = v[0];
                v++;
                c--;
                eflag++;
        }
	gid = getgid() & 0377;
	acctf = open("/usr/adm/sh.act",1);
        promp = "% ";
        if(((uid = getuid())&0377) == 0) {
                promp = "# ";
		close(acctf);
		acctf = -1;
	}
        if(c > 1) {
                if(!eflag){
                        promp = 0;
		}
                if (*v[1]=='-') {
                        **v = '-';
                        if (v[1][1]=='c' && c>2)
                                arginp = v[2];
                        else if (v[1][1]=='t')
                                onelflg = 2;
                } else {
                        close(0);
                        f = open(v[1], 0);
                        if(f < 0) {
                                prs(v[1]);
                                err(": cannot open");
                                return(1);
                        }
                }
        }
        if(**v == '-') {
                setintr++;
                signal(QUIT, 1);
                signal(INTR, 1);
		if(v[0][1] == 'S') {		/* if original login call of shell */
			chgflag = 1;
			topshell++;
			if((f = open("start_up",0)) >= 0)	/* execute startup file */
				next(f);
		}
        }
	fstat(2,&statbuf);		/* get stats on standard input */
	if((devdir = open(DEVDIR,0)) < 0)	/* open directory to find ttyname */
			prs("\ncant open 'dev'");
	ttynm[8] = 'x';		/* set default ttyname */
	while(read(devdir,&ttybuf,16) == 16) {	/* read indiv device entry */
		if(statbuf.sinode == ttybuf.dinode) {	/* see if inodes match */
			if((ttybuf.tt1 == 't') && (ttybuf.tt2 == 't')	/* see if it's a tty */
			&& (ttybuf.yy == 'y') && (ttybuf.dend == '\0')
			&& (ttybuf.dname != '\0')) {
				ttynm[8] = ttybuf.dname;	/* get tty name */
				break;		/* get out */
			}
		}
	}
	ttyname = &ttynm;		/* set pointer to tty name */
        dolv = v+1;
        dolc = c-1;
        vbls['N'-'A'] = putn(dolc);

loop:
        if(promp != 0) {
                prs(promp);
		if(nrcmds++ > 20) {
			if(stat(mailbx,&statbuf) >= 0 && statbuf.size1) {
				prs("You have mail.\n");
				nrcmds = 0;
				prs(promp);
			}
		}
	}
        peekc = getch();
        main1();
        goto loop;
}

/*

Name:
	main1

Function:
	Gather and save input line.

Algorithm:
	Set up argument array and line pointers.  Take in command line, word by
	word.  Save it over previous line, unless CR or 'prev' command was found.
	Check for syntax and execute error-free command.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
main1()
{
        register char c, *cp;
        register *t;
	int i, ii;
	char *p1, *p2;

        argp = args;
        eargp = args+ARGSIZ-5;
        linep = line;
        elinep = line+LINSIZ-5;
        error = 0;
        gflg = 0;
	/* take in cmd line, word by word, */
	/* and set up arg ptrs in 'args' */
        do {
                cp = linep;
                word();
        } while(*cp != '\n');
	if((!equal(&line, "prev")) && (line[0] != '\n')) {	/* copy line as the previous command */
		i = ii = 0;			/* only if we're not doing 'prev' now */
		while(line[i] != '\n') {
			if(line[i] == '\0') {	/* substitute spaces for nulls at word ends */
				if(line[i-1] & 0177600)
					prev_line[ii++] = '\"';
				prev_line[ii] = ' ';
				if(line[i+1] & 0177600)
					prev_line[++ii] = '\"';
			}
			else
				prev_line[ii] = line[i]&0177;
			i++;
			ii++;
		}
		prev_line[ii++] = '\n';	/* finish up line */
		prev_line[ii++] = '\0';
	}
        treep = trebuf;
        treeend = &trebuf[TRESIZ];
        if(gflg == 0) {
                if(error == 0) {
                        setexit();
                        if (error)
                                return;
			/* analyze each word for function */
                        t = syntax(args, argp);
                }
                if(error != 0)
                        err("syntax error");
		else {
                        execute(t);
		}
        }
}

/*

Name:
	word

Function:
	Gather 'words' from command line.

Algorithm:
	Parse command line into words (arguments).  Handle separators (blank and
	tab), enclosures ('"), and terminators (&;<>|\n).  Character by character,
	input is actually done in 'pack' loop, until special case is found.
	Then 'loop' switch is called to process the case.

Parameters:
	None.

Returns:
	None.

Files and Programs:
	None.


*/
word()
{
        register char c, c1;

        *argp++ = linep;

loop:
        switch(c = getch()) {

        case ' ':
        case '\t':
                goto loop;

        case '\'':
        case '"':
                c1 = c;
                while((c=readc()) != c1) {
                        if(c == '\n') {
                                error++;
                                peekc = c;
                                return;
                        }
                        *linep++ = c|QUOTE;
                }
                goto pack;

        case '&':
        case ';':
        case '<':
        case '>':
        case '(':
        case ')':
        case '|':
        case '^':
        case '\n':
                *linep++ = c;
                *linep++ = '\0';
                return;
        }

        peekc = c;

pack:
        for(;;) {
                c = getch();
                if(any(c, " '\"\t;&<>()|^\n")) {
                        peekc = c;
                        if(any(c, "\"'"))
                                goto loop;
                        *linep++ = '\0';
                        return;
                }
                *linep++ = c;
        }
}

/*

Name:
	tree

Function:
	Get tree extension (sprig).

Algorithm:
	Check if requested amount of tree space is available.  If not, put out message
	and reset.

Parameters:
	Amount of tree needed.

Returns:
	Pointer to beginning of requested tree space or error reset.

Files and Programs:
	None.


*/
tree(n)
int n;
{
        register *t;

        t = treep;
        treep =+ n;
        if (treep>treeend) {
                prs("Command line overflow\n");
                error++;
                reset();
        }
        return(t);
}

/*

Name:
	getch

Function:
	Get a char from the input stream.

Algorithm:
	Return any previously read (peeked at) character.  Check for legal bounds
	(number of args and total line length).  Get character from variable buffer
	if reading from buffer.  Otherwise, read character from stadndard input.
	Process any escaped character.  Process any variable names (alpha, numeric, $) and set
	up pointers to continue to read from variable buffer.
	

Parameters:
	None.

Returns:
	Character

Files and Programs:
	None.


*/
getch()
{
        register char c;

        if(peekc) {
                c = peekc;
                peekc = 0;
                return(c);
        }
        if(argp > eargp) {
                argp =- 10;
                while((c=getch()) != '\n');
                argp =+ 10;
                err("Too many args");
                gflg++;
                return(c);
        }
        if(linep > elinep) {		/* if line too long */
                linep =- 10;
                while((c=getch()) != '\n');
                linep =+ 10;
                err("Too many characters");
                gflg++;
                return(c);
        }
getd:
        if(dolp) {		/* if we're inputting from $ variable*/
                c = *dolp++;
                if(c != '\0') {		/* middle of variable */
                        echo(c);
                        return(c);
                }
                dolp = 0;		/* end of variable */
                echo(echoff);
        }
        c = readc();
        if(c == '\\') {			/* if char is escaped with backslash */
                c = readc();		/* get real char */
                if(c == '\n')
                        return(' ');
                return(c|QUOTE);
        }
        if(c == '$') {		/* we may be starting to take input */
				/* fr variable or arg.  If so, set the */
				/* dolp ptr to the proper variable buffer */
				/* and go to 'getd' to get the 1st char */
				/* of the variable */
                c = readc();
                if(c>='0' && c<='9') {		/* it's a cmd line arg */
                        if(c-'0' < dolc) {
                                dolp = dolv[c-'0'];
                                echo(echon);
                        }
                        goto getd;
                }
                if(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {	/* it's a variable */
                        if(c >= 'a')
                                c =+ 'Z'+1 - 'a';
                        if(vbls[c-'A'] == 0)
                                goto getd;      /* undefined */
                        dolp = vbls[c-'A'];
                        echo(echon);
                        goto getd;
                }
                if(c == '$') {		/* it's the shell's process id */
                        dolp = pidp;
                        echo(echon);
                        goto getd;
                }
        }
        return(c&0177);
}

/*

Name:
	readc

Function:
	Perform character read.

Algorithm:
	Take next charcter from input argument pointer, standard input, next
	file, or previous line buffer.  If end of next file, reset file descriptors
	to standard input.

Parameters:
	None.

Returns:
	Character or exit.

Files and Programs:
	None.


*/
readc()
{
        char cc;
        register c;

        if (arginp) {
                if (arginp == 1)
                        exit();
                if ((c = *arginp++) == 0) {
                        arginp = 1;
                        c = '\n';
                }
                echo(c);
                return(c);
        }
        if (onelflg==1)
                exit();
	if(!prev_flag) {	/* if we're working on a new line, read from 0 */
        	if(read(0, &cc, 1) != 1) {
                	if(sinfil) {    /* end of next file */
                        	close(0);
                        	dup(sinfil);
                        	close(sinfil);
                        	sinfil = 0;
                        	if(setintr)
                                	signal(INTR,1);
                        	if(promp = rpromp)
                                	prs(promp);
                        	return(readc());
	                }		
			/* end of standard input */
			batch();	/* wait for batch jobs */
	        }
	}
	else {		/* otherwise, read from previous line buffer */
		cc = *p_linep++;
	}
        if (cc=='\n' && onelflg)
                onelflg--;
        echo(cc);
        return(cc);
}

