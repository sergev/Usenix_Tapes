#
#define MOV       101
#define DRW       102
#define END       103
#define READ      104
#define MOVE      105
#define DRAW      106
#define WINDOW    107
#define VIEWPORT  108
#define FRAME     109
#define UNFRAME   110
#define AXES      111
#define LPOFF     112
#define LPON      113
#define RECPIX    114
#define HARDCOPY  115
#define RESET     116
#define SETUP     117
#define DFSTAT    118
#define OPEN      119
#define POST      120
#define UNPOST    121
#define KILL      122
#define ERASE     123
#define ERASEVP   124
#define SKEL      125
#define SOLID     126
#define THREED    127
#define PLOT3     128
#define STATUS    129
#define POSITION  130
#define DIRECTION 131
#define SIZE      132
#define DISTANCE  133
#define PROMPT    134

/* The int and double variables below are global to much of doodle and
	are in the include file globals.incl. */
int kumquat [16],       /* This is the Grinnell display buffer. */
    xs0, ys0,           /* screen (grinnell) coordinates */
    GRAFMOD 0;          /* 1 if displaying vectors */

double vp[] {0., 0., 4., 3.75}, /* The viewport's ur and ll corners. */
       dw[] {0., 0., 4., 3.75}, /* The size of the display window. */

       Dis  10.,                /* The distance from the screen to the observer. */
       Dir[] {-6., -8., -7.5},   /* The direction the observer is looking in. */
       Pos[] { 6.,  8.,  7.5},   /* The position of the observer. */
       Siz[] {5., 5.};           /* The size of the screen. */

char prompt[16] "=>>";

main()
{
	char s[80], t[80];		/* for tokens from scanner */
	double atof(), x, y;
	int is, descr; /* type of operation, other inout file */
	int i, j, k; 	/* for some temporary veriables */
	printf("Gyuri's DOODLER\n");
	descr = 0;	/* first read tty */
	init();
	while(1) { /* exit from loop is in case END. */
		printf("%s ", prompt);
		token(s, &is, &descr);
		what(s, &is);
		switch (is) {
			case MOV:
				token(s, &is, &descr);
				token(t, &is, &descr);
				plt(atof(s), atof(t), 0);
				break;
			case DRW:
				token(s, &is, &descr);
				token(t, &is, &descr);
				plt(atof(s), atof(t), 1);
				break;
			case END:
				printf("End doodle\n");
				exit();
				break;
			case READ:
				token(s, &is, &descr);
				descr = open(s, 0);
				if (descr < 0){
					printf("open error\n");
					descr = 0;
				}
				break;
			case MOVE:
				token(s, &is, &descr);
				token(t, &is, &descr);
				plot(atof(s), atof(t), 0);
				break;
			case DRAW:
				token(s, &is, &descr);
				token(t, &is, &descr);
				plot(atof(s), atof(t), 1);
				break;
			case WINDOW:
				token(s, &is, &descr);
				token(t, &is, &descr);
				x = atof(s);
				y = atof(t);
				token(s, &is, &descr);
				token(t, &is, &descr);
				window(x, y, atof(s), atof(t));
				break;
			case VIEWPORT:
				token(s, &is, &descr);
				token(t, &is, &descr);
				x = atof(s);
				y = atof(t);
				token(s, &is, &descr);
				token(t, &is, &descr);
				viewport(x, y, atof(s), atof(t));
				break;
			case FRAME:
				frame();
				break;
		     /* case UNFRAME:
				unframe();
				break;  */
			case ERASE:
				erase();
				break;
			case ERASEVP:
				erasevp();
				break;
			case SKEL:
				token(s, &is, &descr);
				printf("skel file: %s\n", s);
				skel(s);
				break;
			case SOLID:
				token(s, &is, &descr);
				printf("solid\ninput file: %s\n", s);
				solid(s);
				break;
			case THREED:
				token(s, &is, &descr);
				token(t, &is, &descr);
				printf("input picture: %s\noutput picture: %s\n", s, t);
				threed(s, t);
				break;
			case PLOT3:
				plot3();
				break;
			case STATUS:
				printf("position:  (%f, %f, %f)\n",
					Pos[0],Pos[1],Pos[2]);
				printf("direction: (%f, %f, %f)\n",
					Dir[0],Dir[1],Dir[2]);
				printf("screen:     %f by %f at %f\n",
					Siz[0],Siz[1],Dis);
				printf("\nviewport:%f, %f, %f, %f\n",
					vp[0],vp[1],vp[2],vp[3]);
				printf("\nwindow  :%f, %f, %f, %f\n",
					dw[0],dw[1],dw[2],dw[3]);
				break;
			case POSITION:
				token(s, &is, &descr);
				token(t, &is, &descr);
				Pos[0] = atof(s);
				Pos[1] = atof(t);
				token(s, &is, &descr);
				Pos[2] = atof(s);
				break;
			case DIRECTION:
				token(s, &is, &descr);
				token(t, &is, &descr);
				Dir[0] = atof(s);
				Dir[1] = atof(t);
				token(s, &is, &descr);
				Dir[2] = atof(s);
				break;
			case SIZE:
				token(s, &is, &descr);
				token(t, &is, &descr);
				Siz[0] = atof(s);
				Siz[1] = atof(t);
				break;
			case DISTANCE:
				token(s, &is, &descr);
				Dis = atof(s);
				break;
			case PROMPT:
				token(prompt, &is, &descr);
				break;
			case -1:
				printf("bad: %s - try again\n",s);
				break;
			default:
				printf("feature not yet implemented\n");
				break;
		}
	}
}

