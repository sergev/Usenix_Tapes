
/*
**  vol.c - create volume header files for `tar'
**
**  I hereby release this source code into the public domain.  Anyone
**  may use, copy, or modify this source code, as long as:
**
**	1) it is not sold for profit;
**	2) modifications are clearly identified with the date,
**         author's name, and e-mail address; and
**	3) the entire text of this comment remains intact.
**
**  I would also appreciate it if ports, modifications and/or flames
**  were sent to me at the address below.
**
**  Brian Yost    {topaz,clyde}!infopro!bty!yost    14 June 1986
*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

#define OKAY		0
#define ERROR		1
#define	TOLERANCE	10	/* how close to actually come to limit */
#define MAX_HEADER	5000L	/* maximum size of header file (bytes) */
#define UNDER_10	"./Vol_%dof%d"
#define OVER_9		"./Vol_%02dof%02d"

#define SIGNAL(S,A)	if (signal((S),SIG_IGN) != SIG_IGN) signal((S),(A))

typedef enum {FALSE, TRUE} boolean;

char *ProgName;
char *Default = "/etc/default/vol";
char *Usage = "Usage: %s [-ptd] [-s maxblocks] pathname...\n";
char *Size = "VOLSIZE";

boolean Preserve = FALSE;	/* ok to rearrange file sequence */

FILE *TempFile;
char *TempName = "/tmp/volXXXXXX";

long MaxBlocks = 0L;
long N_Records = 0L;

extern int CreateVol();	    /* top level */
extern void GetSize();	    /* determines size of a pathname */
extern void ProcessDir();   /* run GetSize() on directory contents */
extern void AddList();	    /* adds to the list of processed pathnames */
extern char *GetDate();	    /* returns today's date in MM/DD/YY format */
extern int rm_temps();	    /* interrupt handler */

main(ac, av)
int ac;
char *av[];
{
	int status, option;
	FILE *deflt;
	extern char *optarg;
	extern int optind;
	extern long atol();
	extern char *getenv(), *fgets();

	ProgName = av[0];
	while ((option = getopt(ac, av, "tds:ph")) != EOF)
		switch (option) {
		case 't':	/* tape */
			Size = "TAPESIZE";
			break;
		case 'd':	/* disk */
			Size = "DISKSIZE";
			break;
		case 's':	/* size */
			MaxBlocks = atol(optarg);
			break;
		case 'p':	/* preserve file sequence */
			Preserve = TRUE;
			break;
		case 'h':	/* help */
		case '?':	/* error */
		default:
			fprintf(stderr, Usage, ProgName);
			exit(ERROR);
			break;
		}
	
	if (ac - optind < 1) {	/* not enough args */
		fprintf(stderr, Usage, ProgName);
		exit(ERROR);
	}

	/* Look for environment variable */
	if (!MaxBlocks) {
		char *s;

		if ((s = getenv(Size)) != NULL)
			MaxBlocks = atol(s);
	}
	
	/* Check DEFAULT file */
	if (!MaxBlocks)
		if ((deflt = fopen(Default, "r")) != NULL) {
			int len = strlen(Size);
			char buf[BUFSIZ];

			while (fgets(buf, BUFSIZ, deflt))
			  if (strncmp(buf, Size, len) == 0)
			    if (buf[len] == '=')
			      if (MaxBlocks = atol(buf+len+1))
			        break;
			fclose(deflt);
		}
	

	if (!MaxBlocks) {	/* just give up */
		fprintf(stderr, "%s: you must specify block size\n", ProgName);
		fprintf(stderr, Usage, ProgName);
		exit(ERROR);
	} else if (MaxBlocks < 0L) {	/* no way */
		fprintf(stderr, "%s: invalid block size %ld\n",
			ProgName, MaxBlocks);
		exit(ERROR);
	}
	printf("Volume size = %ld blocks\n", MaxBlocks);

	SIGNAL(SIGHUP, rm_temps);
	SIGNAL(SIGINT, rm_temps);
	SIGNAL(SIGQUIT, rm_temps);
	SIGNAL(SIGTERM, rm_temps);

	mktemp(TempName);
	if ((TempFile = fopen(TempName, "w+")) == NULL) {
		fprintf(stderr, "%s: can't open %s\n", ProgName, TempName);
		exit(ERROR);
	} else {
		setbuf(TempFile, (char*)NULL);
		status = CreateVol(ac-optind, av+optind);
		fclose(TempFile);
		exit(status);
	}
}

int
CreateVol(n, pathname)
int n;
char *pathname[];
{
	int i, c, volume;
	long total, grandtotal, blocks, filepos, filesize;
	char volfile[BUFSIZ], oldfile[BUFSIZ], name[BUFSIZ], buf[BUFSIZ];
	FILE *vol, *old;
	extern long ftell();

	for (i = 0; i < n; i++)
		GetSize(pathname[i]);
	
	if (Preserve != TRUE) {
		/*
		** Since we only make one pass through the list, sort
		** it by blocksize descending.  This should get us as
		** close to MaxBlocks as possible.
		*/
		sprintf(buf, "sort -n -r %s -o %s", TempName, TempName);
		if (system(buf) != 0)
			fprintf(stderr, "%s: error sorting %s\n",
				ProgName, TempName);
	}
	unlink(TempName);	/* (after last close) */

	/*
	** Move items from the list into the volume header files
	*/
	volume = 0;
	grandtotal = 0L;
	while (N_Records) {
		volume++;
		total = 0L;

		sprintf(volfile, "%s-%d", TempName, volume);
		if ((vol = fopen(volfile, "w")) == NULL) {
			fprintf(stderr, "%s: fatal error -- can't open %s\n",
				ProgName, volfile);
			return ERROR;
		}

		rewind(TempFile);
		filepos = ftell(TempFile);
		filesize = 0L;
		while (fgets(buf, BUFSIZ, TempFile)) {
			if (sscanf(buf, "%ld %s", &blocks, name) != 2) {
				fprintf(stderr, "%s: error reading %s\n",
					ProgName, TempName);
				return ERROR;
			}

			if (blocks >= 0L)	/* hasn't been deleted */
			    if ((total + blocks) <= (MaxBlocks - TOLERANCE)
				&& (filesize + strlen(name)+1) < MAX_HEADER) {
				   total += blocks;
				   filesize += strlen(name) + 1;
				   fprintf(vol, "%s\n", name);
   
				   /* delete from tempfile */
				   fseek(TempFile, filepos, 0);
				   fprintf(TempFile, " -1    ");
				   fgets(buf, BUFSIZ, TempFile);
				   N_Records--;
			    } else if (Preserve) /* can't rearrange files */
				   break;
			
			filepos = ftell(TempFile);
		}

		fprintf(vol, "TOTAL-%ld-blocks-%s\n", total, GetDate());
		fclose(vol);
		printf("Volume %d: %ld blocks\n", volume, total);
		grandtotal += total;
	}

	/*
	** Now we know how many were needed altogether, so
	** we can create the proper volume header file names.
	*/
	for (i = 1; i <= volume; i++) {
		sprintf(oldfile, "%s-%d", TempName, i);
		if (volume < 10)
			sprintf(volfile, UNDER_10, i, volume);
		else
			sprintf(volfile, OVER_9, i, volume);
		
		if ((vol = fopen(volfile, "w")) == NULL) {
			fprintf(stderr, "%s: fatal error -- can't open %s\n",
				ProgName, volfile);
			return ERROR;
		} else if ((old = fopen(oldfile, "r")) == NULL) {
			fprintf(stderr, "%s: fatal error -- can't open %s\n",
				ProgName, volfile);
			return ERROR;
		}
		
		fprintf(vol, "%s\n", volfile);
		while ((c = getc(old)) != EOF)
			putc(c, vol);
		
		fclose(old);
		fclose(vol);
		unlink(oldfile);
	}

	printf("Total %ld blocks in %d volume%c\n",
		grandtotal, volume, (volume == 1? ' ' : 's'));

	return OKAY;
}

void
GetSize(pathname)
char *pathname;
{
	struct stat stat_buf;
	char du_pipe[BUFSIZ], buf[BUFSIZ];
	FILE *du, *popen();

	if (stat(pathname, &stat_buf) != 0) {
		fprintf(stderr, "%s: can't stat %s\n",
			ProgName, pathname);
		return;
	} else if ((stat_buf.st_mode & S_IFDIR) == 0
		   && (stat_buf.st_mode & S_IFREG) == 0) {
		fprintf(stderr, "%s: %s not a file or directory\n",
			ProgName, pathname);
		return;
	}

	sprintf(du_pipe, "du -s %s", pathname);
	if ((du = popen(du_pipe, "r")) == NULL)
		fprintf(stderr, "%s: can't popen du\n", ProgName);
	else {
		if (fgets(buf, BUFSIZ, du)) {
			pclose(du);	/* before recursion */
			if (atol(buf) > (MaxBlocks - TOLERANCE))
				if (stat_buf.st_mode & S_IFDIR)
					ProcessDir(pathname);
				else
					fprintf(stderr, "%s: %s too big\n", 
						ProgName, pathname);
			else
				AddList(buf);
		} else {
			pclose(du);
			fprintf(stderr, "%s: du not cooperating\n",
				ProgName);
		}
	}

	return;
}

void
ProcessDir(pathname)
char *pathname;
{
	FILE *dir;
	char buf[BUFSIZ];
	struct direct dir_buf;

	if ((dir = fopen(pathname, "r")) == NULL)
		fprintf(stderr, "%s: can't read directory %s\n",
			ProgName, pathname);
	else {
		while (fread((char*)&dir_buf, sizeof(dir_buf), 1, dir) == 1)
			if (dir_buf.d_ino
			    && strcmp(dir_buf.d_name, ".") != 0
			    && strcmp(dir_buf.d_name, "..") != 0) {
				if (pathname[strlen(pathname)-1] == '/')
					sprintf(buf, "%s%.14s",
						pathname, dir_buf.d_name);
				else
					sprintf(buf, "%s/%.14s",
						pathname, dir_buf.d_name);
				GetSize(buf);
			}
		fclose(dir);
	}
	return;
}

void
AddList(buf)
char *buf;
{
	long blocks;
	char pathname[BUFSIZ];

	if (sscanf(buf, "%ld %s", &blocks, pathname) != 2)
		fprintf(stderr, "%s: unexpected data from du - \"%s\"\n",
			ProgName, buf);
	else if (blocks >= 0) {
		fprintf(TempFile, "%07ld %s\n", blocks, pathname);
		N_Records++;
	}

	return;
}

char *
GetDate()
{
	long now, time();
	struct tm *tm, *localtime();
	static char date[9];

	if (!(*date)) {
		now = time((long *)0);
		tm = localtime(&now);
		sprintf(date, "%02d/%02d/%02d",
			tm->tm_mon+1, tm->tm_mday, tm->tm_year);
	}
	return date;
}


int
rm_temps(signum)
int signum;
{
	char buf[BUFSIZ];

	signal(signum, SIG_IGN);
	sprintf(buf, "rm -f %s*", TempName);
	system(buf);

	switch (signum) {
	case SIGHUP:
		fprintf(stderr, "%s: line gone\n", ProgName);
		exit(ERROR);
		break;
	case SIGINT:
		fprintf(stderr, "%s: terminated by user\n", ProgName);
		exit(ERROR);
		break;
	case SIGQUIT:
		signal(SIGQUIT, SIG_DFL);
		kill(getpid(), SIGQUIT);
		break;
	case SIGTERM:
		fprintf(stderr, "%s: killed\n", ProgName);
		exit(ERROR);
		break;
	default:
		return;
	}
}
