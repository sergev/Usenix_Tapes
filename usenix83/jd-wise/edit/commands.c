/*
 * ed mode commands
 */

#include "ed.h"
#include "terminal.h"
#include "window.h"
#include "edit.h"
#include "enscreen.h"
#include "file.h"
#include "make.h"	/* for leftbreak */
#include "process.h"
#include "shell.h"	/* for shellactive */
#include "spell.h"	/* for spellpid */

char	prompt[]={"A: "};
int	mode=cmdmode;
int	wrapp=0;
extern errno;

#define textcol 7	/* temp */


#undef next
#define next goto nextcomd

commands()
	{
	int gettty();
	register *a1,c;
	int l,n;
	int headline;	/* first line of limited window */
	struct address addr;
	register struct window *wp;
	char line[128], *cp;

#ifdef debugging
debug(44,"fc=%d wc=0%o",fc,wc);
#endif
	wp=wc;	/* in case one of the initial tests bypasses
		 * calling address */
loop:
	addr1.ad_addr=0;
	addr1.ad_window=wc;
	addr2.ad_addr=0;
	addr2.ad_window=wc;
	if(hflag){
		hflag=0;
		pflag=0;
		if(wc->wi_dol == wc->wi_zero)
			errfunc("Buffer empty");
		addr1.ad_addr=addr2.ad_addr=wp->wi_zero+1;
		goto head;
		}
	if(pflag){
		pflag=0;
		addr1.ad_addr=addr2.ad_addr=wc->wi_dot;
		goto print;
		}
	/** this may be in the wrong place */
	if(globp==0)
		showstat(wc);
	do{
		addr1=addr2;
		addr=address();
		if(addr.ad_addr==0){
			c=getchr();
			break;
			}
		addr2=addr;
		if((c=getchr())==';'){
			c=',';
			addr.ad_window->wi_dot=addr.ad_addr;
			}
		} while(c==',');
	if(addr1.ad_addr==0)
		addr1=addr2;
	wp=addr2.ad_window;	/* window for this operation */

	if(invflag){	/** this could be the wrong window */
		invflag=0;
		if(c != 'P') oneline(wp,wp->wi_dot);
		}
	if(redrawflag){
		redrawflag=0;
		redraw();
		}

/*	if(c>='A' && c<='Z') c |= '`';	/* force lower case */
#ifdef debugging
debug(18,"addr1=0%o addr2=0%o",addr1.ad_addr,addr2.ad_addr);
#endif
	switch(c){

	case 'a':
		setdot(wp);
		newline();
		appendonscreen(wp,addr2.ad_addr);
		return(co_inserting);
/*		next;*/

	case 'b':
		setdot(wp);
		nonzero();
		find(wp,0);
		split(curline,curcol);
		syncscreen(wp);
		next;

	case 'c':
		deletes(wp);
		appendonscreen(wp,addr1.ad_addr-1);
		return(co_inserting);
/*		next;*/

	case 'C':
		newline();
		setdot(wp);
		nonzero();
		center();
		next;

	case 'd':
		deletes(wp);
		next;

	case 'e':
		/** this code is really sloppy */
		setnoaddr();
		if((n=gow())>=0){
			if(n>helpwindow || (!globp && n>=helpwindow))
				errfunc("Bad window %c",c);
			/** this should be:
			 ** if(window[n].wi_flags&wi_active)
			 ** and the w array gotten rid of
			 **/
			if(w[n]==0){
				if(nwindows>=3)
					errfunc("Too many windows");
				init(n);
				/* init set modflag to -1
				 * this would cause the test below to fail*/
				filedata[n].modflag=0;
				}
			fc = n;
			addr1.ad_window=addr2.ad_window=wp=wc=w[n];
			if((peekc = getchr()) == '\n' || peekc==EOF){ /* just changing windows */
				newline();
				next;
				}
			}
#ifdef debugging
debug(61,"peekc=0%o",peekc);
#endif
		if((peekc = getchr()) != ' ')
			errfunc("Bad command format");
		if(filedata[fc].modflag){	/** this should be for the window being reused */
			while(getchr() != '\n');	/* eat rest of line */
			errmsg("Nothing saved. Confirm:");
			filedata[fc].modflag=0;
			next;
			}
		filedata[fc].savedfile[0]=0;
		init(fc);	/** this must be fixed for multiple files */
		addr2.ad_addr=0;
		newflag++;
		hflag++;
		filedata[fc].modflag = -1;		/* initial read doesn't count */
		goto caseread;

	case 'f':
		setdot(wp);
		nonzero();
		find(wp,1);
		next;

	case 'g':
		global(wp,1);
		next;

	case 'h':
		if((c=getchr())=='e')
			return(co_help);
		else
			peekc=c;
		newline();
		setdot(wp);
		nonzero();
		headline=wp->wi_first;
		goto head1;
	case 'H':
		newline();
		setdot(wp);
	head:		/* was before setdot */
#ifdef debugging
debug(60,"head: a1=0%o a2=0%o zero=0%o dol=0%o",
addr1.ad_addr,addr2.ad_addr,wp->wi_zero, wp->wi_dol);
#endif
		nonzero();
		headline=max(wp->wi_first,wp->wi_defline-expsize);
	head1:
		if(newflag){
			a1=addr1.ad_addr;
			clearwindow(wp);
			if(wp->wi_ulast== -1)
				wp->wi_ulast=headline-1;
			n=0;
			while(installine(wp,wp->wi_ulast+1,a1++) >= 0 && a1 <= wp->wi_dol)
				if((++n > 2*expsize)
				&& (wp != &window[helpwindow]))
					break;
			}
		else{
			shift(wp,headline,addr1.ad_addr);
			}
		for(l=wp->wi_ufirst; l<= wp->wi_defline && l <= wp->wi_ulast; l++)
			if((screen[l].sl_flags & (sl_written | sl_conting)) == sl_written)
				wp->wi_dot=screen[l].sl_addr;
		if(newflag)
			newflag=0;
		else
			updatedata(wp);
		next;

	case 'i':
	caseins:
		setdot(wp);
		nonzero();
		newline();
		appendonscreen(wp,addr2.ad_addr-1);
		return(co_inserting);
/*		next; */

	case 'j':
	case 'J':
		if(addr2.ad_addr == 0){
			addr1.ad_addr=wp->wi_dot;
			addr2.ad_addr=addr1.ad_addr+1;
			}
		setdot(wp);
		nonzero();
		newline();
		if(c=='j')
			join();
		else
			justify();
		next;

	case 'k':
		if((c=getchr()) < 'a' || c > 'z')
			errfunc("Bad mark name");
		newline();
		setdot(wp);
		nonzero();
		if(namet[c-'a'] == 26)
			namet[c-'a']=highmark++;
		marks[namet[c-'a']].mk_window = addr2.ad_window;
		marks[namet[c-'a']].mk_addr   = *addr2.ad_addr | 01;
		for(l=0; l<nlines; l++){
			if(screen[l].sl_mark == c+'A'-'a'){
				screen[l].sl_mark=' ';
				showdata(l);
				}
			}
		if((l=getslno(addr1.ad_addr))>=0){
			screen[l].sl_mark=c+'A'-'a';
			showdata(l);
			}
/*
		if(addr1 != addr2){
			if(namet[c-'`'] == 26)
				namet[c-'`']=highmark++;
			marks[namet[c-'`']]=addr2.ad_addr;
			if((l=getslno(addr2)) >= 0){
				screen[l].sl_mark=c+'A'-'`';
				showdata(l);
				}
			}
*/
		next;

	case 'K':	/* send signal to shell */
		setnoaddr();
		for(n=0; c=getchr()!='\n';){
			if(c==' ' || c=='\t') continue;
			n = n*10 + c-'0';
			}
		sigshell(n);
		next;

	case 'l':
		newline();
		listf++;
		invflag++;
		goto print;

	case 'M':
		setnoaddr();
		newline();
		mapcomd();
		next;

	case 'm':
		if((c=getchr())=='o')
			return(co_more);
		else
			peekc=c;
		move(wp,0);
		txflag++;
		pflag++;
		next;

	case 'n':
		if(getchr()!='o') goto badcomd;
		c=getchr();
		while(getchr()!='\n')	/* eat rest of line */
			;
		peekc='\n';	/* for newline in quit */
		switch(c){
		case 'h':	/* nohelp */
			n=helpwindow;
			break;
		case 's':	/* noshell */
			n=shellwindow;
			break;
		default:
			goto badcomd;
			}
		goto caseno;

	case 'o':
		setnoaddr();
		if((c=getchr()) == '\n'){
			message("Enter option:");
			c=getchr();
			}
		getopt(c,(char*)0);
		next;

	case 'p':
		setdot(wp);
		n=gol();
		newline();
		put(n>=0?n:0);
		txflag++;
		pflag++;
		next;
	case 'P':
		newline();
	print:
		setdot(wp);
		nonzero();
		a1=addr1.ad_addr;
		wp->wi_dot=addr2.ad_addr;
		oneline(wp,a1);
		listf=0;
		if(txflag || xflag) expand(wp);
		next;

	case 'q':
	casequit:
		setnoaddr();
		if((n=gow())>=0){
#ifdef debugging
debug(79,"casequit: c=%c n=%d globp=%d",c,n,globp);
#endif
			if(n>helpwindow || (!globp && n>=helpwindow))
				errfunc("Bad window %c",c);
	caseno:
			newline();	/* this has to be after the test of globp */
			if(w[n]==0)
				errfunc(n==helpwindow?"No help window":"Window %c not active",c);
			if(nwindows==1)
				errfunc("Can't delete last window");
			if(n>0 && filedata[n].modflag){
				errmsg("Nothing saved. Confirm quit: ");
				filedata[n].modflag=0;
				next;
				}
			if(n==0) killshell();
			deletewindow(w[n]);
			w[n]=0;
			if(fc==n){
				for(n=0,l=1; l<4; l++)	/* find first non shell window */
					if(w[l]){
						n=l;
						break;
						}
				wc=w[n];
				fc=wc->wi_index;
				}
/*			newline(); */
			next;
			}
		newline();
		/** this should test all active files */
		/** it does now */
		if(nwindows==1){
			if(filedata[fc].modflag){
				errmsg("Nothing saved. Confirm quit:");
				filedata[fc].modflag=0;
				next;
				}
			}
		else{
			if(shellpid&&shellactive){
				errmsg("Shell process active. Confirm quit:");
				shellactive=0;
				next;
				}
			for(n=1; n<4; n++)
				if(w[n] && filedata[n].modflag){
					errmsg("Window %c not saved. Confirm quit:",wname(n));
					filedata[n].modflag=0;
					next;
					}
			}
		quit();

	case 'r':
	caseread:
#ifdef debugging
debug(59,"caseread: fc=%d",fc);
#endif
		filename(fc);
errno=0;
		if((io=open(file,0)) < 0){
			lastc='\n';
			filedata[fc].modflag++;	/* edit invoked with bad file name */
			hflag=0;
			clearwindow(wp);
#ifdef debugging
#ifdef eunice
debug(93,"errno=%d uid=%d",errno,getuid());
#else
debug(93,"errno=%d uid=%d euid=%d",errno,getuid(),geteuid());
#endif
#endif
			errfunc("Can't open input file: %s",file);
			}
		setdot(wp);	/* read now inserts after . */
		ninbuf=0;
		append(wp,getfile,addr2.ad_addr);
		exfile();
		if(!globp) pflag++;	/* inhibit initial print */
		next;

	case 'S':	/* old-fashioned shell */
		for(cp=line; (*cp=getchr())!='\n' && *cp!=EOF; cp++)
			;
		*cp='\0';
		callunix(line);
		next;

	case 's':
		setdot(wp);
		nonzero();
		/*clearat(wp); now done in setat */
		substitute(wp,(int)globp);
		setat(wp);
		/** let's try it without this for a while
		expand(wp);
		**/
		next;


	case 't':
		move(wp,1);
		txflag++;
		pflag++;
		next;

	case 'u':
		setdot(wp);
		nonzero();
		newline();
	/*	clearat(wp); */
		addr1=addr2;	/* to make sure */
		getonscreen(wp,addr2.ad_addr);
	/*	delete(wp); */
		updflag++;
		appendonscreen(wp,addr2.ad_addr);
		return(co_inserting);
/*		next;*/

	case 'v':
		global(wp,0);
		next;

	case 'W':
		wrapp++;
	case 'w':
		setall(wp);
		nonzero();
		filename(fc);
		if(!wrapp
		|| ((io = open(file,1)) == -1)
		|| ((lseek(io, 0L, 2)) == -1))
			if ((io = creat(file, 0666)) < 0)
				errfunc("Can't create output file");
		wrapp = 0;
		putfile(wp);
		exfile();
		if (addr1.ad_addr==wp->wi_zero+1 && addr2.ad_addr==wp->wi_dol)
			filedata[fc].modflag = 0;
		next;

	case 'X':
		setnoaddr();
		getname();
		run(file,(char*)0);	/** this will cause a recursive call
				 ** of commands. could use co_run */
		next;

	case 'x':
		setdot(wp);
		nonzero();
		newline();
		wp->wi_dot=addr2.ad_addr;
		shift(wp,wp->wi_defline,wp->wi_dot);
		expand(wp);
		next;

	case 'y':
		setdot(wp);
		nonzero();
		n=gol();
		newline();
		yank(n>=0?n:0);
		next;

	case '\n':
		if(addr2.ad_addr == 0){	/* null line, advance one page */
			addr2.ad_addr = addr1.ad_addr = wp->wi_dot;
			goto head;
			}
		if(screen[cmdline].sl_text[0] == '-' && screen[cmdline].sl_text[1] == '\n'){
			/* single minus sign, retreat one page */
			goto retreat;
			}
		addr1=addr2;
		goto print;

	case '^':	/* retreat one page */
		newline();
	retreat:
		setused(wp);
		if(screen[wp->wi_defline].sl_flags != sl_empty)
			headline=max(wp->wi_ufirst,wp->wi_defline-expsize);
		else
			headline=wp->wi_ufirst;
		addr2.ad_addr=screen[headline].sl_addr;
		/*clearat(wp); now done in setat */
		wp->wi_dot=addr1.ad_addr=addr2.ad_addr;
		shift(wp,wp->wi_defline,addr2.ad_addr);
		expand(wp);	/* take care of single line */
		setat(wp);
		next;

	case '=':
		newline();
		statflag++;
		next;

	case '>':
		setnoaddr();
		n=gol();
		newline();
		loadbuf(n>=0?n:0);
		next;

	case '@':
		setdot(wp);
		n=gol();
#ifdef debugging
debug(86,"case @: n=%d",n);
#endif
		macro(n>=0?n:0);
		next;

#ifndef noshell
	case '!':
#ifdef old
		if(addr2.ad_addr==0){	/* !command */
			sendtoshell(&screen[cmdline].sl_text[1]);	/** temp */
			cmlptr = -1;	/* to kill the line */
			next;
			}
		else{			/* a1,a2! */
			nonzero();
			newline();
			linestoshell(wp);
			next;
			}
#else
		for(cp=line; (*cp=getchr())!='\n' && *cp!=EOF; cp++)
			;
		*cp='\0';
		if(line[0]!='\0')	/* ... !command */
			sendtoshell(line);
		if(line[0]=='\0' && addr2.ad_addr==0)	/* null command */
			sendtoshell(line);
		if(addr2.ad_addr!=0){	/* a1,a2! ... */
			nonzero();
			linestoshell(wp);
			}
		next;
#endif
#endif

	case esc:	/*temp*/
		return(co_esc);

	case EOL:	/* return to dispatch */
		return(EOL);

	case EOF:	/* returned on exhaustion of globp */
		return(EOF);
		}
badcomd:
#ifdef debugging
debug(7,"command char was %o",c);
#endif
	errfunc("Bad command");
	next;
nextcomd:
	if(globp)
		goto loop;
	else
		return(co_normal);
	}
