#include "types.h"
#include "ext.h"

double bet(x,n,d)
double x;
int n,d;
{
	int dollars=(int)x,quarters;

	if(n==0 || d==0 || x==(double)0.0) return(0.0);
	quarters=(int)((x-(double)dollars)/.25)+((dollars%d)*4);
	dollars -= (dollars%d);
	return((double)(((dollars/d*n)+(dollars%d))) +
	       (double)(((quarters/d)*n*.25)+((double)(quarters%d)*.25)));
}

ppassln()	/* pay the pass line */
{
	double bet(),x;

	x=pass+bet(odds[point],op[point][0],op[point][1]);
	total=total+x+odds[point];
	wins=wins+x;
	if(pass!=0.0) {
		sprintf(line,"You won %.2f on the Pass Line! ~",x);
		announce(line);
	}
	odds[point]=0.0;
}

cpassln()	/* clear the pass line */
{
	loss=loss+pass+odds[point];
	if(pass!=0.0) announce("Line Away! ~");
	pass=0.0;
	odds[point]=0.0;
}

pcomeb(on)	/* pay the come bar */
{
	total=total+comeb;
	wins=wins+comeb;
	if(comeb!=0.0) announce("You won your Come Bar Bet! ~");
	if(on) {
		total=total+comeb;
		if(comeb!=0.0) announce("Your Come Bet is back. ~");
		comeb=0.0;
	}
}

ccomeb()	/* clear the come bar */
{
	loss=loss+comeb;
	if(comeb!=0.0) announce("You lost your Come Bar Bet! ~");
	comeb=0.0;
}

ccome(off)	/* clear the come points */
int off;
{
	int i,j=0;

	for(i=0;i<11;i++) {
		loss=loss+come[i];
		if(come[i]!=0.0) j++;
		if(off) total=total+odds[i];
		else loss=loss+odds[i];
		come[i]=odds[i]=0.0;
	}
	sprintf(line,"Your come %s lost! ~",(j==1) ? "bet":"bets all");
	if(j) announce(line);
	if(off && j) announce("But you got your odds back! ~");
}

do_come(n,on)
int n,on;
{
	double bet(), x;

	x=come[n];
	if(on) x = x + bet(odds[n],op[n][0],op[n][1]);
	if(come[n]!=0.0 && comeb!=0.0) {
		if(come[n]==comeb) {
			total=total+x;
			wins=wins+x;
			sprintf(line,"Off and On on the %d for %.2f. ~",n,x);
			announce(line);
		} else {
			total=total+x+come[n]+odds[n];
			wins=wins+x;
			sprintf(line,"You won %.2f on your Come Bet! ~",x);
			announce(line);
			announce("You got your come bet");
			if(odds[n]!=0.0) announce("/odds");
			announce(" back. ~");
			come[n]=odds[n]=0.0;
			come[n]=comeb;
			comeb=0.0;
			sprintf(line,"Your come bet got moved to the %d. ~",n);
			announce(line);
		}
	} else if(come[n]!=0.0 && comeb==0.0) {
		total=total+x+come[n]+odds[n];
		wins=wins+x;
		sprintf(line,"You won %.2f on your Come Bet! ~",x);
		announce(line);
		if(!on) announce("But your odds were off! ~");
		come[n]=odds[n]=0.0;
	} else if(comeb!=0.0) {
		come[n]=comeb;
		comeb=0.0;
		sprintf(line,"Your come bet got moved to the %d. ~",n);
		announce(line);
	}
}
