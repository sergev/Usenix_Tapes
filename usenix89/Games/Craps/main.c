/*
 *  I hearby put this program in the Public Domain.  It can be used
 *  in any way, shape or form.
 *
 *  I assume no responsibility for anything this program may procure
 *  (or not procure) on those who use it. :-)
 *
 *  Ray Tripamer,
 *  Unviersity of Nevada, Las Vegas (Yes, that says Las Vegas!)
 *
 */
#include "types.h"

int pcol[11] = {0,0,0,0,C+11,C+19,C+27,0,C+35,C+43,C+51};

char *keys="pcdDofblhsa23yuCq?!r",line[81];
char *nums[] = {
	"","","Two","Three","Four","Five","Six","Seven","Eight","Nine",
	"Ten","Eleven","Twelve"
};
char *Bets[] = {
	"pass line:", "come:", "don't pass:", "don't come:",
	"take odds:", "field:", "place:","lay odds:", "hard way:",
	"any seven:", "any craps:", "snake eyes:", "ace-duece:",
	"eleven:", "boxcars:"
};

double total, wins, loss, otot, olos, owin, handle;

double pass,comeb,come[11],odds[11];
double dont,dcomeb,dcome[11],lodds[11];
double place[11],field;
double hways[11],aseven,acraps,eeyo,boxcars,aces,aceduece;

int dice[2]={3,4},sum,point,cheat;
int plcpays[11][2] = {	{0,0},{0,0},{0,0},{0,0},
			{9,5},{7,5},{7,6},{0,0},
			{7,6},{7,5},{9,5} };
int op[11][2] ={	{0,0},{0,0},{0,0},{0,0},
			{2,1},{3,2},{6,5},{0,0},
			{6,5},{3,2},{2,1} };
int hp[11][2] = {	{0,0},{0,0},{0,0},{0,0},
			{7,1},{0,0},{9,1},{0,0},
			{9,1},{0,0},{7,1} };
int fpays[13] = {0,0,2,1,1,0,0,0,0,1,1,1,3};

long numbets=0;

main()
{
	int stop(), i;

	signal(SIGINT,stop);
	initscr();
	noecho();
	crmode();
	seedrand();
	total=100.0;
	otot=owin=olos=handle=0.0;
	loss=wins=pass=dont=dcomeb=comeb=0.0;
	aseven=acraps=aceduece=boxcars=aces=eeyo=0.0;
	for(i=0;i<11;i++) dcome[i]=come[i]=odds[i]=place[i]=hways[i]=0.0;
	print_board();
	update(1);
	cheat=1;
	roll();
	while(1) {
		make_bets();
		roll();
		pay_winners();
	}
}

stop()
{
	final();
	move(23,0); clrtoeol(); refresh();
	echo();
	nocrmode();
	endwin();
	exit(0);
}
