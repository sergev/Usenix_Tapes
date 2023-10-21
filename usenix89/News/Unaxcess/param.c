/*
 *	@(#)param.c	1.1 (TDI) 7/18/86 21:01:51
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:51 by brandon
 *     Converted to SCCS 07/18/86 21:01:51
 *	@(#)Copyright (C) 1984, 85, 86 by Brandon S. Allbery.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:51 by brandon
 *     Converted to SCCS 07/18/86 21:01:51
 *
 *	@(#)This file is part of UNaXcess version 0.4.5.
 *
 * Modification History:
 *
 *   Ver. 1.1 created 07/18/86 at 21:01:51 by brandon
 *     Converted to SCCS 07/18/86 21:01:51
 *
 *	Permission is hereby granted to copy and distribute this program
 *	freely.  Permission is NOT given to modify this program or distribute
 *	it at cost, except for charging a reasonable media/copying fee.
 */

#ifndef lint
static char _FileID_[] = "@(#)param.c	1.1 (TDI) 7/18/86 21:01:51";
static char _UAID_[]   = "@(#)UNaXcess version 0.4.5";
#endif lint

#include "ua.h"

struct sys parms = {
#ifdef NOAUTOPATH
	NOAUTOPATH,
#else
	"/usr/unaxcess",
#endif NOAUTOPATH
	1,
	0,
	"ua-edit",
	"/bin/sh",
	1,
	"unaxcess",
	30,
	"sysop",
	1,
	0,
	"",
	"",
	1,
	3,
	"trap '' 2; stty -echo; echo 'Begin sending your file.  End with a CONTROL-D.'; cat - > %s; stty echo",
	"trap '' 2; cat %s",
	"umodem -rb",
	"umodem -sb",
	"kermit -iwr",
	"kermit -iws",
};

#define NUM		0
#define STR		1
#define BOOL		2

struct rparm {
	char *parmname;
	char parmtype;
	char *parmval;
} sysparms[] = {
	"bbs-directory",STR,	parms.ua_home,
	"readonly",	BOOL,	&parms.ua_roc,
	"x-rated",	BOOL,	&parms.ua_xrc,
	"editor",	STR,	parms.ua_edit,
	"shell",	STR,	parms.ua_shell,
	"read-env",	BOOL,	&parms.ua_env,
	"bbs-user",	STR,	parms.ua_bbs,
	"time-limit",	NUM,	&parms.ua_tlimit,
	"sysop",	STR,	parms.ua_sysop,
	"private-msgs",	BOOL,	&parms.ua_pm,
	"logging",	BOOL,	&parms.ua_log,
	"banner",	STR,	parms.ua_bnr,
	"login-msg",	STR,	parms.ua_login,
	"pauses",	NUM,	&parms.ua_hco,
	"login-tries",	NUM,	&parms.ua_nla,
	"ascii-upload",	STR,	parms.ua_auc,
	"ascii-download",STR,	parms.ua_adc,
	"xmodem-upload",STR,	parms.ua_xuc,
	"xmodem-download",STR,	parms.ua_xdc,
	"kermit-upload",STR,	parms.ua_kuc,
	"kermit-download",STR,	parms.ua_kdc,
	0,		0,	0,
};

/*
 * 1. Get home directory
 * 2. Open $HOME/uaconfig
 * 3. Parse lines; # is a comment, input form is KEYWORD VALUE
 *    VALUE is numeric, Y/N, "string" and backslash escapes for
 *     \n \t \r \b \f \e \nnn are understood
 * 4. Assign the values to the parms structure
 */

static char line[512], var[20], sval[50];
 
getparms() {
 	char home[512];
 	FILE *cfp;
 	short nval, cnt, pos, scnt, canon;
 	
#ifdef NOAUTOPATH
	strcpy(home, NOAUTOPATH);
#else
 	strcpy(home, getpwuid(geteuid())->pw_dir);
#endif NOAUTOPATH
	strcpy(parms.ua_home, home);
 	strcpy(line, home);
 	strcat(line, "/");
 	strcat(line, CONFIG);
 	if ((cfp = fopen(line, "r")) == NULL) {
 		fprintf(stderr, "panic: param get, %s\n", line);
 		exit(1);
 	}
 	while (fgets(line, 512, cfp) != NULL) {
 		line[strlen(line) - 1] = '\0';
 		if (Index(line, '#') != NULL)
 			*(Index(line, '#')) = '\0';
 		scnt = 0;
 		pos = 0;
 		while (line[pos] != '\0' && line[pos] != ' ' && line[pos] != '\t')
 			var[scnt++] = line[pos++];
 		var[scnt] = '\0';
 		if (var[0] == '\0')
 			continue;
 		for (cnt = 0; sysparms[cnt].parmname != NULL; cnt++)
 			if (strcmp(sysparms[cnt].parmname, var) == 0)
 				break;
 		if (sysparms[cnt].parmname == NULL) {
 			fprintf(stderr, "Please inform the sysop that there is an invalid parameter\n%s in the setup file.\n", var);
 			continue;
 		}
		while (line[pos] == ' ' || line[pos] == '\t')
			pos++;
 		switch (sysparms[cnt].parmtype) {
 			case NUM:
 				*((char *) sysparms[cnt].parmval) = atoi(&line[pos]) & 0xff;
 				break;
 			case BOOL:
 				if (line[pos] == '\0' || ToLower(line[pos]) == 'y')
 					*((char *) sysparms[cnt].parmval) = 1;
 				else
 					*((char *) sysparms[cnt].parmval) = 0;
 				break;
 			case STR:
 				if (line[pos] == '"') {
 					canon = 1;
 					pos++;
 				}
 				for (scnt = 0; (canon? line[pos] != '"': line[pos] != '\0' && line[pos] != ' ' && line[pos] != '\t'); pos++, scnt++) {	
 					if (canon && line[pos] == '\\') {
 						switch (line[++pos]) {	
 							case 'n':
 								sval[scnt] = '\n';
 								break;
 							case 't':
 								sval[scnt] = '\t';
 								break;
 							case 'r':
 								sval[scnt] = '\r';
 								break;
 							case 'b':
 								sval[scnt] = '\b';
 								break;
 							case 'f':
 								sval[scnt] = '\f';
 								break;
 							case 'e':
 							case 'E':
 								sval[scnt] = '\033';
 								break;
 							case 'a':
 								sval[scnt] = '\7';	/* proposed extension of C string metasyntax */
 								break;
 							case '0':
 							case '1':
 							case '2':
 							case '3':
 							case '4':
 							case '5':
 							case '6':
 							case '7':
 								sval[scnt] = 0;
 								while (Index("01234567", line[pos]) != NULL)
 									sval[scnt] = sval[scnt] * 8 + (line[pos++] - '0');
								pos--;
								break;
							default:
								sval[scnt] = line[pos];
 						}
 					}
 					else
 						sval[scnt] = line[pos];
 				}
 				sval[scnt] = '\0';
 				strcpy(sysparms[cnt].parmval, sval);
 		}
 	}
}
