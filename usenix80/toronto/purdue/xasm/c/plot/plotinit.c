#include "/v/wa1yyn/c/plot/plot.h"
plotinit(pstat)
	int pstat[];
/*
 *	Initalize current plotting device.  Clear screen, set modes
 *	and initalize status vector.
 */
{
	register i,j;
	register char *p;

	if((pstat[CD] != HP) && (pstat[CD] != TEK)){
		pstat[CD] == TEK;
		}

    if(pstat[CD] == TEK){
	plotcls(pstat);
	pstat[FILL] = 0;
	setchr(pstat,'1');
	pstat[DEBUG] = 1;
	setvec(pstat,'`');
	return;
	}

    if(pstat[CD] == HP){
	pstat[CM] = 0;
	pstat[HPBP] = pstat[HPBS];
	pstat[HPBC] = 0;
	i = open("/dev/ttyl",0);
	if(i == -1){
		printf("Cant open '/dev/ttyl'.\n");
		return;
		}
	pstat[FDI] = i;
	i = open("/dev/ttyl",1);
	if(i == -1){
		printf("Can't open '/dev/ttyl' for writing.\n");
		return;
		}
/*	put stty stuff here	*/
	pstat[FD] = i;
	plotp(pstat,ESC);    plotp(pstat,'.');	plotp(pstat,'(');
	plotp(pstat,ESC);  plotp(pstat,'.'); plotp(pstat,'M');
	plotp(pstat,':');  plotp(pstat,ESC);
	p = ".I513;5;71;10:";
	while(*p)plotp(pstat,*p++);
/*  Set handshake mode 2, buff size 513 bytes, enquire  = ^E,
	GO string singal = "G\n";				*/
	plotp(pstat,'~');	plotp(pstat,'W');
/* include next two lines if want size of HP plot to
   be same physical size of TEK screen.  
							*/
	pmbpo(pstat,380,366);
	pmbpo(pstat,15240,10668);
	plotp(pstat,'~');	plotp(pstat,'S');
	pmbpo(pstat,10244,7804);
	plotp(pstat,'p');
	pmbpo(pstat,0,7678);
	plotp(pstat,'}');
	pstat[CM] = 1;
	setchr(pstat,'1');
	setvec(pstat,'`');
	goalpha(pstat);
	plotpen(pstat,BLACK);
	}
}
