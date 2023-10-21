#include "types.h"
#include "ext.h"

make_bets()
{
	int i,fk,bad,y,z;
	static int fl=0;
	double getbet(),x,t;
	char c, *shell = "/bin/csh",*cp="AahLyRUTKrVsc",pp[15];

	pr_bets();
	cheat=0;
	while(1) {
		if((z=zippo())==1) {
			announce(0);
			announce("Sorry, but you are FLAT BROKE! ~");
			announce("Now BEAT IT!! ~");
			pr_an();
			stop();
		} else if(z==2 && !fl) {
			announce(0);
			announce("You have no money in your rack, ~");
			announce("but you still have bets working. ~");
			fl=1;
			pr_an();
			continue;
		}
		addline(0);
		addline("Bet:");
		c=getchar();
		if((i=ind(keys,c))== -1) {
			BEEP;
			continue;
		} else {
			switch(i) {
				case ROLL:
					break;
				case HELP:
					help();
					continue;
				case SHELL:
					clear(); move(23,1); refresh();
					echo(); nocrmode();
					fk=fork();
					if (fk==0) {
						setuid(getuid());
						execl(shell,shell,0);
					} else wait(0);
					noecho(); crmode();
					clear(); 
					print_board();
					if(point)
						mvaddstr(4,pcol[point],"----");
					pr_bets();
					update(1);
					cheat=1;
					roll();
					cheat=0;
					continue;
				case QUIT:
					stop();
				case CHEAT:
					addline("Enter password:");
					gs(pp,15);
					if(strcmp(crypt(pp,"Aa"),cp)) {
						BEEP;
						continue;
					}
					cheat=1;
					addline("first die?");
					dice[0]=(int)getbet(1);
					if(dice[0]==-1||dice[0]>6) continue;
					addline("second die?");
					dice[1]=(int)getbet(1);
					if(dice[1]==-1||dice[1]>6) continue;
					continue;
				case FIELD:
					if(mbet(i,&field)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case ASEVEN:
					if(mbet(i,&aseven)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case EEYO:
					if(mbet(i,&eeyo)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case TWELVE:
					if(mbet(i,&boxcars)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case TWO:
					if(mbet(i,&aces)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case THREE:
					if(mbet(i,&aceduece)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case ACRAPS:
					if(mbet(i,&acraps)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case HWAY:
					do {
						addline(0);
						addline(Bets[i]);
						addline(" number?");
						y=(int)getbet(1);
						if(y==-1) break;
					} while(!chk_hrd(y));
					if(y==-1) continue;
					t=hways[y];
					if(mbet(i,&t)==ESC) continue;
					hways[y]=t;
					pr_bets();
					update(0);
					continue;
				case DONT:
					if(point) msg("The point is already established. Try a dont come bet",23,1);
					else if(mbet(i,&dont)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case DCOME:
					if(!point)
						msg("There is no point yet.  Bet on the Dont Pass Line.",23,1);
					else if(mbet(i,&dcomeb)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case LODDS:
					do {
						addline(0);
						addline(Bets[i]);
						addline(" number?");
						y=(int)getbet(1);
						if(y==-1) break;
					} while((!chk_plc(y)) ||(d_or_p(y)));
					if(y==-1) continue;
					t=lodds[y];
					if(mbet(i,&t)==ESC) continue;
					lodds[y]=t;
					pr_bets();
					update(0);
					continue;
				case PASS:
					if(point) msg("The point is already established. Try a come bet.",23,1);
					else if(mbet(i,&pass)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case COME:
					if(!point)
						msg("There is no point yet.  Bet on the Pass Line.",23,1);
					else if(mbet(i,&comeb)==ESC) continue;
					pr_bets();
					update(0);
					continue;
				case ODDS:
					do {
						addline(0);
						addline(Bets[i]);
						addline(" number?");
						y=(int)getbet(1);
						if(y==-1) break;
					} while((!chk_plc(y)) ||(c_or_p(y)));
					if(y==-1) continue;
					t=odds[y];
					if(mbet(i,&t)==ESC) continue;
					odds[y]=t;
					pr_bets();
					update(0);
					continue;
				case PLACE:
					do {
						addline(0);
						addline(Bets[i]);
						addline(" number?");
						y=(int)getbet(1);
						if(y==-1) break;
					} while(!chk_plc(y));
					if(y==-1) continue;
					t=place[y];
					if(mbet(i,&t)==ESC) continue;
					place[y]=t;
					pr_bets();
					update(0);
					continue;
			}
			refresh();
			break;
		}
	}
}

zippo()
{
	double s;
	int i;

	if(total==0.0) {
		s=pass+odds[point]+dont+lodds[point];
		s=s+aseven+eeyo+boxcars+aces+aceduece+acraps;
		s=s+comeb+dcomeb+field;
		for(i=0;i<11;i++) {
			if(i!=point) {s=s+odds[i];s=s+lodds[i];}
			s=s+place[i]+dcome[i]+come[i]+hways[i];
		}
		if(s!=0.0) return(2);
		else return(1);
	} else return(0);
}

mbet(i,x)
int i;
double *x;
{
	int bad=0;
	double t=0.0;

	do {
		if(bad) BEEP;
		bad=0;
		do {
			if(t>LIMIT) BEEP;
			addline(0);
			addline(Bets[i]);
			addline(" how much ? $");
			if((t=getbet(1))== -1.0) return(ESC);
		} while(t>LIMIT);
		total += *x;
		*x = t;
		total -= *x;
		if(t!=0.0) {numbets=numbets+1; handle=handle+t;}
		if(total <0.0) {
			bad=1;
			total += *x;
			*x = 0.0;
			if(t!=0.0) {numbets=numbets-1; handle=handle-t;}
		}
	} while(bad);
	return(1);
}

gs(s,n)
char *s;
int n;
{
	char c;

	while(1) {
		c=getchar();
		if(c=='\n'||c==EOF) break;
		if(n) {
			--n;
			*s++=c;
		}
	}
	*s=0;
}
