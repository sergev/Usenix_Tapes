#include	<stdio.h>
int m[4][76]={
	{ 1, 5, 9,13,17,21,25,29,33,37,41,45,49,53,57,61,
	  1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,
	  1,17,33,49, 2,18,34,50, 3,19,35,51, 4,20,36,52,
	  1,17,33,49,13,29,45,61, 1, 2, 3, 4,49,50,51,52,
	  1, 5, 9,13,49,53,57,61, 1,16, 4,13},
	{ 2, 6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,
	 17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
	  5,21,37,53, 6,22,38,54, 7,23,39,55, 8,24,40,56,
	  6,22,38,54,10,26,42,58,21,22,23,24,37,38,39,40,
	 18,22,26,30,34,38,42,46,22,27,23,26},
	{ 3, 7,11,15,19,23,27,31,35,39,43,47,51,55,59,63,
	 33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
	  9,25,41,57,10,26,42,58,11,27,43,59,12,28,44,60,
	 11,27,43,59, 7,23,39,55,41,42,43,44,25,26,27,28,
	 35,39,43,47,19,23,27,31,43,38,42,39},
	{ 4, 8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,
	 49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
	 13,29,45,61,14,30,46,62,15,31,47,63,16,32,48,64,
	 16,32,48,64, 4,20,36,52,61,62,63,64,13,14,15,16,
	 52,56,60,64, 4, 8,12,16,64,49,61,52}
};
int x[65],y[16]={1,49,52,4,13,61,64,16,22,39,23,38,26,42,27,43};
int p[65],c[65];
#include	"getline"
main()
{
	int n,i,j;
	char ans[80];
	for (;;) {
		board();
		for (i=1;i<65;i++)
			x[i]=0;
		for (i=1;i<33;i++) {
			printf("\13 \rYour move? \33K",n);
			getline(ans,80);
			if (*ans=='s' || *ans=='S')
				return;
			if (*ans=='n' || *ans=='N')
				break;
			n=atoi(ans);
			if (n<0)
				return;
			if (n==0 || n>64) {
				printf("\13a\rIllegal move, 1-64 allowed\33K");
				i--;
				continue;
			}
			if (x[n]) {
				printf("\13a\rThat location is already taken!!!\33K");
				i--;
				continue;
			}
			dispmove(n,1);
			n=move(n);
			if (n>0) {
				printf("Again? \33K");
				getline(ans,80);
				if (*ans!='y' && *ans!='Y')
					return;
				break;
			}
		}
	}
}
board()
{
	int i,j,k,n,ii;
	char mov2,mov3;
	static char c[]=" ---=|||l";
	printf("\f\33\60d");
	for(i=1;i<5;i++)
		for (ii=0;ii<4;ii++) {
			mov2=(char)(67+i+5*ii);
			mov3=(char)(30-3*i);
			n=65-16*i+4*ii;
			printf("\13%c\20%c",mov2,mov3+mov3/10*6);
			printf("%2d",n);
			for (j=1;j<4;j++) {
				for (k=1;k<15;k++)
					putchar(c[i]);
				printf("%2d",++n);
			}
			if (ii<3)
				for (j=1;j<5;j++) {
					++mov2;
					for (k=0;k<4;k++) {
						mov3=(char)(31-3*i+16*k);
						printf("\13%c\20%c",mov2,mov3+mov3/10*6);
						putchar(c[4+i]);
					}
				}
		}
	printf("\13\2\33\60A\13@\20\1\n");
}
dispmove(n,player)
int n,player;
{
	static char piece[]="CP";
	int h,v,dm;
	if (!player)
		printf("I move to %2d\33K",n);
	dm=(player>1)?0:1;
	player=(player==2)?1:player;
	player=(player==3)?0:player;
	v=7-(n-1)/16+(n-1)%16/4*5;
	h=100+(n-1)/16*3+(n-1)%4*16;
	printf("\13%c\20%c\33\60A\b\b\b\33\60%c%c\13@\20\1\n",(char)v+64,(char)(h+h/10*6),dm?'@':'P',piece[player]);
}
#define      FNL    (x[m[0][i]]+x[m[1][i]]+x[m[2][i]]+x[m[3][i]])
move(n)
int n;
{
	int r,s,t,i,j,k,a,b,rr[76];
	x[n]=8;
	for (i=1;i<65;i++) {
		if (x[i]==8) {
			p[i]=907;
			c[i]=0;
		} else
		if (x[i]==40) {
			c[i]=907;
			p[i]=0;
		} else
		c[i]=p[i]=1;
	}
	for (i=0;i<76;i++) {
		a=b=0;
		for (j=0;j<4;j++) {
			a+=x[m[j][i]]==8?1:0;
			b+=x[m[j][i]]==40?1:0;
		}
		if (!b) {
			if (a==4) {
				lose(m[0][i],m[1][i],m[2][i],m[3][i]);
				return(2);
			}
			for (j=0;j<4;j++)
				if (p[m[j][i]]!=907)
					p[m[j][i]]*=(a==3)?127:(a==2)?17:(a==1)?2:1;
		}
	}
	for (i=0;i<76;i++) {
		a=b=0;
		for (j=0;j<4;j++) {
			a+=x[m[j][i]]==40?1:0;
			b+=x[m[j][i]]==8?1:0;
		}
		if (!b) {
			if (a==3) {
				for (j=0;x[m[j][i]]==40;j++);
				dispmove(m[j][i],0);
				win(m[0][i],m[1][i],m[2][i],m[3][i]);
				return(1);
			}
			for (j=0;j<4;j++)
				if (c[m[j][i]]!=907)
					c[m[j][i]]*=(a==2)?17:(a==1)?2:1;
		}
	}
	for (i=1;i<65;i++)
		if (p[i] && p[i]/127*127==p[i]) {
			x[i]=40;
			printf("Nice try -- ");
			dispmove(i,0);
			return(-1);
		}
	for (i=1;i<65;i++)
		if (c[i] && c[i]/289*289==c[i]) {
			x[i]=40;
			printf("Let's see you get out of this!!!  ");
			dispmove(i,0);
			return(-2);
		}
	for (i=0;i<76;i++) {
		t=a=b=r=0;
		for (j=0;j<4;j++) {
			if (s=c[m[j][i]] , s/17*17==s) {
				a++;
				if (s>t && s!=907) {
					t=s;
					k=m[j][i];
				}
				b+=s?0:1;
			}
			r+=(s==907);
		}
		if (!b && a>2) {
			x[k]=40;
			dispmove(k,0);
			return(-3);
		}
		if (!b && a==2 && r==1) {
			x[k]=40;
			dispmove(k,0);
			return(-4);
		}
	}
	for (i=1;i<65;i++)
		if (p[i] && p[i]/289*289==p[i]) {
			x[i]=40;
			printf("You fox!!!  Just in the nick of time, ");
			dispmove(i,0);
			return(-5);
		}
	for (i=0;i<76;i++) {
		t=a=b=r=0;
		for (j=0;j<4;j++) {
			if (s=p[m[j][i]] , s/17*17==s) {
				a++;
				if (s>t && s!=907) {
					t=s;
					k=m[j][i];
				}
				b+=s?0:1;
			}
			r+=(s==907);
		}
		if (!b && a>2) {
			x[k]=40;
			dispmove(k,0);
			return(-6);
		}
		if (!b && a==2 && r==1) {
			x[k]=40;
			dispmove(k,0);
			return(-7);
		}
	}
	for (i=0;i<16;i++)
		if (!x[y[i]]) {
			x[y[i]]=40;
			dispmove(y[i],0);
			return(0);
		}
	for (i=1;i<65;i++)
		if (!x[i]) {
			x[i]=40;
			dispmove(y[i],0);
			return(0);
		}
}
win(a,b,c,d)
int a,b,c,d;
{
	dispmove(a,3);
	dispmove(b,3);
	dispmove(c,3);
	dispmove(d,3);
	printf("\20\23, and win:%5d%5d%5d%5d\33K\n",a,b,c,d);
}
lose(a,b,c,d)
int a,b,c,d;
{
	dispmove(a,2);
	dispmove(b,2);
	dispmove(c,2);
	dispmove(d,2);
	printf("CONGRATULATIONS!!!  You won:%5d%5d%5d%5d\33K\n",a,b,c,d);
}
