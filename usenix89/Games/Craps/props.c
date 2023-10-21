#include "types.h"
#include "ext.h"

props(n)		/* do the proposition bets */
int n;
{
	int i,j=0,affect;
	double x,bet();

	if(aseven!=0.0)
		if(n==7) {
			x=bet(aseven,4,1);
			total=total+x;
			wins=wins+x;
			sprintf(line,"You won $%.2f on your Any Seven! ~",x);
			announce(line);
		} else {
			loss=loss+aseven;
			announce("Any Seven Bet Lost! ~");
			aseven=0.0;
		}
	if(acraps!=0.0)
		if(n==2||n==3||n==12) {
			x=bet(acraps,8,1);
			total=total+x;
			wins=wins+x;
			sprintf(line,"You won $%.2f on Any Craps! ~",x);
			announce(line);
		} else {
			loss=loss+acraps;
			announce("Any Craps Bet Lost! ~");
			acraps=0.0;
		}
	if(eeyo!=0.0)
		if(n==11) {
			x=bet(eeyo,14,1);
			total=total+x;
			wins=wins+x;
			sprintf(line,"You won $%.2f on Eleven! ~",x);
			announce(line);
		} else {
			loss=loss+eeyo;
			announce("Eleven Bet Lost! ~");
			eeyo=0.0;
		}
	if(boxcars!=0.0)
		if(n==12) {
			x=bet(boxcars,29,1);
			total=total+x;
			wins=wins+x;
			sprintf(line,"You won $%.2f on Boxcars! ~",x);
			announce(line);
		} else {
			loss=loss+boxcars;
			announce("Boxcars Bet Lost! ~");
			boxcars=0.0;
		}
	if(aces!=0.0)
		if(n==2) {
			x=bet(aces,29,1);
			total=total+x;
			wins=wins+x;
			sprintf(line,"You won $%.2f on Snake Eyes! ~",x);
			announce(line);
		} else {
			loss=loss+aces;
			announce("Snake Eyes Bet Lost! ~");
			aces=0.0;
		}
	if(aceduece!=0.0)
		if(n==3) {
			x=bet(aceduece,14,1);
			total=total+x;
			wins=wins+x;
			sprintf(line,"You won $%.2f on Threee Bet! ~",x);
			announce(line);
		} else {
			loss=loss+aceduece;
			announce("Three Bet Lost! ~");
			aceduece=0.0;
		}
	for(affect=0,i=0;i<11;i++) {
		if(hways[i]!=0.0) j++;
		if(hways[i]!=0.0 && n==7)
			if(point) {loss=loss+hways[i]; hways[i]=0.0;}
			else affect=1;
		if(hways[i]!=0.0 && n==i && dice[0]==dice[1])
			if(point) {
				if(i==4||i==10) x=bet(hways[i],7,1);
				else x=bet(hways[i],9,1);
				total=total+x;
				wins=wins+x;
				sprintf(line,"You won $%.2f on the Hard %s.~",x,nums[i]);
				announce(line);
			} else affect=1;
		if(hways[i]!=0.0 && n==i && dice[0]!=dice[1])
			if(point) {
				loss=loss+hways[i];
				hways[i]=0.0;
				sprintf(line,"Hard %s Down! ~",nums[i]);
				announce(line);
			} else affect=1;
	}
	sprintf(line,"Hardway Bet%sDown! ~",(j==1)?" ":"s ");
	if(n==7 && j && point) announce(line);
	if(j && !point && affect) announce("Hardways off on the Comeout! ~");
}
