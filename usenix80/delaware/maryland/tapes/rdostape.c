char *argv0 0;
int skip 1;
int infile -1;
char buff[44544];
char ufilf;
char gfilf;
char user;
char group;
char *tname;
char *dname "/dev/nrw_rmt0"; /* Default drive is low density */
int header[256];
int newhead[3];

struct  integ {
	char    lobyte;
	char    hibyte;
};

main(argc, argv)
int argc;
char *argv[];
{
	register char **args;
	register int count;

	args = argv;
	argv0 = *args++;
	tname = dname;
	while(**args == '-' && --argc > 0) {
		if(*++*args == 's') {
			skip = atoi(++*args);
		} else if('0' <= **args && **args < '8' && args[0][1] == 0) {
			dname[12] = **args;
			tname = dname;
		} else if(**args == 'u') {
			if(*++*args == 0) ufilf = 0;
			else {
				ufilf++;
				user = atoi(*args);
			}
		} else if(**args == 'g') {
			if(*++*args == 0) gfilf = 0;
			else {
				gfilf++;
				group = atoi(*args);
			}
		} else if(**args == '-' && args[0][1] == 0) {
			tname = "standard input";
			infile = 0;
		} else {
			tname = *args;
		}
		args++;
	}
	if(infile != 0 && (infile = open(tname, 0)) < 0)
		errprnt("cannot open %s", tname);
	if(skip != 0 && argc > 1) {
		makename(*args, newhead);
		for(;;) {
			if((count = read(infile, header,
				(infile == 0 ? 14 : 512))) == 0) {
				close(1); dup(2);
				printf("on %s ", tname);
				errprnt("cannot find %s before EOT", *args);
			}
			if(count < 14)
				errprnt("read error on %s header", tname);
			if(((header[0] == newhead[0] &&
			     header[1] == newhead[1] &&
			     header[2] == newhead[2]) || argc <= 1) &&
			   (ufilf == 0 || header[3].hibyte == user) &&
			   (gfilf == 0 || header[3].lobyte == group)) {
				skip--;
				break;
			}
			if(close(infile) == -1)
				errprnt("close error on %s", tname);
			if((infile = open(tname, 0)) < 0)
				errprnt("cannot reopen %s", tname);
		}
	}

	if(skip) seek(infile, skip, 4);  /* relative by blocks */
	while(count = read(infile, buff, sizeof buff)) {
		if(count == -1)
			errprnt("data read error on %s", tname);
		if(write(1, buff, count) != count)
			errprnt("write error");
	}
}

makename(ascii, rad50)
char *ascii;
int *rad50;
{
	register char *apt, *dot;
	register int *pnt;
	static int cnt, tmp;

	/* find last filename and last dot within it */
	apt = ascii;
	cnt = 0;
	for(dot = apt; *apt != 0;)
		if(*apt == '.') dot = apt++;
		else if(*apt++ == '/') ascii = dot = apt;
	/* translate file name */
	apt = ascii;
	pnt = rad50;
	*pnt = 0;
	for(cnt = 0; cnt < 6;) {
		if((apt != dot || *dot != '.') && *apt != 0) {
			if((tmp = ato50(*apt++)) >= 0) {
				*pnt = *pnt * 050 + tmp;
				if(++cnt == 3) *++pnt = 0;
			}
		} else {
			*pnt =* 050;
			if(++cnt == 3) *++pnt = 0;
		}
	}
	/* translate extension */
	if(*dot == '.') apt = ++dot;
	*++pnt = 0;
	for(cnt = 0; cnt < 3;) {
		if(*apt != 0) {
			if((tmp = ato50(*apt++)) >= 0) {
				cnt++;
				*pnt = *pnt * 050 + tmp;
			}
		} else {
			cnt++;
			*pnt =* 050;
		}
	}
}

ato50(cc)
int cc;
{
	register c;

	c = cc & 0177;
	if('a' <= c && c <= 'z') return(c - 'a' + 1);
	if('0' <= c && c <= '9') return(c - '0' + 036);
	if(c == ' ' || c == '\t') return(0);
	if('A' <= c && c <= 'Z') return(c - 'A' + 1);
	if(c == '-' || c == '_' || c == '$') return(033); /* was just $ */
	if(c == '.') return(034);
	return(035); /* undefined character */
}
