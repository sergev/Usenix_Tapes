#define D_FILES
#include	"empdef.h"
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/dir.h>

info()
{
	register char	*cp;
	struct	stat	statbuf;
	char	 *copy(), *ctime();

	if( argp[1] != 0 ) goto X30;

	ilist();
	return(NORM_RETURN);
X30:	
	cp = copy(infodir, junk);
	*cp++ = '/';
	copy(argp[1], cp);
	if( stat(junk, &statbuf) <  0 ) goto X134;
	if( argp[1][0] == '.' ) goto X134;
	if( argp[1][0] != '/' ) goto X160;
X134:	
	printf("There is no info on %s\n", argp[1]);
	return(SYN_RETURN);
X160:	
	printf("Info on %s -- Last mod : %s", argp[1], ctime(&statbuf.st_mtime));
	if( fork() != 0 ) goto X316;
	execl(nroffil, "nroff", "-s", nroffhd, junk, 0);
	printf("Sorry, %s is not execlable\n", nroffil);
	exit();
X316:	
	if( wait((int *)0) >= 0 ) goto X316;
	return(NORM_RETURN);
}

ilist()
{
	register char	*cp;
	register	i;
	int	n, n4, ifh, compar();
	struct	direct	ent[256];
	char	 *copy();

	if( (ifh = open(infodir, O_RDONLY)) < 0 ) {
		perror(infodir);
		return;
	}
	printf("Information is currently available on:\n");
	i = n = read(ifh, ent, 256*sizeof(struct direct))/sizeof(struct direct);
	i++;
	goto X140;
X124:	
	ent[i].d_ino = 0;
X140:	
	i--;
	if( i >= 0 ) goto X124;
	i = 0;
	goto X264;
X150:	
	cp = ent[i].d_name;
	goto X256;
X166:	
	cp = ent[i].d_name;
	copy(ent[n].d_name, cp);
	goto X256;
X234:	
	if( *cp++ != '.' ) goto X256;
	n--;
	if( n > i ) goto X166;
	*cp = 0;
X256:	
	if( *cp != '\0' ) goto X234;
	i++;
X264:	
	if( i < n ) goto X150;
	if( n >  0 ) goto X310;
	exit(1);
X310:	
	qsort(ent, n, sizeof(struct direct), compar);
	n4 = (n + 3)>>2;
	i = 0;
X364:	
	if( i >= n4 ) return;
	printf("%-16.16s", ent[i].d_name);
	if( n <= i + n4 ) goto X506;
	printf("%-16.16s", ent[i+n4].d_name);
X506:	
	if( n <= i + n4 + n4 ) goto X574;
	printf("%-16.16s", ent[i+n4+n4].d_name);
X574:	
	if( n <= i + n4 + n4 + n4 ) goto X656;
	printf(ent[i+n4+n4+n4].d_name);
X656:	
	printf("\n");
	i++;
	goto X364;
}

compar(e1, e2)
char	*e1, *e2;
{
	register char	*cp1, *cp2;
	register	i;

	cp1 = e1 + 2;
	cp2 = e2 + 2;
X24:	
	if( (i = *cp1++ - *cp2) != 0 ) goto X42;
	if( *cp2++ != '\0' ) goto X24;
X42:	
	return(i);
}
