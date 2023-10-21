#include "types.h"
#include "ext.h"

pdontln()	/* pay the dont pass line */
{
	double bet(),x;

	x=dont+bet(lodds[point],op[point][1],op[point][0]);
	total=total+x+lodds[point];
	wins=wins+x;
	if(dont!=0.0) {
		sprintf(line,"You won %.2f on the Dont Pass! ~",x);
		announce(line);
	}
	lodds[point]=0.0;
}

cdontln()	/* clear the dont pass line */
{
	loss=loss+dont+lodds[point];
	if(dont!=0.0) announce("You lost on Dont Pass! ~");
	dont=0.0;
	lodds[point]=0.0;
}

pdontb()	/* pay the dont bar */
{
	total=total+dcomeb;
	wins=wins+dcomeb;
	if(dcomeb!=0.0) announce("You won your Dont Come Bet! ~");
}

cdontb()	/* clear the dont bar */
{
	loss=loss+dcomeb;
	if(dcomeb!=0.0) announce("You lost your Dont Come Bar Bet! ~");
	dcomeb=0.0;
}

pdonts()	/* pay all of the dont come points */
{
	int i,j=0;
	double x=0.0,bet();

	for(i=0;i<11;i++) {
		x=x+dcome[i]+bet(lodds[i],op[i][1],op[i][0]);
		total=total+dcome[i]+lodds[i];
		if(dcome[i]!=0.0) j++;
		dcome[i]=lodds[i]=0.0;
	}
	sprintf(line,"You won %.2f on your Dont Come Bet%s~",x,(j==1)?". " :"s. ");
	if(j) announce(line);
	total=total+x;
	wins=wins+x;
}

csdont(n,off)	/* clear a single dont come bet */
int n,off;
{
	loss=loss+dcome[n]+lodds[n];
	if(dcome[n]!=0.0) announce("Down in Back! ~");
	dcome[n]=lodds[n]=0.0;
}

do_dont(n)
int n;
{
	dcome[n]=dcomeb;
	if(dcomeb!=0.0) {
		sprintf(line,"Dont Come moved behind the %d. ~",n);
		announce(line);
	}
	dcomeb=0.0;
}
