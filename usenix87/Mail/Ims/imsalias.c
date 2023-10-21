#include "ims.h"

struct alias {
	char *a_name;
	char *a_value;
	struct alias *a_next;
} *alias = (struct alias *) 0;

readalias() {
	char vfile[256];
	char *cp;
	
	aliread(IMSALIAS);
	if ((cp = getenv("IMSALIAS")) != (char *) 0)
		strcpy(vfile, cp);
	else {
		if ((cp = getenv("HOME")) == (char *) 0)
			cp = "/usr/guest";
		sprintf(vfile, "%s/.imsalias", cp);
	}
	aliread(vfile);
}

aliread(vfile)
char *vfile; {
	char vline[1024], v[80], val[1024];
	char *cp, *dp, *ip;
	FILE *fp;
	int lineno;
	
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
			printf("Error in \"%s\", line %d:  Missing or invalid alias name\n", vfile, lineno);
			continue;
		}
		while (*cp == ' ' || *cp == '\t')
			cp++;
		if (*cp != ':') {
			printf("Error in \"%s\", line %d:  Missing ``:''\n", vfile, lineno);
			continue;
		}
		cp++;
		while (*cp == ' ' || *cp == '\t')
			cp++;
		for (dp = val; *cp != '\0'; cp++) {
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
		setalias(v, val);
	}
	fclose(fp);
}

setalias(ali, value)
char *ali, *value; {
	struct alias *ap, *newa;
	
	for (ap = alias; ap->a_name != (char *) 0; ap = ap->a_next)
		if (strcmp(ap->a_name, ali) == 0)
			break;
	if (ap->a_name == (char *) 0) {
		if (value == (char *) 0 || value[0] == '\0') {
			printf("No such alias: %s.\n", ali);
			return -1;
		}
		if ((newa = (struct alias *) calloc(1, sizeof *newa)) == (struct alias *) 0) {
			fprintf(stderr, "No room for new alias: %s.\n", ali);
			return -1;
		}
		if ((newa->a_name = calloc(strlen(ali) + 1, sizeof *newa->a_name)) == (char *) 0) {
			fprintf(stderr, "No room for new alias name: %s.\n", ali);
			free((char *) newa);
			return -1;
		}
		if ((newa->a_value = calloc(strlen(value) + 1, sizeof *newa->a_value)) == (char *) 0) {
			fprintf(stderr, "No room for new alias value: %s.\n", ali);
			free(newa->a_name);
			free((char *) newa);
			return -1;
		}
		strcpy(newa->a_name, ali);
		strcpy(newa->a_value, value);
		newa->a_next = alias;
		alias = newa;
		return 2;
	}
	free(ap->a_value);
	if (value == (char *) 0 || value[0] == '\0') {
		free(ap->a_name);
		for (newa = alias; newa->a_next != (struct alias *) 0; newa = ap->a_next)
			if (newa->a_next == ap)
				break;
		if (newa == (struct alias *) 0)
			alias = ap->a_next;
		else
			newa->a_next = ap->a_next;
		free((char *) ap);
		return 0;
	}
	if ((ap->a_value = calloc(strlen(value), sizeof *ap->a_value)) == (char *) 0) {
		fprintf(stderr, "No space for new value of alias: %s.  Alias deleted.\n", ali);
		for (newa = alias; newa->a_next != (struct alias *) 0; newa = newa->a_next)
			if (newa->a_next == ap)
				break;
		if (newa == (struct alias *) 0)
			alias = ap->a_next;
		else
			newa->a_next = ap->a_next;
		free(ap->a_name);
		free((char *) ap);
		return -1;
	}
	strcpy(ap->a_value, value);
	return 1;
}

xalias(to, buf)
char *to, *buf; {
	struct alias *ap;
	
	for (ap = alias; ap != (struct alias *) 0; ap = ap->a_next)
		if (strcmp(ap->a_name, to) == 0)
			break;
	if (ap == (struct alias *) 0) {
		strcpy(buf, to);
		return 0;
	}
	strcpy(buf, ap->a_value);
	return 1;
}

unalias(cmdp)
char *cmdp; {
	char *cp;
	int didalias;
	
	didalias = 0;
	
	while (*cmdp != '\0') {
		while (*cmdp == ' ' || *cmdp == '\t')
			cmdp++;
		if (*cmdp == '\0')
			break;
		for (cp = cmdp; *cp != ',' && *cp != ' ' && *cp != '\t'; cp++)
			;
		if (*cp != '\0')
			*cp++ = '\0';
		didalias++;
		setalias(cmdp, "");
		cmdp = cp;
	}
	if (didalias == 0) {
		puts("Usage:  unalias alias-list\n");
		return -1;
	}
	return 1;
}

aliasops(cmdp)
char *cmdp; {
	char *cp, *dp;
	char aexp[1024];
	
	while (*cmdp == ' ' || *cmdp == '\t')
		cmdp++;
	if (*cmdp == '\0')
		return listalias();
	for (cp = cmdp; *cp != '\0' && *cp != '\t' && *cp != ' '; cp++)
		;
	if (*cp != '\0')
		*cp++ = '\0';
	for (dp = cmdp; *dp != '\0' && strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01235456789_-", *dp) != (char *) 0; dp++)
		;
	if (*dp != '\0') {
		printf("Illegal alias name: %s.\n", cmdp);
		return -1;
	}
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '\0')
		if (xalias(cmdp, aexp)) {
			printf("Alias %s: %s\n", cmdp, aexp);
			return 1;
		}
		else {
			printf("Unknown alias: %s.\n", cmdp);
			return -1;
		}
	if (setalias(cmdp, cp) < 0)
		return -1;
	return 1;
}

listalias() {
	struct alias *ap;
	int nalias;
	
	nalias = 0;
	for (ap = alias; ap != (struct alias *) 0; ap = ap->a_next, nalias++)
		printf("  %s: %s\n", ap->a_name, ap->a_value);
	if (nalias != 0)
		return 1;
	puts("No aliases defined.");
	return -1;
}
