#include "types.h"
#include "ext.h"

char bark[600];

clrtocol(n)
int n;
{
	int y,x,x1;

	getyx(stdscr,y,x); x1=x;
	while(x<=n) mvaddch(y,x++,' ');
	move(y,x1);
}

ind(s,c)
char *s,c;
{
	int i;

	for(i=0;*s;i++,s++) if(*s==c) break;
	if(!(*s)) return(-1);
	else return(i);
}

msg(s,y,x)
char *s;
{
	char c;

	move(y,x); clrtocol(D-1); refresh();
	printw("%s-More-",s); refresh();
	while((c=getchar())!=' ') BEEP;
}

addline(s)
char *s;
{
	static int acol=1;
	if(!s) {
		move(23,1); clrtoeol(); refresh(); acol=1;
	} else {
		if(acol+strlen(s) > 70) acol=1;
		move(23,acol); clrtoeol(); printw("%s ",s); refresh();
		acol += strlen(s)+1;
	}
}

double getbet(half)
int half;
{
	char c;
	int nd=0,dot=0,dc=0;
	double i=0.0,frac=0.0;

	while(1) {
		c=getchar();
		if(c=='\n') {
			if(frac==0.20||frac==0.70) {BEEP; continue;}
			else break;
		}
		if((c<'0' || c>'9')&&(c!='.')&&(c!=(char)8)&&(c!=ESC)) BEEP;
		else if(c==ESC) return(-1.0);
		else if(c=='.') {
			if(dot==1) BEEP;
			else {
				dot=1;
				addch(c); refresh();
			}
		} else if(c==(char)8) {
			if(!nd && (!dc && !dot)) BEEP;
			if(dc) {
				if(frac==0.25) frac=0.20;
				else if(frac==0.75) frac=0.70;
				else if(frac==0.50 && dc==2) frac=0.50;
				else if(frac==0.50) frac=0.0;
				else if(frac==0.20) frac=0.0;
				else if(frac==0.70) frac=0.0;
			}
			if(nd) i = (float) ((int) (i/10.0));
			if(nd || dc || dot) {
				addch((char)8); addch(' '); addch((char)8);
				refresh();
				if(dc) dc--;
				else if(dot && !dc) dot=0;
				else if(nd) nd--;
			}
		} else {
			if(dot)
				switch(dc) {
					case 0: if(c=='2'||c=='5'||c=='7') {
							frac=(double)(c-'0')/10.0;
							dc++;
							addch(c); refresh();
						} else BEEP;
						break;
					case 1:	if(c=='5') {
							if(frac==.2||frac==.7) {
								frac=frac+0.05;
								dc++;
								addch(c);
								refresh();
							} else BEEP;
						} else if(c=='0') {
							if(frac==.5) {
								frac=0.5;
								dc++;
								addch(c);
								refresh();
							} else BEEP;
						} else BEEP;
						break;
					default:BEEP;
				}
			else {
				if(nd==3) {BEEP; continue;}
				i = (i*10.0) + c - '0';
				nd++;
				addch(c); refresh();
			}
		}
	}
	return(i+frac);
}

announce(s)
char *s;
{
	if(!s) bark[0]=0;
	else strcat(bark,s);
}

pr_an()
{
	int i=0,j=0,k=0,l,K=11;
	char temp[81];

	line[0]=0;
	while(bark[i]) {
		while(bark[i] != '~') temp[j++]=bark[i++];
		i++;
		temp[j]=0;
		l=K+((D-7-K-strlen(temp))/2);
		move(18,K); clrtocol(D-1); refresh();
		msg(temp,18,l);
		j=0;
	}
	move(18,K); clrtocol(D-1); refresh();
}

update(force)
int force;
{
	if(otot!=total || olos!=loss || owin!=wins || force) {
		move(20,6); clrtocol(D-1); refresh();
		printw("Rack:%.2f",total);
		printw("  Winnings:%.2f",wins);
		printw("  Losses:%.2f",loss);
		refresh();
	}
	otot=total; olos=loss; owin=wins;
}

roll()
{
	int i,k,l;

	announce(0);
	mvaddstr(10,2,"-------");
	mvaddstr(11,2,"|     |");
	mvaddstr(12,2,"|     |");
	mvaddstr(13,2,"|     |");
	mvaddstr(14,2,"-------");
	mvaddstr(15,2,"-------");
	mvaddstr(16,2,"|     |");
	mvaddstr(17,2,"|     |");
	mvaddstr(18,2,"|     |");
	mvaddstr(19,2,"-------");
	if(cheat) {
		spots(dice[0],11,3);
		spots(dice[1],16,3);
	} else for(i=0;i<10;i++) {
		dice[0]=get_rand();
		dice[1]=get_rand();
		spots(dice[0],11,3);
		spots(dice[1],16,3);
		refresh();
	}
	sum=dice[0]+dice[1];
}

spots(i,y,x)
int i,y,x;
{
	switch(i) {
		case 1:
			mvaddstr(y,x,"     ");
			mvaddstr(y+1,x,"  o  ");
			mvaddstr(y+2,x,"     ");
			break;
		case 2:
			mvaddstr(y,x,"o    ");
			mvaddstr(y+1,x,"     ");
			mvaddstr(y+2,x,"    o");
			break;
		case 3:
			mvaddstr(y,x,"o    ");
			mvaddstr(y+1,x,"  o  ");
			mvaddstr(y+2,x,"    o");
			break;
		case 4:
			mvaddstr(y,x,"o   o");
			mvaddstr(y+1,x,"     ");
			mvaddstr(y+2,x,"o   o");
			break;
		case 5:
			mvaddstr(y,x,"o   o");
			mvaddstr(y+1,x,"  o  ");
			mvaddstr(y+2,x,"o   o");
			break;
		case 6:
			mvaddstr(y,x,"o   o");
			mvaddstr(y+1,x,"o   o");
			mvaddstr(y+2,x,"o   o");
			break;
	}
}

mark_point(n)
int n;
{
	mvaddstr(4,pcol[n],"----");
	refresh();
	point=n;
	sprintf(line,"The Point is %d! ~",point);
	announce(line);
	move_place(n);
}

cl_point()
{
	mvaddstr(4,pcol[point],"    ");
	refresh();
	point=0;
}

bar_the_12()
{
	int i,j=0;

	for(i=0;i<11;i++) if(dcome[i]!=0.0) j=1;
	if(dcomeb!=0.0) j=1;
	if(j || (dont!=0.0 && !point)) announce("Bar the Dont's! ~");
}
