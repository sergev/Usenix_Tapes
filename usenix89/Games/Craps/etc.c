#include "types.h"
#include "ext.h"

pplace(n)	/* pay a place bet */
int n;
{
	double bet(),x;

	x=bet(place[n],plcpays[n][0],plcpays[n][1]);
	total=total+x;
	wins=wins+x;
	if(place[n]!=0.0) {
		sprintf(line,"You hit your place bet for %.2f! ~",x);
		announce(line);
	}
}

move_place(n)
int n;
{
	if(place[n]!=0.0) {
		announce("Your place bet is off on the Comeout! ~");
		if(pass!=0.0) {
			announce("Your place bet is in your Tray. ~");
			total=total+place[n];
			place[n]=0.0;
		}
	}
}

cplace()	/* clear all place bets */
{
	int i,j=0;
	char line[81];

	for(i=0;i<11;i++) { 
		if(place[i]!=0.0) j++;
		loss=loss+place[i];
		place[i]=0.0;
	}
	if(j) {
		sprintf(line,"You lost your Place Bet%s~",(j>1)?"s. ":". ");
		announce(line);
	}
}

pfield()
{
	double x;

	x=fpays[sum]*field;
	if(x==0.0) {
		loss=loss+field;
		if(field) announce("You lost your Field Bet! ~");
		field=0.0;
	} else {
		wins=wins+x;
		field=field+x;
		if(field>LIMIT) {
			total=total+(field-LIMIT);
			field=LIMIT;
		}
		if(field) {
			sprintf(line,"You won%s in the field! ~",
				(sum==2||sum==12)?
				((sum==12)?" Triple":" Double"):" ");
			announce(line);
		}
	}
}

chk_hrd(x)
int x;
{
	return((x==4)||(x==6)||(x==8)||(x==10));
}

chk_plc(x)
int x;
{
	return((x==4)||(x==5)||(x==6)||(x==8)||(x==9)||(x==10));
}

d_or_p(x)
int x;
{
	if(!x) return(0);
	if(dont==0.0 && x==point) {
		msg("You must have a dont pass bet to lay odds.",23,1);
		return(1);
	} else if(dcome[x]==0.0 && x!=point) {
		sprintf(line,"You don't have a dont come bet on %d.",x);
		msg(line,23,1);
		return(1);
	} else return(0);
}

c_or_p(x)
int x;
{
	if(!x) return(0);
	if(pass==0.0 && x==point) {
		msg("You must have a pass line bet to take odds.",23,1);
		return(1);
	} else if(come[x]==0.0 && x!=point) {
		sprintf(line,"You don't have a come bet on %d",x);
		msg(line,23,1);
		return(1);
	} else return(0);
}
