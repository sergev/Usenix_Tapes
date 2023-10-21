#include "globals.h"
#include "process.h"

#define CKPERIOD(start,end,t) ((start <= end) ? ((t >= start) && (t <= end)) \
					      : ((t >= start) || (t <= end)))
#define CURSIZE(a,b)	((a >= b) ? (a - b) : (1200 + a - b))

process_file(fname,writeerrors)
char *fname;
int writeerrors;
{
    char	tmname[1024];	/* temp file name */
    char	*ptr;
    int		i,j;		/* counters for for loops */
    int		plusmoved;	/* has entry been shifted because of aa '+' */
    int		msgptr = 0;	/* pointer to message in 'line' */
    char	flags[BUFSIZ];	/* options per line */
    FILE	*fil;		/* dates file descriptor */
    FILE	*tfil;		/* temp file descriptor */
    int		plusbmsg;	/* message plus blanks for plus option */
    int		modflag;	/* has dates file been modified? */
    int		modline;	/* has current line been modified? */
    char	incname[BUFSIZ]; /* include file name */

    /* Command option flags */

    bool	execute;	/* execute message line as process? */
    bool	mail;		/* send mail? */
    bool	always;		/* always print? */
    bool	once;		/* print first time only? */
    bool	remind;		/* run remind program? */
    bool	plusdate;	/* move date when ready? */
    bool	pending;	/* should something be done with this line? */
    bool	newpending;	/* pending afterwards? */
    bool	ignore;		/* commented out? */
    bool	ignoresave;	/* keep comment? */
    bool	delete;		/* delete later? */
    bool	includefile;	/* additionally use other dates file? */
    bool	nodates;	/* no dates for parsing on line */
    int		in_period;	/* currently within period? */
    int		real_in_period;
    char	cmdbuf[BUFSIZ];
    int		canwrite;

	fil = fopen(fname,"r");
	if (fil == NULL) {
		perror(fname);
		exit(errno);
	}

	canwrite = 1;

	if (access(fname,2)) {
	    canwrite = 0;
	    if (writeerrors)
		fprintf(stderr,"calend: can't write out %s.\n", fname);
	}

	/* open temp for copying */

	sprintf(tmname,"%s-%c",tmplate,'A'+level);
	tfil = fopen(tmname,"w");
	if (tfil == NULL) {
		perror(tmname);
		exit(errno);
	}

	modflag = 0;

	/* Main loop */

	while(fgets(cmdbuf,sizeof cmdbuf,fil)) {

		/* Reset flags */

		strcpy(flags,"#");
		strcpy(line,"");

		if (*cmdbuf == ':' || *cmdbuf == '#' || *cmdbuf == '<')
		     sscanf(cmdbuf,"%1s%[^\n]\n",flags,line);
		else sscanf(cmdbuf,"%s%[^\n]\n",flags,line);

		mail = NO;
		always = NO;
		once = NO;
		remind = NO;
		plusdate = NO;
		execute = NO;
		delete = NO;
		pending = NO;
		newpending = NO;
		ignore = NO;
		ignoresave = NO;
		nodates = NO;
		includefile = NO;
		ptr = flags;
		lineptr = 0;
		monthmax = 0;
		daymax = 0;
		in_period = 0;
		modline = 0;

		while(*ptr) switch(*ptr++) {
			case 'M' :
			case 'm' :
				mail = YES;
				break;

			case 'A' :
			case 'a' :
				always = YES;
				break;

			case '1' :
				once = YES;
				break;

			case 'D' :
			case 'd' :
				delete = YES;
				break;

			case 'R' :
			case 'r' :
				remind = YES;
				break;

			case '+' :
				plusdate = YES;
				break;

			case 'X' :
			case 'x' :
				execute = YES;
				break;

			case '*' :
				pending = YES;
				break;

			case '<' :
				includefile = YES;
				nodates = YES;
				break;

			case ',' :
				break;

			case ':' :
				ignoresave = YES;
			case '#' :
				ignore = YES;
				nodates = YES;
				break;

			default :
				fprintf(stderr,"Illegal flag %c\n",*(ptr-1));
		}

		if (nodates) {
		    if (includefile) {
			sscanf(line,"%s",incname);
			level++;
			process_file(incname,0);
			level--;
		    }
		    goto copy;
		}

		/* ignore everything but remind lines if remind-only is set */

		if (remonly && !remind)
		{
		    newpending = pending;
		    goto copy;
		}

		/* ignore comments and error lines */

		if (ignore || yyparse()) goto copy;

		plusbmsg = lineptr;

		while (line[lineptr] == ' ' || line[lineptr] == '\t')
			lineptr++;

		msgptr = lineptr;

		if (plusdate) get_plus_parts();

		while (line[lineptr] == ' ' || line[lineptr] == '\t')
			lineptr++;

		if (plusdate && offmonth == 0 && offday == 0)
		    fprintf(stderr,"No offset given for '+' option in: %s\n",
					line+msgptr);

		/* handle '*[{+-}offset]' appropriately */

		if ((type1 * type2) == 0) {
		    dtype = 0;
		    monthmax = 1;
		    month1[0] = 0;
		    month2 = 0;
		}
		else dtype = 1;

		tdate = dtype ? cdate : cwday;

		if ((type1 == -1) || (type2 == -1)) {
		    if ((type1 == 2) || (type2 == 2))
			for (i=0;(i < monthmax) && (! in_period);i++) {
			    dayt1 = month1[i] * 100 + day1[i];
			    dayt2 = add_date(dayt1,star);

			    if (type1 == -1) SWAP(dayt1,dayt2);

			    in_period = CKPERIOD(dayt1,dayt2,tdate);
			}
		    else {
			for (i=0;(i < monthmax) && (! in_period);i++)
			for (j=0;(j < daymax) && (! in_period);j++) {
			    dayt1 = month1[i] * 100 + day1[j];
			    dayt2 = add_date(dayt1,star);

			    if (type1 == -1) SWAP(dayt1,dayt2);

			    in_period = CKPERIOD(dayt1,dayt2,tdate);
			}
		    }
		}
		else {
		    dayt1 = month1[0]*100+day1[0];
		    dayt2 = month2 * 100 + day2;

		    in_period = CKPERIOD(dayt1,dayt2,tdate);
		}

		mptr = message;

		plusmoved = 0;

		if (plusdate && in_period && (newpending = 1,dtype == 0)) {
		    int diff1;
		    int diff2;

		    diff1 = dayt1 - tdate - ((dayt1 <= tdate) ? 0 : 7);
		    diff2 = dayt2 - tdate + ((tdate <= dayt2) ? 0 : 7);

		    dtype = 1;
		    tdate = cdate;

		    plusmoved = 1;

		    dayt1 = add_date(cdate,diff1);
		    dayt2 = add_date(cdate,diff2);

		}

		if (plusdate && (dtype == 1) && (! in_period)
			&& ((monthmax * daymax) == 1)
			&& (offday > 0 || offmonth > 0)) {
		    int diff,newdate,tmon,tday,oldsize, pnd = pending;

		    diff = ((dayt2 < dayt1) ? (dayt1 - dayt2)
					    : (365 - (1 + dayt2 - dayt1))) / 2;
		    newdate = add_date(dayt2,diff);

		    oldsize = 1200;

		    while (CKPERIOD((dayt2+1),newdate,tdate) &&
			   (CURSIZE(newdate,(dayt2+1)) < oldsize)) {

			oldsize = CURSIZE(newdate,(dayt2+1));

			if (! pnd && ! i_opt && ! execute && ! remind) {

			    if (mptr != message) *mptr++ = '\n';

			    create_message(line+lineptr);
			    create_message(" [old message from %2]");
			    real_in_period = in_period;
			    in_period = 1;
			}

			pnd = 0;

			plusmoved = 1;
			modline++;

			tmon = dayt1 / 100 + offmonth;
			tday = dayt1 % 100 + offday;
			dayt1 = add_date((tmon * 100 + tday),0);

			tmon = dayt2 / 100 + offmonth;
			tday = dayt2 % 100 + offday;
			dayt2 = add_date((tmon * 100 + tday),0);

		    }

		    if (CKPERIOD(dayt1,dayt2,tdate)) {
			if (mptr != message) *mptr++ = '\n';
			create_message(line+lineptr);
			in_period = 1;
			pending = 0;
		    }
		}
		else if (in_period) create_message(line+lineptr);

		if (in_period) {

		    if (execute) {
			if (! once) always = YES;
			if (always || (once && ! pending)) execute_thing();
			if (!always) newpending = YES;
		    }
		    else {
			if (remind) run_remind();
			if (mail && ! pending) send_mail();
			if (once && ! pending) pr_message();
			if (always) pr_message();
			if (once || mail) newpending = YES;
			if ((once || mail) &&
			    (!remind)	   &&
			    (!always)	   &&
			    delete) {
				ignore = YES;
				modline++;
			    }
		    }
		    if (delete) newpending = YES;
		    if (plusdate && real_in_period) newpending = YES;
		}
		else {
			newpending = NO;
			if (delete && pending) {
			    ignore = YES;
			    modline++;
			}
		}

	copy:
		if (canwrite) {
		    char opts[10];

		    if (newpending != pending) modline++;

		    if (! modline) {
			if (!clean || !ignore || ignoresave)
			    fprintf(tfil,"%s",cmdbuf);
			continue;
		    }
		    modflag++;

		    ptr = opts;
		    if (ignore) *ptr++ = ignoresave ? ':' : '#';
		    if (execute) *ptr++ = 'x';
		    if (once) *ptr++ = '1';
		    if (always) *ptr++ = 'a';
		    if (remind) *ptr++ = 'r';
		    if (mail) *ptr++ = 'm';
		    if (delete) *ptr++ = 'd';
		    if (plusdate) *ptr++ = '+';
		    if (newpending && ! ignore) *ptr++ = '*';
		    *ptr = 0;

		    if (!clean || !ignore || ignoresave) {
			fprintf(tfil,"%s", opts);

			if (ignore || ! plusmoved) fprintf(tfil,"%s\n", line);
			else {
			    char *l = line;

			    while (*l == ' ' || *l == '\t') putc(*l++,tfil);

			    if (type1 == -1) {
				if (star == 0) fprintf(tfil,"*");
				else if (star < 0) fprintf(tfil,"*%d",star);
				else fprintf(tfil,"*+%d",star);
			    }
			    else {
				mptr = message;
				create_message("%1");
				fprintf(tfil,"%s",message);
			    }
			    fprintf(tfil,"\t");
			    if (type2 == -1) {
				if (star == 0) fprintf(tfil,"*");
				else if (star < 0) fprintf(tfil,"*%d",star);
				else fprintf(tfil,"*+%d",star);
			    }
			    else {
				mptr = message;
				create_message("%2");
				fprintf(tfil,"%s",message);
			    }

			    if (lastchar != '}') putc(lastchar,tfil);
			    fprintf(tfil,"%s\n",line+msgptr);
			}
		    }
		}
	}

	fclose(tfil);

	if (canwrite && modflag) copy_file(tmname,fname);
	unlink(tmname);
}
