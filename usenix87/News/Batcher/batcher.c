/*	Chris Lewis, June 1986	*/
#include <stdio.h>
#if	defined(BSD4_2) || defined(BSD4_1C)
#include <strings.h>
#else
#include <string.h>
#endif
extern char *malloc();
extern void free();

#define	NUMFLDS	7
#define	NAME	0
#define	CLASS	1
#define	COMPRESS 2
#define	UUXFLAGS 3
#define	SIZE	4
#define	QSIZE	5
#define	UUX	6

#define	UUQ	/* define this if you have BSD 4.3 uuq utility */
#define	DFABLE	/* define this if "df /usr/spool/uucp" works on your system */

/* Take a look at these, too. */
#ifdef	DEBUG
#define	BATCHCTRL	"batcher.ctrl"
#else
#define	BATCHCTRL	"/usr/lib/news/batcher.ctrl"
#endif

#define	BATCH	"/usr/spool/batch"
#define	SBATCH	"/usr/lib/news/batch"


int	fldlen[NUMFLDS] = {10, 2, 20, 10, 7, 10, 30};

struct desc {
	char 		*flds[NUMFLDS];
	struct desc	*next;
} *head = (struct desc *) NULL, 
  *dptr = (struct desc *) NULL;

#if	defined(BSD4_2) || defined(BSD4_1C)
#define	strchr	index
#endif

struct desc *getflds();

int	verbose = 0;
int	class;
int	spoolok = 1;

long	spoollim = 5000000;

/*
 *	main:
 *		- process arguments
 *		- read control file
 *		- for each system selected, see if there is any work,
 *		    if so, go try to do it.
 */

main(argc, argv)
int	argc;
char	**argv; {
	register char *p;
	register struct desc *curptr;
	argc--; argv++;
	for (;argc > 0 && **argv == '-'; argv++) {
		for (p = (*argv)+1; *p; p++)
			switch(*p) {
				case 'v':
					verbose = 1;
					break;
				case 'c':
					class = *(p+1);
					if (class == 0)
						class = 'z';
					else
						p++;
					break;
				default:
					fprintf(stderr, "batcher: Bad arg %s\n", *argv);
					exit(1);
			}
	}
	readctrl();
	if (!checkspool()) {
		exit(0);
	}
	if (verbose)
		dumpctrl();
	if (class) {
		for (curptr = head; curptr && spoolok; curptr = curptr->next) {
			if (*(curptr->flds[CLASS]) == class &&
				(chkbatch(curptr->flds[NAME], "") || 
				chkbatch(curptr->flds[NAME], ".work")) &&
				(spoolok = checkspool()))
				doit(curptr->flds[NAME]);
		}
	}
	while(*argv && spoolok) {
		if (chkbatch(*argv, "") || chkbatch(*argv, ".work")) {
			doit(*argv);
		}
		argv++;
	}
	exit(0);

}

/*
 *	readctrl:
 *
 *	Each line in the batch.ctrl file contains NUMFLDS colon-separated
 *	parameters.  This function reads each line, and calls getflds
 *	to separate out the parameters.  If getflds returns a system descriptor
 *	it is linked into the list of system descriptors.
 */

readctrl() {
	char	buf[BUFSIZ];
	register char	*p;
	register struct desc *ptr;
	struct desc *getflds();
	register FILE	*ctrl = fopen(BATCHCTRL, "r");
	if (!ctrl) {
		fprintf(stderr, "batcher: could not open %s file\n",
		    BATCHCTRL);
		exit(1);
	}
	while (fgets(buf, sizeof(buf), ctrl)) {
		if (buf[0] == '#')
			continue;
		p = buf + strlen(buf) - 1;
		if (*p == '\n')
			*p = '\0';
		if ((ptr = getflds(buf)) && processctrl(ptr)) {
			if (!head)
				head = dptr = ptr;
			else {
				dptr->next = ptr;
				dptr = ptr;
			}
			ptr->next = (struct desc *) NULL;
		}
	}
	fclose(ctrl);
}

/*
 *	dumpctrl:
 *
 *	If verbose is on, dump the tables
 */

dumpctrl() {
	register struct desc *p;
	register int i;
	for (p = head; p; p = p->next) {
		for (i = 0; i < NUMFLDS; i++)
			printf("%-*s", fldlen[i], p->flds[i]);
		printf("\n");
	}
}

char *strsave();

/*
 *	getflds:
 *
 *	This routine parses a single line from the batch.ctrl file,
 *	and if successfully parsed and checked out, returns a system
 *	descriptor pointer
 */

struct desc *
getflds(buf)
char	*buf; {
	register int cnt;
	char b2[100];
	char *curp, *p;
	int	len;
	struct desc *dptr;
	dptr = (struct desc *) malloc(sizeof (struct desc));
	if (!dptr) {
		fprintf(stderr, "batcher: Cannot malloc\n");
		exit(1);
	}
	curp = buf;
	for (cnt = 0; cnt < NUMFLDS; cnt++) {
		if (cnt == (NUMFLDS - 1)) {
			if (strchr(curp, ':')) {
				fprintf(stderr, "batcher: too many colons: %s\n",
					buf);
				free(dptr);
				return(NULL);
			}
			p = curp + strlen(curp);
		} else
			p = strchr(curp, ':');
		if (p == NULL) {
			fprintf(stderr, "batcher: invalid format (%d): %s\n", 
				cnt, buf);
			free(dptr);
			return(NULL);
		}
		len = p - curp;
		if (len >= fldlen[cnt]) {
			fprintf(stderr, "batcher: field %d too long: %s\n",
				cnt+1, buf);
			free(dptr);
			return(NULL);
		}
		if (!(dptr->flds[cnt] = malloc(len + 1))) {
			fprintf(stderr, "batcher: cannot malloc\n");
			exit(1);
		}
		strncpy(dptr->flds[cnt], curp, len);
		dptr->flds[cnt][len] = '\0';
		curp = p + 1;
	}
	return(dptr);
}

/*
 *	strsave:
 *	returns pointer to malloc'd copy of argument
 */
char *
strsave(s)
char	*s; {
	register char *p = malloc(strlen(s) + 1);
	if (!p) {
		fprintf(stderr, "batcher: cannot malloc\n");
		exit(1);
	}
	strcpy(p, s);
	return(p);
}

/*
 *	chkbatch:
 *
 *	return 1 if a batcher work file <batchdir>/<name><type> exists.
 */

chkbatch(name, type)
char	*name;
char	*type; {
	char batch[BUFSIZ];
	sprintf(batch, "%s/%s%s", BATCH, name, type);
	if (access(batch, 04) == 0)
		return(1);
	else
		return(0);
}

/*
 *	doit:
 *
 *	This routine is called with the name of the system that has
 *	been determined to have work for it.  The system is searched
 *	for in the system descriptors.  If found, a "system" line
 *	is contructed from the tables, and executed if system has not
 *	exceeded it's UUCP queue limit.
 */

doit(name)
char	*name; {
	char	cmdbuf[BUFSIZ];
	int	rc;
	long	queuesize;
	long	checkqueue(), checkspool();
	long	queuemax;
	register struct desc *ptr;
	if (verbose)
		printf("batcher: doing %s\n", name);
	for (ptr = head; ptr; ptr = ptr->next)
		if (!strcmp(ptr->flds[NAME], name)) {
			/*	form the command line for batching	*/
			sprintf(cmdbuf, "%s %s/%s %s",
				SBATCH, BATCH, name, ptr->flds[SIZE]);
			if (*(ptr->flds[COMPRESS]))
				sprintf(cmdbuf + strlen(cmdbuf), "|%s",
					ptr->flds[COMPRESS]);
			/*	Find the queue limit	*/
			sprintf(cmdbuf + strlen(cmdbuf), "|%s", ptr->flds[UUX]);
			if (1 != sscanf(ptr->flds[QSIZE], "%ld", &queuemax)) {
				fprintf(stderr, "batcher: bad qmax field: %s\n",
					ptr->flds[QSIZE]);
				return;
			}
			queuesize = checkqueue(ptr->flds[NAME]);
			rc = 0;
			/*	While we haven't exceeded the queue limit &
				there's work to do, issue the command */
			while (queuesize < queuemax && !rc && 
				(chkbatch(ptr->flds[NAME], "") || 
				   chkbatch(ptr->flds[NAME], ".work")) &&
				   (spoolok = checkspool())) {
#ifdef	DEBUG
				printf("batcher: cmd: %s\n", cmdbuf);
				rc = 1;
#else
				rc = system(cmdbuf);
				queuesize = checkqueue(ptr->flds[NAME]);
#endif
			}
			return;
		}
	fprintf(stderr, "batcher: no control line for %s\n", name);
}

/*
 *	processctrl:
 *
 *	Check validity of batch.ctrl entries and supply defaults.
 */
processctrl(ptr)
struct	desc *ptr; {
	char	buf[100];
	register char	*p, *uuxflags;
	if (!ptr) return;
	if (strlen(ptr->flds[NAME]) == 0) {
		fprintf(stderr, "batcher: null system name\n");
		free(ptr);
		return(0);
	}

	if (strlen(ptr->flds[QSIZE]) == 0) {
		free(ptr->flds[QSIZE]);
		ptr->flds[QSIZE] = strsave("500000");
	}

	if (strlen(ptr->flds[CLASS]) == 0) {
		free(ptr->flds[CLASS]);
		ptr->flds[CLASS] = strsave("z");
	}

	if (strlen(ptr->flds[SIZE]) == 0) {
		free(ptr->flds[SIZE]);
		ptr->flds[SIZE] = strsave("100000");
	}

	if (strlen(ptr->flds[UUXFLAGS]) == 0) {
		free(ptr->flds[UUXFLAGS]);
		ptr->flds[UUXFLAGS] = strsave("-r -z -gd");
	}

	if (strlen(ptr->flds[UUX]) == 0) {
		sprintf(buf, "uux - %s %s!%s", ptr->flds[UUXFLAGS],
			ptr->flds[NAME], 
			*ptr->flds[COMPRESS] ? "cunbatch" : "rnews");
		ptr->flds[UUX] = strsave(buf);
	}
	return(1);
}

/*
 *	checkqueue:
 *
 *	Logically, all this code does is return the size of the UUCP queue
 *	for system "name".
 *	I've taken the easy way out and popen'd "uuq" (4.3 BSD UUCP utility)
 *	to parse the first line, which looks something like this:
 *
 *	<systemname>: <n> jobs, <nnnn> bytes, ....
 *
 *	I merely look for the first comma, and sscanf the number following.
 *	A proper solution would be to dive in and parse the UUCP queues
 *	themselves, but: it's moderately difficult, and it changes from 
 *	system to system.
 */

long
checkqueue(name)
char	*name; {
#ifdef	UUQ
	char buf[BUFSIZ];
	long retval;
	register char *p2;
	FILE *p, *popen();
	/* Gross, but the easiest way */
	sprintf(buf, "uuq -l -s%s", name);
	p = popen(buf, "r");
	if (!fgets(buf, sizeof(buf), p)) {
		return(0);
	}
	pclose(p);
	p2 = strchr(buf, ',');
	if (p2 && 1 == sscanf(p2+1, "%ld", &retval)) {
		return(retval);
	}
	fprintf(stderr, "batcher: could not interpret %s\n", buf);
	return(10000000);
#else
	return(10000000);
#endif
}

/*	This function returns the amount of free space on the spool
	device, this may not work on your system - it reads the
	second line from a "df /usr/spool/uucp" */
#define	SPOOL "/usr/spool/uucp"

checkspool() {
#ifdef	DFABLE
	char buf[100];
	FILE *p, *popen();
	long val;
	sprintf(buf, "df %s", SPOOL);
	if (!(p = popen(buf, "r"))) {
		fprintf(stderr, "batcher: couldn't popen %s\n", buf);
		return(0);
	}
	if (!fgets(buf, sizeof(buf), p)) {
		fprintf(stderr, "batcher: no first line in df\n");
		return(0);
	}
	if (!fgets(buf, sizeof(buf), p)) {
		fprintf(stderr, "batcher: no second line in df\n");
		return(0);
	}
	if (1 != sscanf(buf, "%*s %*ld %*ld %ld", &val)) {
		fprintf(stderr, "batcher: couldn't get size from %s\n", buf);
		return(0);
	}
	if (pclose(p)) {
		fprintf(stderr, "batcher: DF failed\n");
		return(0);
	}
	val *= 1024;
	if (val < spoollim) {
		printf("batcher: spool limit exceeded, %ld bytes left\n", val);
		return(0);
	} else
		return(1);
#else
	return(1);
#endif
}
