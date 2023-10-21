#
#include <stdio.h>
#include <math.h>
#include "bastrans.h"
#define NPKEYS	(sizeof(p_c) / sizeof(struct p))
#define MAXLINE	128
#define abs(x)	(int)(sqrt((double)(x*x)))
FILE *fi, *fopen();
FILE *fo, *fopen();
FILE *fv, *fopen();

/************************************************************************
 *									*
 *			BASTRANS.C	V6.91.0				*
 *									*
 *	2 Mar 82				S. Klyce		*
 *									*
 *	This program partially translates BASIC programs to the C	*
 *  Language.  It is a line for line literal translator.  For the	*
 *  simplist BASIC programs, compilable source is produced.  For more	*
 *  advanced programs, the output will contain untranslated BASIC       *
 *  commands prefixed with "?".						*
 *	This version (V6.91.0) is relatively dumb without apology 	*
 *  to keep the code compact enough (<64 Kbytes) to run on "small"	*
 *  machines.  An even smaller version could be implemented by moving	*
 *  structure b (the input program) out to disk, but execution time	*
 *  will suffer tremendously!						*
 *									*
 ************************************************************************/

struct b {
	int num;
	int ref;
	char command[CLEN];
} b_line[NLINES];
	/* Below are all the commands in one version of APPLE BASIC */
	/* If your BASIC has additional commands, they should be    */
	/* entered in the list.					    */
	/* N.B.:  Many of these are obviously machine dependent	    */
	/* stuffs meaning sommething only to micros - we just	    */
	/* comment these out in C.				    */
struct p {
	char psee[PLEN],pcmd[PLEN],scmd[PLEN];
} p_c[] = {
	"IF"	,	"if ("	,	"",		/*  0 */
	"RETURN",	"return(",	")}",		/*  1 */
	"PRINT"	,	"printf(\"",	");",		/*  2 */
	"REM"	,	"\/* "	,	" *\/",		/*  3 */
	"INPUT"	,	"scanf(\"" ,	");",		/*  4 */
	"GOSUB"	,	"f"	,	"();",		/*  5 */
	"END"	,	"exit()",	"",		/*  6 */
	"GOTO"	, 	"goto l",	";",		/*  7 */
	"LET"	,	""	,	"",		/*  8 */
	"ABS"	,	"fabs"	,	"",		/*  9 */
	"AND"	,	" && "	,	"",		/* 10 */
	"ASC"	,	""	,	"",		/* 11 */
	"ATN"	,	"atan"	,	"",		/* 12 */
	"AT"	,	""	,	"",		/* 13 */
	"CALL"	,	""	,	"",		/* 14 */
	"CHR$"	,	""	,	"",		/* 15 */
	"CLEAR"	,	""	,	"",		/* 16 */
	"COLOR=",	""	,	"",		/* 17 */
	"CONT"	,	""	,	"",		/* 18 */
	"COS"	,	"cos"	,	"",		/* 19 */
	"DATA"	,	""	,	"",		/* 20 */
	"DEF"	,	""	,	"",		/* 21 */
	"DEL"	,	""	,	"",		/* 22 */
	"DIM"	,	""	,	"",		/* 23 */
	"DRAW"	,	""	,	"",		/* 24 */
	"EXP"	,	"exp"	,	"",		/* 25 */
	"FLASH"	,	""	,	"",		/* 26 */
	"FN"	,	""	,	"",		/* 27 */
	"FOR"	,	"for ("	,	")",		/* 28 */
	"FRE"	,	""	,	"",		/* 29 */
	"GET"	,	""	,	"",		/* 30 */
	"GR"	,	""	,	"",		/* 31 */
	"HCOLOR=",	""	,	"",		/* 32 */
	"HGR"	,	""	,	"",		/* 33 */
	"HGR2"	,	""	,	"",		/* 34 */
	"HIMEM:",	""	,	"",		/* 35 */
	"HLIN"	,	""	,	"",		/* 36 */
	"HOME"	,	""	,	"",		/* 37 */
	"HPLOT"	,	""	,	"",		/* 38 */
	"HTAB"	,	""	,	"",		/* 39 */
	"IN#"	,	""	,	"",		/* 40 */
	"INT"	,	"(int)"	,	"",		/* 41 */
	"INVERSE",	""	,	"",		/* 42 */
	"LEFT$"	,	""	,	"",		/* 43 */
	"LEN"	,	""	,	"",		/* 44 */
	"LIST"	,	""	,	"",		/* 45 */
	"LOAD"	,	""	,	"",		/* 46 */
	"LOG"	,	"log"	,	"",		/* 47 */
	"LOMEM:",	""	,	"",		/* 48 */
	"MID$"	,	""	,	"",		/* 49 */
	"NEW"	,	""	,	"",		/* 50 */
	"NEXT"	,	""	,	"",		/* 51 */
	"NORMAL",	""	,	"",		/* 52 */
	"NOT"	,	"!"	,	"",		/* 53 */
	"NOTRACE",	""	,	"",		/* 54 */
	"ON"	,	""	,	"",		/* 55 */
	"ONERR"	,	""	,	"",		/* 56 */
	"OR"	,	" || "	,	"",		/* 57 */
	"PDL"	,	""	,	"",		/* 58 */
	"PEEK"	,	""	,	"",		/* 59 */
	"PLOT"	,	""	,	"",		/* 60 */
	"POKE"	,	""	,	"",		/* 61 */
	"POP"	,	""	,	"",		/* 62 */
	"POS"	,	""	,	"",		/* 63 */
	"PR#"	,	""	,	"",		/* 64 */
	"READ"	,	""	,	"",		/* 65 */
	"RECALL",	""	,	"",		/* 66 */
	"RESTORE",	""	,	"",		/* 67 */
	"RESUME",	""	,	"",		/* 68 */
	"RETURN",	""	,	"",		/* 69 */
	"RIGHT$",	""	,	"",		/* 70 */
	"RND"	,	""	,	"",		/* 71 */
	"ROT="	,	""	,	"",		/* 72 */
	"RUN",		"main()",	"",		/* 73 */
	"SAVE"	,	""	,	"",		/* 74 */
	"SCALE=",	""	,	"",		/* 75 */
	"SCRN("	,	""	,	"",		/* 76 */
	"SGN"	,	""	,	"",		/* 77 */
	"SHLOAD",	""	,	"",		/* 78 */
	"SIN"	,	"sin"	,	"",		/* 79 */
	"SPC("	,	""	,	"",		/* 80 */
	"SPEED=",	""	,	"",		/* 81 */
	"SQR"	,	"sqrt"	,	"",		/* 82 */
	"STEP"	,	""	,	"",		/* 83 */
	"STOP"	,	"exit();"	,	"",	/* 84 */
	"STORE"	,	""	,	"",		/* 85 */
	"STR$"	,	""	,	"",		/* 86 */
	"TAB"	,	"\\t"	,	"",		/* 87 */
	"TAN"	,	"tan"	,	"",		/* 88 */
	"TEXT"	,	""	,	"",		/* 89 */
	"TO"	,	""	,	"",		/* 90 */
	"TRACE"	,	""	,	"",		/* 91 */
	"USR"	,	""	,	"",		/* 92 */
	"VAL"	,	""	,	"",		/* 93 */
	"VLIN"	,	""	,	"",		/* 94 */
	"VTAB"	,	""	,	"",		/* 95 */
	"WAIT"	,	""	,	"",		/* 96 */
	"XPLOT"	,	""	,	"",		/* 97 */
	"XDRAW"	,	""	,	"",		/* 98 */
	"^"	,	""	,	""		/* 99 */
	};
char lref[12] "xxxxxxxxxxx";

main(argc,argv)
int argc;
char **argv;
{
	int totlines;
	int tnum;
	char num[20];
	int lim = NLINES;
	int i,j,q;
	int cntr;
	if (argc < 2) error(USE,"");
	if ((fi=fopen(*++argv,"r"))<=0) error(FRE,*argv);
	if (index(*argv,".bas",0) <= 0) error(FNB,*argv);
	while(lim > 0 && fscanf(fi,"%d",&b_line[NLINES-lim].num) != EOF)
	{
		cntr=NLINES-lim;
		fgets(b_line[cntr].command,CLEN,fi);
		printf("%d\t%s",b_line[cntr].num,b_line[cntr].command);
		squeeze(b_line[cntr].command,' ',0,1);
		for(q=i=0;b_line[cntr].command[i] != '\0';) {
			if (b_line[cntr].command[i] == '\"') q++;
			if (q==2) q=0;
			if (!q) {
				b_line[cntr].command[i++] = upper(b_line[cntr].command[i]);
			}
		}
		for (i=0;i<NPKEYS;i++)
			parse(b_line[cntr].command,p_c[i].psee,p_c[i].pcmd,p_c[i].scmd,i,&b_line[cntr].ref);
		--lim;
	}
	fclose(fi);
	if (!lim) error(OS);
	totlines = NLINES - --lim;
	if ((fv = fopen("/tmp/bvar","w"))<=0) error(FWE,"/tmp/bvar");
	for (i=0;i<totlines;i++) {
		squeeze(b_line[i].command,'\n',0,1);
		replace(b_line[i].command,":",";",0,1);
		if (b_line[i].command[strlen(b_line[i].command)-1] != ';')
			cat(b_line[i].command,";");
		if (b_line[i].ref) {
				for (j=0;j<=totlines && fabs((double)(b_line[j].num)) != fabs((double)(b_line[i].ref));j++);
				if (j>totlines) printf("Reference to missing line made in:\n\t%d %s\n",b_line[i].num,b_line[i].command);
				else if (b_line[j].num > 0)
					b_line[j].num *= -1;
		}
		varsrch(fv,b_line[i].command);
	}
	fclose(fv);
	system("sort -d /tmp/bvar | uniq > /tmp/sbvar");
		/* UNIX system routine used above to save space */
	replace(*argv,".bas",".c",0,0);
	if ((fo=fopen(*argv,"w"))<=0) error(FWE,*argv);
	puthead(fo,*argv);
	dclvars(fo);
	for (i=0;i<totlines;i++) {
		if (b_line[i].num<0) {
			b_line[i].num *= -1;
			tnum=b_line[i].num;
			itoa(tnum,num);
			lref[0]='l';
			for (j=0;(lref[j+1] = num[j]) != '\0';j++);
			++j;
			lref[j++]=':';
			lref[j]='\0';
			fprintf(fo,"%s",lref);
		}
		fprintf(fo,"\t%s\n",b_line[i].command);
	}
	fprintf(fo,"}\n");
	printf("%d lines translated.  Output stored in %s.\n",totlines,*argv);
	fclose(fo);
}
