
/***************************************************************************
* Hrmeom fcd'l leum lxso hcolsdq omzsciorg. Sl'o dcl aczlx sl. Vclx lxso
* cdm edf lxm hzmbscio hcolsdq amzm NCUMO!!! Cueg? Lxso hezlsyirez fsoyreskmz
* aeo qmdmzelmf vg ed DOE yckhilmz...
*
* Dc, dc... S xebm e hzcqzek lxel mdyshxmzo lxsdqo, edf edclxmz hzcqzek lxel
* fmyshxmzo lxsdqo - S iom sl kcolrg lc tsqizm cil lxcom yzghlcqzeko sd
* Qekmo keqepsdm. S'bm eroc kefm e oxmrr oyzshl lc keum e yzghlcqzek tzck
* tczlidm edf hshm lxel sdlc lxm sdlmzeylsbm fmyzghlscd hzcqzek - e tid rsllrm
* fsbmzoscd. St sdlmzmolmf, omdf km e dclm (xcrrcaeg@fzsbej.IIYH).
*****************************************************************************/

#include <stdio.h>

/* Change #undef to #define if you're running under System V. */

#undef	SYSV

#ifndef	SYSV
#include <sys/file.h>
#endif

#define	PASSWD	"/etc/passwd"

extern char *fgets();
extern char *malloc();
extern int strlen();

char *zalloc();

char iline[1024];

#define	NAMELIST	struct _names
NAMELIST{
    NAMELIST *next;
    char *login;
    char *realname;
    char *home;
    int prot;
};

NAMELIST *names = 0;

main(){
    FILE *fchan;
    extern int time();

#ifdef SYSV
    fprintf(stderr,"Federal Regulations prohibit directory scanning.\");
    exit(0);
}
#else

    srand(time((long *)0));

    if(fchan = fopen(PASSWD,"r")){
	while(fgets(iline,(sizeof iline),fchan)) decode(iline);
	fclose(fchan);
	report();
	}
    else{
	fprintf(stderr,"No %s file - Tag for electronic surveillance.\n",PASSWD);
	exit(2);
	}
}

int rnd(n)
int n;
{
    return((rand() & 0x7FFF) % n);
}

char *memerr[] = {
    "Low", "Unlikely", "Possible", "Likely", "High", "Certain"
};

#define	NUMERRS	((sizeof memerr)/sizeof(char *))

char *zalloc(size)
int size;
{
    char *s;

    s = malloc(size);
    if(!s){
	fprintf(stderr,"*** Insufficient memory ***\n");
	fprintf(stderr,"    Possibility of illegal Warsaw pact memory technology sale - %s\n",
					memerr[rnd(NUMERRS)]);
	exit(1);
	}
    else return(s);
}

decode(s)
char *s;
{
    char *s1, *s2;
    int count=0;
    char field[128];
    NAMELIST *n;
    extern int access();

    for(s1=s; *s1;) count += (*s1++ == ':');
    if(count != 6) return;
    n = (NAMELIST *)zalloc(sizeof(NAMELIST));
    for(s1=s, s2=field; *s1 != ':'; *s2++ = *s1++);
    *s2 = 0;
    strcpy(n->login = zalloc(strlen(field)+1),field);
    for(count=0; count<4;) count += (*s1++ == ':');
    for(s2=field; *s1 != ':'; *s2++ = *s1++);
    *s2 = 0;
    strcpy(n->realname = zalloc(strlen(field)+1),field);
    for(s2 = field, ++s1; *s1 != ':'; *s2++ = *s1++);
    *s2 = 0;
    strcpy(n->home = zalloc(strlen(field)+1),field);
    count = 0;
    if(!access(field,R_OK)) count |= R_OK;
    if(!access(field,W_OK)) count |= W_OK;
    if(!access(field,X_OK)) count |= X_OK;
    n->prot = count;
    n->next = names;
    names = n;
}

char *secure[] = {
    "___ Top Level Clearances Granted",
    "__X Suspected Narcotics Trafficer",
    "_W_ Insufficiently Cautious",
    "_WX Potential Terrorist",
    "R__ Potential Hostage",
    "R_X Potential Security Risk",
    "RW_ Likely Security Risk",
    "RWX High Security Risk"
};

#define	F_NAME	"Login"
#define	F_RNAME	"Identity"
#define	F_CLEARANCE "Security Classification"

report(){
    char zap[128];
    int i;
    NAMELIST *n;

    sprintf(zap,"%-12s%-30s%s",F_NAME,F_RNAME,F_CLEARANCE);
    printf("%s\n",zap);
    for(i=strlen(zap); i--;) putchar('-');
    printf("\n");
    for(n=names; n; n=n->next)
	printf("%-12.12s%-30.30s%s\n",n->login,n->realname,secure[n->prot]);

    printf("\n\nPlease send a copy of this report to smith@jones.NSA.GOV\n");
    printf("Penalty for non-compliance is a $10,000 fine and/or imprisonment.\n");
}

#endif
