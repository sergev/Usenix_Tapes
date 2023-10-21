/*
 * main program
 * split off to enable optional profiling
 */

#include <stdio.h>
#include "ed.h"
#include "window.h"
#include "edit.h"
#include "process.h"
#include "shell.h"
#include "make.h"	/* for leftbreak */
#include "file.h"

main(argc,argv,envp)
	int argc; char *argv[],**envp;
	{
	register char *p1,*p2;
	int *monbuf;
	extern etext;
	extern int onintr(), quit(), onhup(), onalarm();
	int (*oldintr)();

	/* setup screen package */
	if(initscr()<0){
		printf("\007## This terminal type not supported. ##\n");
		exit(1);
		}

#ifndef vax
	nice(-20);
#endif
	setuid(getuid());

#ifndef debugging
	oldquit=signal(SIGQUIT,SIG_IGN);
#endif
	oldhup=signal(SIGHUP,SIG_IGN);
	oldintr=signal(SIGINT,SIG_IGN);
	oldalrm=signal(SIGALRM,SIG_IGN);
	if((int)signal(SIGTERM,SIG_IGN)==0)
		signal(SIGTERM,quit);

	/* get options from environment */
	if(p2=getenv("EDIT"))
		getopt(0,p2);
	else
		askhelp++;

	while(--argc > 0 && (*++argv)[0] == '-')
		getopt(0,argv[0]+1);

#ifndef eunice
#ifdef debugging
	if(proflag){	/* turn on profiling */
		monbuf=finmon();
		printf("lowpc=%o, hipc=%o, cntsiz=%d\n",
			monbuf[0],monbuf[1],monbuf[2]);
		flush();
		}
#endif
#endif

	if(argc == 1){
		p1 = *argv;
		p2 = filedata[1].savedfile;
		while (*p2++ = *p1++);
		filemode = (stat(filedata[1].savedfile,&statbuf)<0 ? FILEMODE : statbuf.st_mode);
		globp = "r\n1h";
		}
	if(crashpid)
		globp = "x";
	setup();
	erasescreen();
	init(1);
	wc = w[1];
	fc = 1;
	if(runfile)
		run((char*)runfile,getenv("HOME"));
	if(((int)oldintr&01)==0)
		signal(SIGINT,onintr);
	if(((int)oldhup&01)==0)
		signal(SIGHUP,onhup);
/*
	signal(SIGALRM,onalarm);
	alarm(30);
*/
#ifdef SIG_HOLD
	signal(SIGALRM,SIG_HOLD);
	alarm(30);
#else
	signal(SIGALRM,SIG_IGN);
#endif
/*	signal(SIGSHELL,onfpe);	i don't know what this was doing here */
	setjmp(savej);
	jumpset=1;
	showstat(wc);
/*	home();	this shouldnt be needed */

	dispatch();
	quit();
	}
