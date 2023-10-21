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
execute(t, pf1, pf2)
int *t, *pf1, *pf2;
{
	char c;
        int i, f, pv[2];
        register *t1;
        register char *cp1, *cp2, *cp3;
        extern errno;
        int unnext();

        if(t != 0)
        switch(t[DTYP]) {

        case TCOM:
                cp1 = t[DCOM];
                if(equal(cp1, "chdir")||equal(cp1,"cd")) {
                        if(t[DCOM+1] != 0) {
                                if(chdir(t[DCOM+1]) < 0)
                                        err("chdir: bad directory");
				else {
					copyit(t[DCOM+1],curwkdir);
				}
                        }
			else {
                        	if(chdir(pwbuf) < 0) {
                                	err("chdir: bad directory");
				}
				else {	/* print home directory */
						prs(&pwbuf);
						prs("\n");
					copyit(&pwbuf,curwkdir);
				}
			}
                        return;
                }
                if(equal(cp1, "newbin")) {
                        newbin();
                        return;
                }
                if(equal(cp1, "set")) {
                        doset(t+DCOM);
                        return;
                }
                if(equal(cp1, "shift")) {
                        if(dolc < 1) {
                                prs("shift: no args\n");
                                return;
                        }
                        dolv[1] = dolv[0];
                        dolv++;
                        dolc--;
                        xfree(vbls['N'-'A']);
                        vbls['N'-'A'] = putn(dolc);
                        return;
                }
		if(equal(cp1, "prev")) {	/* edit previous command */
			if(promp != 0) {
				prs(&prev_line);
				cp1 = prev_line;	/* init ptrs */
				cp2 = line;
				p_linep = prev_line;
				gtty(0,ttys);	/* get current settings */
				chgmode();	/* work on indiv chars */
				while((c = getchar()) != 'x') {	/* until exit/execute */
					switch(c) {
					/* print next char */
					case ' ':
						if(*cp1 == '\n')	/* don't go past end of line */
							goto recycle;
						putchar(*cp1);	/* print and go forth */
						*cp2++ = *cp1++;	/* copy char */
						break;
					/* scan line for next input char */
					case 's':
						c = getchar();	/* get search char */
						if(*cp1 == c) {			/* if we're sitting on it */
							putchar(*cp1);		/* print it */
							*cp2++ = *cp1++;	/* and move around it */
						}
						while(*cp1 != c ) {	/* until found */
							if(*cp1 == '\n') {	/* if end of line */
								goto recycle;
							}
							putchar(*cp1);	/* print */
							*cp2++ = *cp1++;	/* and move */
						}
						break;
					/* delete next char, show between #'s */
					case 'd':
						if(*cp1 == '\n')		/* if at end of line */
							goto recycle;
						putchar('#');
						putchar(*cp1++);
						putchar('#');
						break;
					/* delete previous char, show between #'s */
					case 'e':
						if(cp2 == &line)		/* if at begin of line */
							break;
						putchar('#');
						--cp2;
						putchar(*cp2);
						putchar('#');
						break;
					/* insert string until ctl-d */
					case 'i':
						while((c = getchar()) != '\004') {
							if(c == '\010') {	/* if a backspace */
								--cp2;	/* back up */
								putchar(c);
								continue;
							}
							if(c != '\n') {	/* don't allow multi-lines */
								putchar(c);	/* print char */
								*cp2++ = c;
							}
						}
						break;
					/* back up one char */
					case '\010':
						if((cp2 == &line) || (cp1 == prev_line))	/* if at begin of line */
							break;
						--cp2;
						--cp1;
						*cp1 = *cp2;	/* save backed-over char */
						putchar('/');
						putchar(*cp2);
						putchar('/');
						break;
					/* change next char */
					case 'c':
						c = getchar();
						if(c == '\n')
							break;
						cp1++;
						*cp2++ = c;
						putchar(c);
						break;
					/* quit...leave prev cmd as editted */
					case 'q':
						while(getchar() != '\n');
						putchar('\n');
						normode();
						return;
					/* print previous to end of line */
					case '\n':
					case 'p':
					case 'l':
						while(*cp1 != '\n') {
							putchar(*cp1);	/* print */
							*cp2++ = *cp1++;	/* and pass to current buf */
						}
recycle:
						putchar('\n');	/* end line */
						*cp2++ = '\n';
						cp1 = prev_line;	/* move new current line to prev line buf */
						cp2 = line;
						while(*cp2 != '\n') {
							*cp1++ = *cp2++;
						}
						*cp1++ = '\n';	/* end line */
						cp1 = prev_line;	/* reset ptrs */
						cp2 = line;
						break;
					}
				}
				while(*cp1 != '\n') {	/* ready to execute, finish up line */
					*cp2++ = *cp1++;
				}
				*cp2++ = '\n';
				*cp2++ = '\0';
				cp2 = line;	/* reset pointer */
				putchar('\n');	/* start on new line */
				while(*cp2 != '\n')	/* until almost end of line */
					putchar(*cp2++);	/* print */
				while((c = getchar()) != '\n') {	/* display final line for user */
					switch(c) {	/* newline=execute, q newline=quit */
					case 'q':
						getchar();
						putchar('\n');
						normode();
						return;
						break;
					}
				}
				putchar('\n');
				cp1 = prev_line;	/* reset ptrs */
				cp2 = line;
				while(*cp1++ = *cp2++);	/* move current line to prev buf */
				*cp1++ = '\0';
				prev_flag++;	/* set flag for alternate 'readc' */
				normode();
				main1();	/* execute */
				prev_flag = 0;
			}
			else {		/* can't do within shell file */
				prs("prev: cannot execute");
			}
			return;
		}
		if(equal(cp1,"logout") || equal(cp1,"logo") || equal(cp1,"bye")
		|| equal(cp1,"login") || equal(cp1,"logoff") || equal(cp1,"newgrp")) {
			if (promp != 0) {
				batch();	/* wait for batch jobs */
			}
			else {
				prs(t[DCOM]);
				prs(": cannot execute\n");
				return;
			}
		}
		if(equal(cp1,"newproj")) {
			if(promp != 0) {
				newproj();
			}
			else {
				prs("newproj: cannot execute\n");
			}
			return;
		}
                if(equal(cp1, "next")) {        /* next file */
                        if(!t[DCOM+1]) {
                                err("next: arg count");
                                return;
                        }
                        if(sinfil) {
                                err("next: nesting not allowed");
                                return;
                        }
                        if((f=open(t[DCOM+1],0))<0) {
                                err("next: no file");
                                return;
                        }
                        next(f);
                        return;
                }
		if(equal(cp1,"chg")) {
			chgflag = 1;
			return;
		}
		if(equal(cp1,"nochg")) {
			chgflag = 0;
			return;
		}
		if(equal(cp1,"cost")) {
			prtchg(1);
			return;
		}
                if(equal(cp1, "wait")) {        /* wait by pid */
                        if(t[DCOM+1]) {
                                i = getn(t[DCOM+1]);	/* if number supplied, use it */
                                if(i == 0)
                                        return;
                        }
                        else			/* otherwise use -1 */
                                i = -1;
                        if(setintr)
                                signal(INTR,unnext);
                        pwait(i, 0);		/* go wait for any termination */
                        if(setintr)
                                signal(INTR,1);
                        return;
                }
                if(equal(cp1, ":"))
                        return;

        case TPAR:
                f = t[DFLG];			/* look at cmd flags */
                i = 0;
                if((f&FPAR) == 0)
                        i = fork();
                if(i == -1) {		/* cannot execute cmd */
                        err("try again");
                        return;
                }
		/* main line code */
                if(i != 0) {
                        if((f&FPIN) != 0) {
                                close(pf1[0]);
                                close(pf1[1]);
                        }
                        if((f&FPRS) != 0) {
                                prn(i);		/* print out process id */
                                prs("\n");
                                xfree(vbls['P'-'A']);	/* save process id */
                                vbls['P'-'A'] = putn(i);
                        }
                        if((f&FAND) != 0)		/* if ampersanded off into background */
                                return;
						/* if this cmd not being piped to next cmd, */
                        if((f&FPOU) == 0)	/* if not being piped onward, */
                                pwait(i, t);	/* wait for specific process termination */
                        return;
                }
		/* subshell code */
		chgflag = 0;		/* suppress charges in subshell */
		/* redirected input */
                if(t[DLEF] != 0) {
                        close(0);
                        i = open(t[DLEF], 0);
                        if(i < 0) {
                                prs(t[DLEF]);
                                err(": cannot open");
                                exit();
                        }
                }
		/* redirected output */
                if(t[DRIT] != 0) {
                        if((f&FCAT) != 0) {	/* if output appended */
                                i = open(t[DRIT], 1);	/* appended */
                                if(i >= 0) {
                                        seek(i, 0, 2);
                                        goto f1;
                                }
                        }
                        i = creat(t[DRIT], 0644);	/* replaced */
                        if(i < 0) {
                                prs(t[DRIT]);
                                err(": cannot create");
                                exit();
                        }
                f1:
                        close(1);
                        dup(i);
                        close(i);
                }
		/* redirected error output */
		if(t[DERR] != 0) {
			if((f&FCAT) != 0) {		/* appended */
				i = open(t[DERR],1);
				if(i >= 0) {
					seek(i,0,2);
					goto f2;
				}
			}
			i = creat(t[DERR],0644);
			if(i < 0) {
				prs(t[DERR]);
				err(": cannot create");
				exit();
			}
		f2:
			close(2);
			dup(i);
			close(i);
		}
		/* piped input */
                if((f&FPIN) != 0) {
                        close(0);
                        dup(pf1[0]);
                        close(pf1[0]);
                        close(pf1[1]);
                }
		/* piped output */
                if((f&FPOU) != 0) {
                        close(1);
                        dup(pf2[1]);
                        close(pf2[0]);
                        close(pf2[1]);
                }
                if((f&FINT)!=0 && t[DLEF]==0 && (f&FPIN)==0) {
                        close(0);
                        open("/dev/null", 0);
                }
                if((f&FINT) == 0 && setintr) {
                        signal(INTR, 0);
                        signal(QUIT, 0);
                }
                if(t[DTYP] == TPAR) {
                        if(t1 = t[DSPR])
                                t1[DFLG] =| f&FINT;
                        execute(t1);
                        exit();
                }
		close(acctf);
                gflg = 0;
		/* check if glob needed, set gflg */
                scan(t, &tglob);
		/* if global needed, call glob */
		/* include default bin directory only */
		/* cmd exists in local bin */
                if(gflg) {
                        if(hashme(t[DCOM])) {
				t[DERR] = globnam;
				t[DFLG] = "-d";
				t[DSPR] = dfltfile;
				execv(t[DERR],t+DERR);
			}
			else {
				t[DSPR] = globnam;
				execv(t[DSPR],t+DSPR);
			}
                        prs("glob: cannot execute\n");
                        exit();
                }
		/* take off quote bits 0200 */
                scan(t, &trim);
                *linep = 0;
                cp2 = t[DCOM];                          /* khd added */
		/* try current directory or absolute pathname */
                texec(cp2, t);
		/* try private bin directory */
                if (*cp2 != '/') {                      /* khd added */
                        if (hashme(cp2)) {
                                cp1 = linep;
                                cp2 = dfltfile;
                                while (*cp1 = *cp2++) cp1++;
                                cp2 = t[DCOM];
                                while (*cp1++ = *cp2++) ;
                                texec(linep, t);
                        }
                        cp1 = linep;
                        cp2 = "/usr/bin/";
                        while (*cp1 = *cp2++) cp1++;
                        cp2 = t[DCOM];
                        while (*cp1++ = *cp2++);
                        texec(linep+4, t);	/* then try /bin */
                        texec(linep, t);	/* then try /usr/bin */
                }
                prs(t[DCOM]);
                err(": not found");
                exit();

        case TFIL:
                f = t[DFLG];
                pipe(pv);
                t1 = t[DLEF];
                t1[DFLG] =| FPOU | (f&(FPIN|FINT|FPRS));
                execute(t1, pf1, pv);
                t1 = t[DRIT];
                t1[DFLG] =| FPIN | (f&(FPOU|FINT|FAND|FPRS));
                execute(t1, pv, pf2);
                return;

        case TLST:
                f = t[DFLG]&FINT;
                if(t1 = t[DLEF])
                        t1[DFLG] =| f;
                execute(t1);
                if(t1 = t[DRIT])
                        t1[DFLG] =| f;
                execute(t1);
                return;
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
texec(f, at)
int *at;
{
        extern errno;
        register int *t;

        t = at;
	/* try as executable program */
        execv(f, t+DCOM);
	/* if not executable, try as shell file */
        if (errno==ENOEXEC) {
                if (*linep)
                        t[DCOM] = f;
                t[DSPR] = shnam;
                execv(t[DSPR], t+DSPR);
                prs("No shell!\n");
                exit();
        }
	/* if not enough memory, give up */
        if (errno==ENOMEM) {
                prs(t[DCOM]);
                err(": too large");
                exit();
        }
}

