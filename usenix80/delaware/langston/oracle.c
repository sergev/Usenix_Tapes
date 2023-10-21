#include    <stdio.h>
/*
**      ORACLE -- ho ho         Copyright (c) P. S. Langston - NYC NY  1978
** Compile: cc -O -q oracle.c -lS; mv a.out oracle; chmod 755 oracle
*/

char    *whatsccs "@(#)oracle.c	1.7  last mod 6/13/80 -- (c) psl 1978";

#define TOPROG      "/bin/to"
#define INDEXFILE   "/sys/games/.../oracleindex"
#define LOCKFILE    "/usr/tmp/oraclock"

#define TEXTLEN 114         /* max size for questions & answers */

#define PRIVUID 25

struct  questr {
	char    q_askuid;           /* who asked */
	char    q_asknam[9];        /* logname of asker */
	char    q_quest[TEXTLEN];   /* question */
	long    q_askdat;           /* when asked */
	char    q_ansuid;           /* who answered */
	char    q_ansnam[9];        /* logname of answerer*/
	char    q_answer[TEXTLEN];  /* answer */
	long    q_ansdat;           /* when answered */
};

char    *hmm[] {
	"Hmmmmm... ",
	"Well, let me see ... ",
	"Sometimes I wonder if people are taking me seriously.  In any case\n",
	"Boy!  That's a toughie!  ",
	"Enlightenment shall not come easily on this matter.\n",
	"Golly, Gee Whizzikers!  ",
	"That should be obvious; but in case there's some hidden subtlety involved\n",
	"My oh my, I guess ",
	"I hope you really need to know the answer to that one ...\n",
};

int     locked, uid;

char    *uidnam();
int     bye();
struct  questr *findq();

main()
{
	char buf[TEXTLEN];
	struct questr *qp;

	uid = getuid() & 0377;
	time(buf);
	srand(buf[2]);
	printf("Welcome to Ask-the-Oracle ...\n\n");
	printf("I am able to answer short questions of a general nature.\n");
	printf("I'm not too good on personal problems (although I'll try)\n");
	for (;;) {
	    questn("Do you have a question that you need answered? ",
	     buf, TEXTLEN);
	    if (buf[0] == 'y')
		break;
	    else if (buf[0] == 'n') {
		printf("When, (and if), you do, let me know\n");
		exit(1);
	    } else if (buf[0] == 's' && buf[1] == 'n' && uid == PRIVUID) {
		snoop(buf[2]);
		exit(0);
	    } else if (buf[0] == 'a' && buf[1] == 'n' && uid == PRIVUID)
		goto ask;
	    else
		printf("Please answer with \"yes\" or \"no\"\n");
	}
	for (;;) {
	    questn("\nWhat is your question? (one line only)\n", buf, 128);
	    if (issent(buf))
		break;
	    printf("I don't understand that; please try re-phrasing it\n");
	}
	newquest(buf);
	printf("%s", hmm[rand() % 9]);
	printf("I'll think that over and mail you an answer.\n");
ask:
	if ((qp = findq()) == 0)
	    exit(0);
	printf("Meanwhile, perhaps you could answer this question for me:\n");
	answer(qp);
}

answer(qp)
struct  questr *qp;
{
	char buf[TEXTLEN];

	for (;;) {
	    printf("%s", qp->q_quest);
	    questn(" (one line only)\n", buf, TEXTLEN);
	    if (issent(buf))
		break;
	    printf("Please answer the question in more detail\n");
	}
	printf("Thank you\n");
	oldquest(qp, buf);
}

issent(buf)
char    *buf;
{
	register char *cp, c;
	int wds, vow;

	wds = vow = 0;
	for (cp = buf; c = *cp; cp++) {
	    c =& ~040;
	    if (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U')
		vow++;
	    else if (c < '@') {
		if (vow && ++wds > 3)
		    return(1);
		vow = 0;
	    }
	}
	return(0);
}

newquest(quest)
char    *quest;
{
	struct questr q;
	FILE *ifp;


	if ((ifp = fopen(INDEXFILE, "a")) == NULL) {
	    fprintf(stderr, "Can't open %s for append\n", INDEXFILE);
	    exit(3);
	}
	q.q_askuid = uid;
	q.q_ansuid = -1;
	strcpy(q.q_asknam, uidnam(uid));
	lock();
	strcpy(q.q_quest, quest);
	time(&(q.q_askdat));
	fwrite(&q, sizeof q, 1, ifp);
	fclose(ifp);
	unlock();
}

struct  questr *
findq()
{
	register int n;
	FILE *ifp;
	static struct questr q;

	if ((ifp = fopen(INDEXFILE, "r")) == NULL) {
	    fprintf(stderr, "Can't open %s for read\n", INDEXFILE);
	    exit(3);
	}
	for (n = 0; fread(&q, sizeof q, 1, ifp) > 0; n++) {
	    if (q.q_ansuid == -1 && q.q_askuid != uid) {
		fclose(ifp);
		q.q_ansuid = n;
		return(&q);
	    }
	}
	fclose(ifp);
	return(0);
}

snoop(arg)
char    arg;
{
	char buf[10];
	struct questr q;
	FILE *ifp;

	if ((ifp = fopen(INDEXFILE, "r")) == NULL) {
	    fprintf(stderr, "Can't open %s for read\n", INDEXFILE);
	    exit(3);
	}
	while (fread(&q, sizeof q, 1, ifp) > 0) {
	    if (arg != 'o' && q.q_ansuid != -1)
		continue;
	    printf("\nAsked by %s (%d) %s",
	     q.q_asknam, q.q_askuid & 0377, ctime(&q.q_askdat));
	    printf(" %s\n", q.q_quest);
	    if (q.q_ansuid == -1) {
		questn("No answer; do you wish to answer? ", buf, 10);
		if (buf[0] == 'y')
		    answer(&q);
	    } else {
		printf("Answered by %s (%d) %s",
		 q.q_ansnam, q.q_ansuid, ctime(&q.q_ansdat));
		printf(" %s\n", q.q_answer);
	    }
	}
	fclose(ifp);
}

oldquest(qp, answer)
struct  questr *qp;
char    *answer;
{
	register int n, fh;
	char buf[128];
	long addr;

	if ((fh = open(INDEXFILE, 2)) < 0) {
	    fprintf(stderr, "Can't open %s for writing\n", INDEXFILE);
	    exit(3);
	}
	n = qp->q_ansuid;
	addr = n;
	addr =* sizeof *qp;
	lock();
	qp->q_ansuid = uid;
	strcpy(qp->q_ansnam, uidnam(uid));
	strcpy(qp->q_answer, answer);
	time(&(qp->q_ansdat));
	if (sendans(qp) == -1) {
	    fprintf(stderr, "I don't think that's right...\n");
	    unlock();
	    exit(1);
	}
	n = addr / 512;
	seek(fh, n, 3);
	n = addr % 512;
	seek(fh, n, 1);
	write(fh, qp, sizeof (*qp));
	close(fh);
	unlock();
}

/* note: this routine expects the I.S.C. `to' prog for mail; if you have
**      some other mail system you'll probably have to rewrite it.
*/
sendans(qp)       /* (should eventually write a file for the "anondaemon") */
struct  questr *qp;
{
	char buf[512], fname[64];

	switch (fork()) {
	case -1:
	    fprintf(stderr, "Unable to fork()\n");
	    return(-1);
	case 0:
	    sprintf(buf, "-TRemember when you asked \"%s\"\n\n", qp->q_quest);
	    strcat(buf, "Well, the answer is ...\n\n");
	    strcat(buf, qp->q_answer);
	    close(0);                /* because the "to" program has a few */
	    close(1);            /* "features" that haven't been fixed yet */
	    execl(TOPROG, "to", qp->q_asknam, "-SAnswer from the oracle!",
	     buf, 0);
	    fprintf(stderr, "Can't execl %s\n", TOPROG);
	    exit(3);
	}
	return(0);
}

lock()
{
	register int i;

	while (link("/etc/locknode", LOCKFILE) == -1)
	    sleep(3);
	locked++;
	for (i = 1; i < 16; i++)
	    signal(i, &bye);
}

unlock()
{
	register int i;

	if (locked) {
	    unlink(LOCKFILE);
	    locked = 0;
	} else
	    fprintf("Ooops!  Extra unlock.\n");
	for (i = 1; i < 16; i++)
	    signal(i, 0);
}

bye()
{
	if (locked)
	    unlock();
	exit(1);
}

char *
uidnam(uid)
{
	register char *cp;
	static char buf[128];

	getpw(0377 & uid, buf);
	for (cp = buf; *cp++ != ':'; );
	*--cp = '\0';
	return(buf);
}

questn(quest, resp, max)
char    *quest, *resp;
{
	register char *cp;

	printf(quest);
	fgets(resp, max, stdin);
	for (cp = resp; *cp != '\n'; cp++);
	*cp = '\0';
}
