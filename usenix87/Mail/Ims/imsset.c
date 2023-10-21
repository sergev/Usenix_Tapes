#include "ims.h"

struct var {
	char *v_name;
	char *v_val;
} var[] = {
	"folder",			folder,
	"cabinet",			cabinet,
	"prompt",			prompt,
	"pager",			pager,
	"system-mailbox",		sysmbox,
	"mail-sending-command",		sender,
	"mailbox",			mailbox,
	"editor",			editor,
	"save-folder",			savefolder,
	"print-command",		printcmd,
	"type-next-automatically",	autonext,
	"read-new-mail-automatically",	autoread,
	"ask-before-append",		askappend,
	"lines",			lines,
	"edit-forwarded-mail",		edforward,
	"alias-recursion-level",	alicount,
	(char *) 0,			(char *) 0,
};

readvars() {
	char vfile[256];
	char *cp;
	
	varead(IMSINIT);
	if ((cp = getenv("IMSINIT")) != (char *) 0)
		strcpy(vfile, cp);
	else {
		if ((cp = getenv("HOME")) == (char *) 0)
			cp = "/usr/guest";
		sprintf(vfile, "%s/.imsinit", cp);
	}
	varead(vfile);
}

varead(vfile)
char *vfile; {
	char vline[512], v[80], val[128];
	char *cp, *dp, *ip;
	FILE *fp;
	int lineno, canon;

	if ((fp = efopen(vfile, "r")) == (FILE *) 0)
		return;
	lineno = 0;
	while (fgets(vline, sizeof vline, fp) != (char *) 0) {
		lineno++;
		if ((cp = strchr(vline, '\n')) != (char *) 0)
			*cp = '\0';
		if ((cp = strchr(vline, '#')) != (char *) 0)
			*cp = '\0';
		for (cp = vline; *cp == ' ' || *cp == '\t'; cp++)
			;
		if (*cp == '\0')
			continue;
		for (dp = v; *cp != '\0' && strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01235456789_-", *cp) != (char *) 0; cp++)
			*dp++ = *cp;
		*dp = '\0';
		if (v[0] == '\0') {
			printf("Error in \"%s\", line %d:  Missing or invalid variable name\n", vfile, lineno);
			continue;
		}
		while (*cp == ' ' || *cp == '\t')
			cp++;
		if (*cp != '=') {
			printf("Error in \"%s\", line %d:  Missing ``=''\n", vfile, lineno);
			continue;
		}
		cp++;
		while (*cp == ' ' || *cp == '\t')
			cp++;
		canon = 0;
		if (*cp == '"') {
			canon = 1;
			cp++;
		}
		for (dp = val; *cp != '\0' && (canon? *cp != '"': *cp != ' ' && *cp != '\t'); cp++) {
			if (*cp == '\\' && *(cp + 1) != '\0')
				*dp++ = *++cp;
			else if (*cp != '\\')
				*dp++ = *cp;
			else {
				*dp++ = '\n';
				if (fgets(vline, sizeof vline, fp) == (char *) 0) {
					cp++;
					printf("Unexpected end of file in \"%s\", line %d\n", vfile, lineno + 1);
					break;
				}
				lineno++;
				if ((ip = strchr(vline, '\n')) != (char *) 0)
					*ip = '\0';
				if ((ip = strchr(vline, '#')) != (char *) 0)
					*ip = '\0';
				cp = vline;
				*dp++ = *cp;
			}
		}
		if (canon) {
			if (*cp == '\0') {
				printf("Error in \"%s\", line %d:  No closing quote\n", vfile, lineno);
				continue;
			}
			cp++;
		}
		*dp = '\0';
		while (*cp == ' ' || *cp == '\t')
			cp++;
		if (*cp != '\0') {
			printf("Error in \"%s\", line %d:  Text after set command\n", vfile, lineno);
			continue;
		}
		setvar(v, val);
	}
	fclose(fp);
}

setvar(v, val)
char *v, *val; {
	struct var *vp;
	
	for (vp = var; vp->v_name != (char *) 0; vp++)
		if (strcmp(vp->v_name, v) == 0)
			break;
	if (vp->v_name == (char *) 0) {
		printf("Unknown variable: %s\n", v);
		return -1;
	}
	strcpy(vp->v_val, val);
	return 0;
}

varops(cmdp)
char *cmdp; {
	char *cp, *dp;
	char v[80], val[128];
	int canon;
	
	while (*cmdp == ' ' || *cmdp == '\t')
		;
	if (*cmdp == '\0') {
		listvar("");
		return 1;
	}
	for (dp = v, cp = cmdp; *cp != '\0' && strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01235456789_-", *cp) != (char *) 0; cp++)
		*dp++ = *cp;
	*dp = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '\0') {
		listvar(v);
		return 1;
	}
	if (*cp != '=') {
		puts("Invalid assignment statement syntax.  Usage:  set [variable [= value]]");
		return -1;
	}
	cp++;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	canon = 0;
	if (*cp == '"') {
		canon = 1;
		cp++;
	}
	for (dp = val; *cp != '\0' && (canon? *cp != '"': *cp != ' ' && *cp != '\t'); cp++) {
		if (*cp == '\\' && *(cp + 1) != '\0')
			*dp++ = *++cp;
		else if (*cp != '\\')
			*dp++ = *cp;
		else {
			puts("Multiline variables invalid on command line.");
			return -1;
		}
	}
	if (canon) {
		if (*cp == '\0') {
			printf("Missing closing quote.");
			return -1;
		}
		cp++;
	}
	*dp = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp != '\0') {
		printf("Extra text after assignment.");
		return -1;
	}
	setvar(v, val);
	return 1;
}

listvar(v)
char *v; {
	struct var *vp;
	int cnt;
	
	cnt = 0;
	for (vp = var; vp->v_name != (char *) 0; vp++)
		if (v[0] == '\0' || strcmp(vp->v_name, v) == 0) {
			printf("%s => \"%s\"\n", vp->v_name, vp->v_val);
			cnt++;
		}
	if (cnt == 0)
		if (v[0] == '\0')
			puts("There are no variables in this version of IMS.");
		else
			printf("Unknown variable: \"%s\".\n", v);
}
