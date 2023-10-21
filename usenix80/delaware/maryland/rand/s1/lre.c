#define HELP	025
#define ARG	000
#define LEFT	001
#define SETFILE 002
#define C_PORT	003
#define OPEN	004
#define SCH_M	005
#define CLOSE	006
#define PUT	007
#define L_OVER	010
#define TAB	011
#define DOWN	012
#define HOME	013
#define PICK	014
#define UP	016
#define INSERT	017
#define GOTO_	020
#define PAGE_M	021
#define SCH_P	022
#define RIGHT	023
#define LINE_P	024
#define SUCK	025
#define NAMEFIL 026
#define LINE_M	027
#define PAGE_P	031
#define S_PORT	032
#define MDEF	033
#define QUOTE	034
#define BACKTAB 035
#define BSPACE	036
#define R_OVER	037
#define QUIT	0177


int ttystat[3], statsave, pip[2], parent;
int pflg 1;
int flg 0;
char str[50];

main (argc,argv)
int argc;
char *argv[];
{	register char c;

	/* Set to Raw mode, echo */
	gtty(0,ttystat);
	statsave = ttystat[2];
	ttystat[2] = 040 | (ttystat[2]&(~072) );
	stty(0,ttystat);

	pipe(pip);
	if(!pfork())
	{	close(0); dup(pip[0]);
		argv[argc]=0;
		execv("/bin/re",argv);
		putm("re Failed");
		kill(parent);
		exit();
	}
	close(1); dup(pip[1]);
	sleep(1);

	for(;;)
	{	if( (c=get1()) == HELP )
		{	if(flg){ put1(ARG); flg=0; }
			put1(ARG);
			putm("Press desired key for Help");
			switch(get1())
			{	case TAB:
					doit("tab"); break;
				case BACKTAB:
					doit("backtab"); break;
				case PAGE_M:
					doit("page-"); break;
				case PAGE_P:
					doit("page+"); break;
				case LINE_M:
					doit("line-"); break;
				case LINE_P:
					doit("line+"); break;
				case HOME:
					doit("home"); break;
				case PUT:
					doit("put"); break;
				case PICK:
					doit("pick"); break;
				case CLOSE:
					doit("close"); break;
				case OPEN:
					doit("open"); break;
				case ARG:
					doit("arg"); break;
				case QUIT:
					doit("del"); break;
				case MDEF:
					doit("esc"); break;
				case GOTO_:
					doit("goto"); break;
				case SCH_M:
					doit("sch-"); break;
				case SCH_P:
					doit("sch+"); break;
				case INSERT:
					doit("insert"); break;
				case HELP:
					doit("help"); break;
				case C_PORT:
					doit("switch"); break;
				case S_PORT:
					doit("port"); break;
				case SETFILE:
					doit("set"); break;
				case NAMEFIL:
					doit("make"); break;
				case RIGHT:
					doit("right"); break;
				case LEFT:
					doit("left"); break;
				case QUOTE:
					doit("quote"); break;
				case 'A': case 'a':
				case 'B': case 'b':
				case 'C': case 'c':
				case 'S': case 's':
				case 'V': case 'v':
				case '{': case '[':
				case '|': case '\\':
					doit("control"); break;
				case 'K': case 'k':
					doit("key"); break;
				case 'E': case 'e':
					doit("error"); break;
				case 'W': case 'w':
					doit("window"); break;
				case UP:
				case DOWN:
				case L_OVER:
				case R_OVER:
					doit("arrow"); break;
				default:
					put1(ARG);
					put1(ARG);
					putm("STANDARD KEYBOARD CHAR");
					flg=1;
		}	}
		else
		{	if(c==INSERT)
			{	put1(c);
				while( (c=get1())!=INSERT) put1(c);
			}
			put1(c);
			if(c==QUOTE) put1(get1());
}	}	}



doit(s) char *s;
{	char c;
	cat("/rnd/sdl/ld/",s);
	put1(ARG);
	put1(ARG);
	putm(str);
	put1(SETFILE);
	while((c=get1())==LINE_M||c==LINE_P||c==PAGE_M||c==PAGE_P)
		put1(c);
	put1(SETFILE);
}

cat(s,t) char *s, *t;
{	register char *u;
	u = str;
	while(*s) *u++ = *s++;
	while(*u++ = *t++);
}


putm(s) char *s;{
	while(*s) put1(*s++);}

put1(c) char c;
{	write(1,&c,1);
	if (c == QUIT)
	{	ttystat[2] = statsave;
		stty(0,ttystat);
		exit();
}	}

get1()
{	char c;
	read(0,&c,1);
	return(c);
}
