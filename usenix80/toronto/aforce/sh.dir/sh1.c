#
#include	"ext.h"
/*
 *	UNIX SHELL
 *
 *	Modified by John Levine, Yale U., 1976
 *		Added variables $a-$Z
 *		Set default values for variables
 *			$P	decimal process id of most recent execution
 *			$$	octal process id of this shell
 *			$M	prompt
 *			$N	number of arguments
 *		Added private bin directory search and newbin command
 *		Changed wait command to address specific process id
 *		Added -e for echo  of commands as executed
 *		Added -t -c for one line input
 *		Added logout command
 *		Added next command and start_up file handling
 *		Added cd as chdir command and automatic homing
 *	Modified by Walter Lazear, AFDSC, Jun 1977
 *		Removed 'home' command (Yale dependent)
 *		Added prev command for command editting
 *		Changed chdir homing to print home directory
 *		Set default for variables
 *			$T	original standard input tty
 *			$U	user's login name
 *			$D	login day/month (numeric)
 *			$H	users home directory
 *			$C	command just executed
 *			$W	day of the week
 *			$O	old working directory
 *			$K	current working directory
			$J	current project id
 *		Added logo, logoff, and bye command
 *		Added set command operator (:) for file input
 *	Modified by Walter Lazear, Sept 1977
 *		Added -p project id flag
 *		Changed accounting records for project
 *		Added newproj command
 *		Added session cost printing
 *		Added chg, nochg, and cost commands
 *		Changed login and newgrp to log out of shell
 *		Set default for $J as project
 *		Changed exits to 'batch' mode processing
 *	Modified by Walter Lazear, Dec 1978
 *		Changed -p flag to accept OPR
 *		Added disk quota processing.
 *		Streamlined acct records.
 *		Made newproj and cost separate programs.
 *		Added 'c' to prev.
 *		Changed batch to not wait (hung rotary).
 *		Added check for mail at logout.
 *		Set default for variables
 *			$R	current project opr
 *			$D	login day
 *			$X	login month (numeric)
 *			$Y	login year (00-99)
*		Removed $C (current command) and $O (old directory)
 *		Modified 'texec' to use /bin shell files.
 *		Added mail check every 20 prompts.
 *		Fixed prev's scan to look inside quotes.
 *		Prevented previous line from being wiped out by empty cmd
 *		Redirected error output (fd 2) with ^.
 *		Syncronized cost and quota in batch.
 */
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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


*/
main1()
{
        register char c, *cp;
        register *t;
	int i;
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
		i = 0;			/* only if we're not doing 'prev' now */
		while(line[i] != '\n') {
			if(line[i] == '\0')	/* substitute spaces for nulls at word ends */
				prev_line[i] = ' ';
			else
				prev_line[i] = line[i]&0177;
			i++;
		}
		prev_line[i++] = '\n';	/* finish up line */
		prev_line[i++] = '\0';
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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

Function:

Algorithm:

Parameters:

Returns:

Files and Programs:


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

