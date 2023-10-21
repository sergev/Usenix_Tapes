#

/* System Instrumentation utilities - S.R. Kumar, CWRU, Jan. 1979 */

#include "stdio.h"
#include "/usr/sys/param.h"
#include "/usr/sys/inst.h"

#define CASE	CASE	/* Run at ECMP 11/34 UNIX */

long stoptim 0l;	/* Time trace stopped (if any) */
	struct {
		char nam[8];
		int type;
		unsigned value;
	} nl[2] {"_I", 0, 0, 0, 0, 0};
main()
{
	char name[10];
	int stat;
	stoptim = 0l;
	impex(0);	/* Get Instrumentation Trace data from kernel space */
next:
	printf("\nSpecify Utility ('help' for listing) -> ");
	gets(&name);
	stat = 0;
	if (strcmp(name,"help")==0) {
		printf(" \nThe following utilities are available:-\n");
		printf("\n\thelp\t:\tPrints this listing.\n");
		printf("\tstart\t:\tZeros data & initiates trace.\n");
		printf("\trestart\t:\tInitiates trace without zeroing.\n");
		printf("\tstop\t:\tDisables system trace.\n");
		printf("\tdump\t:\tTrace data dumped to specified file.\n");
		printf("\tsnap\t:\tSnapshot dump & restart.\n");
		printf("\tquit\t:\tLeave utility level.\n\n");
	}
	else
		if (strcmp(name,"start")==0)
			stat = start();
	else
		if (strcmp(name,"stop")==0)
			stat = stop();
	else
		if (strcmp(name,"restart")==0)
			stat = restart();
	else
		if (strcmp(name,"snap")==0)
			stat = snap();
	else
		if (strcmp(name,"dump")==0)
			stat = dump();
	else
		if (strcmp(name,"quit")==0) {
			impex(1);	/* Write-out I-data to kernel space */
			exit(0);
		}
	else
			printf(" Unknown Command: %s \n", &name);
	if (stat)
		printf("Abnormal termination status: %d on: %s\n",stat,name);
	goto next;
}

start()
{
	int i;
	long l;
loop:
	printf("\n System Configuration (RF-HS-DV-RK-CPU) is: %o\n",I.inst_conf);
	printf(" Do you wish to alter configuration? - y/n ->");
	gets(&i);
	if (i=='n')
		goto next;
	if (i != 'y')
		goto loop;
	printf("\n Specify octal configuration vector ->");
	gets(&i);
	sscanf(&i,"%o",&I.inst_conf);
	goto loop;
next:
	for (i = 0; i<NSIZ; i++) I.c_stat[i] = 0l;
	I.c_ticks = 0l;
	I.c_intrpt = 0l;
	I.c_sysint = 0l;
	I.c_idle = 0l;
	I.c_user = 0l;
	I.c_rnrun = 0l;
	for (i = 0; i<4; i++) I.swp_stat[i] = 0l;
	I.swp_rnout = 0l;
	I.swp_rnin = 0l;
	I.swp_cnt = 0l;
	I.swp_wds = 0l;
	I.rk_busy = 0;
	for (i = 0; i<RKSIZ; i++) I.rk_stat[i] = 0l;

#ifndef CASE
	I.dv_busy = 0;
	I.hs_busy = 0;
	I.rf_busy = 0;
	for (i = 0; i<DVSIZ; i++) I.dv_stat[i] = 0l;
	for (i = 0; i<HSSIZ; i++) I.hs_stat[i] = 0l;
	for (i = 0; i<RFSIZ; i++) I.rf_stat[i] = 0l;
#endif

	I.tk_nin = 0l;
	I.tk_nout = 0l;
	I.ik_hit = 0l;
	I.ik_miss =0l;
	I.bk_hit = 0l;
	I.bk_miss =0l;
	time(&l);	/* To set d_start & reduce sync errors in d_accum */
	for (i = 0; i<LNRK; i++) {
		I.rk_time[i].d_start = l;
		I.rk_time[i].d_accum = 0l;
		I.rk_numb[i] = 0l;
		I.rk_wds[i] = 0l;
	}

#ifndef CASE
	for (i = 0; i<LNDV; i++) {
		I.dv_time[i].d_start = l;
		I.dv_time[i].d_accum = 0l;
		I.dv_numb[i] = 0l;
		I.dv_wds[i] = 0l;
	}
	for (i = 0; i<LNHS; i++) {
		I.hs_time[i].d_start = l;
		I.hs_time[i].d_accum = 0l;
		I.hs_numb[i] = 0l;
		I.hs_wds[i] = 0l;
	}
	for (i = 0; i<LNRF; i++) {
		I.rf_time[i].d_start = l;
		I.rf_time[i].d_accum = 0l;
		I.rf_numb[i] = 0l;
		I.rf_wds[i] = 0l;
	}
#endif

	time(&I.inst_tim);
	restart();
	return(0);
}

restart()
{
	int i;
	long l;

/* Clear all disk-busy bit vectors & set d_start times to reduce sync errors */

	I.rk_busy = 0;
	time(&l);
	for (i = 0; i<LNRK; i++) I.rk_time[i].d_start = l;

#ifndef CASE
	I.dv_busy = 0;
	I.hs_busy = 0;
	I.rf_busy = 0;
	for (i = 0; i<DVSIZ; i++) I.dv_time[i].d_start = l;
	for (i = 0; i<HSSIZ; i++) I.hs_time[i].d_start = l;
	for (i = 0; i<RFSIZ; i++) I.rf_time[i].d_start = l;
#endif

	I.inst_go = 1;	/* Set flag to start System Trace */
	return(0);
}

stop()
{
	I.inst_go = 0;
	time(&stoptim);
	return(0);
}

snap()
{
	int stat;
	stop();
	stat = dump();
	restart();
	return(stat);
}

dump()
{

/* Get a filename and dump out all data gathered so far */

	char filnam[20];
	int i,stat;
	FILE *fopen(),*n;
	if (stoptim==0l)	/* If Trace not disabled already */
		stop();		/* Disable before dump */
	i = stat = 0;
	n = NULL;
loop:
	getnam(filnam);
	if ((n=fopen(filnam,"w"))==NULL) {
		printf("Can't open file: %s\n",filnam);
		goto loop;
	}
	for (i=0; i<NSIZ; i++)
		fprintf(n,"%ld%s",I.c_stat[i],(i==NSIZ-1||i%5==4) ? "\n" : " ");
	fprintf(n,"%ld%s%ld%s%ld%s%ld%s%ld%s%ld\n",I.c_ticks," ",I.c_intrpt," ",I.c_sysint,
		" ",I.c_idle," ",I.c_user," ",I.c_rnrun);
	for (i=0; i<4; i++)
		fprintf(n,"%ld%s",I.swp_stat[i],(i==3 ? "\n" : " "));
	fprintf(n,"%ld%s%ld%s%ld%s%ld\n",I.swp_rnout," ",I.swp_rnin," ",I.swp_cnt,
		" ",I.swp_wds);
	for (i=0; i<RKSIZ; i++)
		fprintf(n,"%ld%s",I.rk_stat[i],(i==RKSIZ-1||i%5==4) ? "\n" : " ");

#ifndef CASE
	for (i=0; i<DVSIZ; i++)
		fprintf(n,"%ld%s",I.dv_stat[i],(i==DVSIZ-1||i%5==4) ? "\n" : " ");
	for (i=0; i<HSSIZ; i++)
		fprintf(n,"%ld%s",I.hs_stat[i],(i==HSSIZ-1||i%5==4) ? "\n" : " ");
	for (i=0; i<RFSIZ; i++)
		fprintf(n,"%ld%s",I.rf_stat[i],(i==RFSIZ-1||i%5==4) ? "\n" : " ");
#endif

	fprintf(n,"%ld%s%ld%s%ld%s%ld%s%ld%s%ld\n",I.tk_nin," ",I.tk_nout," ",I.ik_hit," ",
		I.ik_miss," ",I.bk_hit," ",I.bk_miss);
	for (i=0; i<LNRK; i++)
		fprintf(n,"%ld%s%ld%s%ld\n", I.rk_time[i].d_accum," ",
			I.rk_numb[i]," ",I.rk_wds[i]);

#ifndef CASE
	for (i=0; i<LNDV; i++)
		fprintf(n,"%ld%s%ld%s%ld\n", I.dv_time[i].d_accum," ",
			I.dv_numb[i]," ",I.dv_wds[i]);
	for (i=0; i<LNHS; i++)
		fprintf(n,"%ld%s%ld%s%ld\n", I.hs_time[i].d_accum," ",
			I.hs_numb[i]," ",I.hs_wds[i]);
	for (i=0; i<LNRF; i++)
		fprintf(n,"%ld%s%ld%s%ld\n", I.rf_time[i].d_accum," ",
			I.rf_numb[i]," ",I.rf_wds[i]);
#endif

	fprintf(n,"%ld%s%ld\n",I.inst_tim," ",stoptim);
	fprintf(n,"%o\n", I.inst_conf);
	stat = ferror(n);
	fclose(n);
	return(stat);
}

getnam(s)
char s[];
{
/* Reads the standard input to get a filename */

	printf("\n Specify name of trace-data storage file -> ");
	gets(s);
	return(0);
}

impex(i)
int i;
{

/* Import/Export (i=0/1) of Instrumentation trace data from/to Unix kernel */

	int mem;
	char *coref;

	coref = "/dev/mem";
	nlist("/unix", nl);	/* Get add of I-structure from sys. namelist */
	if(nl[0].type == -1) {
		printf("\n** Symbol not found in namelist - Run Aborted **\n");
		exit(0);
	}
	mem = open(coref, i);	/* Open kernel space for reading or writing */
	printf("%o\n", nl[0].value); // debug
	seek(mem, nl[0].value, 0);
	if (i == 0)
		read(mem, &I, sizeof I);   /* Import Trace data from kernel */
	else
		write(mem, &I, sizeof I); /* Export Trace values to kernel */
	close(mem);
	return(0);
}
