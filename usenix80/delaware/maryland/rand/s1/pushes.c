#
/* Interpret button pushes to Rand Editor */
/* Jim Gillogly, Dec. 1974 */
#include "/lisp/ned.new/ned.defs"	/* for codes of button pushes */
main(argn,args) 	/* interpret button pushes to the Rand Editor (re) */
int argn, **args;	/* file is given as argument to command */
{	int fildes,i;
	char filename[40];
	char ch;
	if (argn<2)			/* file-name not supplied to program*/
	{	printf("file? ");
		for (i=0; (filename[i]=getchar()) != '\n'; i++);/* rd to nl */
		filename[i]=0;			/* end of string */
		if ((fildes=open(filename,0)) == -1) bomb_out(0);
	}
	else	/* filename was supplied */
	if ((fildes=open(args[1],0)) == -1) bomb_out(0);
	while (read(fildes,&ch,1))	 /* rd a char, stop when EOF */
	{	switch(ch)
		{	case CCENTER: printf("<ARG>"); break;
			case CCLPORT: printf("<PORT LEFT>\n"); break;
			case CCSETFILE: printf("<SET FILE>\n"); break;
			case CCCHPORT: printf("<CHANGE PORT>\n"); break;
			case CCOPEN: printf("<OPEN LINE>\n"); break;
			case CCMISRCH: printf("<-SRCH>\n"); break;
			case CCCLOSE: printf("<DELETE LINE>\n"); break;
			case CCPUT: printf("<PUT>\N"); break;
			case CCMOVELEFT: printf("<BACKSPACE>\n"); break;
			case CCTAB: printf("<TAB>\n"); break;
			case CCMOVEDOWN: printf("<RETURN>\n"); break;
			case CCHOME: printf("<HOME>\n"); break;
			case CCPICK: printf("<PICK>\n"); break;
			case CCRETURN: printf("<DOWN>\n"); break;
			case CCMOVEUP: printf("<UP>\n"); break;
			case CCINSMODE: printf("<INSERT MODE>\n"); break;
			case CCGOTO: printf("<GO TO>\n"); break;
			case CCMIPAGE: printf("<-PAGE>\n"); break;
			case CCPLSRCH: printf("<+SRCH>\n"); break;
			case CCRPORT: printf("<PORT RIGHT>\n"); break;
			case CCPLLINE: printf("<+LINE>\n"); break;
			case CCDELCH: printf("<SUCK>\n"); break;
		   /*   case CCMAKEFILE: printf("<MAKE FILE>\n"); break; */
      /* 6-28-76 jal */ case CCSAVEFILE: printf("<MAKE FILE>\n"); break;
			case CCMILINE: printf("<-LINE>\n"); break;
      /* 6-28-76 jal    case CCUNMAKEPORT: printf("<UNMAKE PORT>\n"); break;*/
/* 19-Apr-79 RL Kirby */case CCDOCMD: printf("<XQT>\n"); break;
			case CCPLPAGE: printf("<+PAGE>\n"); break;
			case CCMAKEPORT: printf("<MAKE PORT>\n"); break;
			case CCCTRLQUOTE: printf("<QUOTE>\n"); break;
			case CCBACKTAB: printf("<TAB LEFT>\n"); break;
			case CCBACKSPACE: printf("<BACKSPACE>\n"); break;
			case CCMOVERIGHT: printf("<MV RIGHT>\n"); break;
			case CCQUIT: printf("<EXIT>\n"); break;
			default: printf("%c",ch);	/* print chars */
		}
	}
}


bomb_out(err)	/* error msgs */
int err;
{	switch(err)
	{	case 0: printf("file not found\n");
			exit();
	}
}
