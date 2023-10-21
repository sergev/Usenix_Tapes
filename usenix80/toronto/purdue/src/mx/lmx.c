#

/*
 *	mx library
 */

#define MINF    1
#define MAXF    32

char    mnm[]   "/dev/mx/X";

struct hosts {
	char    num;
	char    *str;
} hosts[] {
	1,      "p",
	1,      "potr",
	2,      "a",
	3,      "b",
	4,      "aa",
	5,      "sp",
	0
};

mxfile()
{
	register i,j;
	for(i = MINF ; i <= MAXF ; i++) {
		mnm[8] = '@' + i;
		if((j = open(mnm,2)) < 0) continue;
		return(j);
	}
	return(-1);
}

mxcon(f,h,s)
{
	return(mxxs(f,1,h,s));
}

mxdis(f)
{
	return(mxxs(f,2,0,0));
}

mxwait(f,s)
{
	return(mxxs(f,3,s,0));
}

mxpgrp(f)
{
	return(mxxs(f,5,0,0));
}

mxsig(f,s)
{
	return(mxxs(f,6,s,0));
}

mxeof(f)
{
	return(mxxs(f,7,0,0));
}

mxxs(f,a,b,c)
{
	int s[3];
	s[0] = a;  s[1] = b;  s[2] = c;
	return(stty(f,s));
}

mxscon(f,hs,s)
{
	register struct hosts *hp;
	for(hp = &hosts[0] ; hp->num ; hp++) {
		if(strcmp(hs,hp->str)) continue;
		return(mxcon(f,hp->num,s));
	}
	return(-1);
}

mxserve(s)
{
	register i;
	clrtty();
	for(i=0;i<10;i++) close(i);

	for(;;) {
		while(mxfile() != 0) sleep(10);
		while(mxwait(0,s)) sleep(10);
		while((i=fork()) == -1) sleep(10);
		if(i == 0) {
			dup(0); dup(0);
			return(0);
		}
		close(0);
		while((i=fork()) == -1) sleep(10);
		if(i) exit(0);
	}
}
