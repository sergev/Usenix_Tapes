

*/

char *msg[] = {
   "Usage : %s [-Dgdumhv] [userlist]\n",
   "-D : Don't print public dirs (because it can be slow)\n",
   "-d : just print public dirs\n",
   "-g : just print group(s)\n",
   "-u : just print user's id\n",
   "-m : just print if they have mail\n",
   "-h : just print the home dir\n",
   "-v : visual mode. Prints printable versions of ctrl chars\n",
   "-M : Don't do \"more\". Automatically off if any redirects.\n",
   "\nTo make a toefile, create a file called ~/.toerc. If this program\n",
   "is NOT setuid to root, chmod 644 .toerc.\n",
   0 };

#define utmpfile  "/etc/utmp"
#define maildir   "/usr/spool/mail"

#include <stdio.h>
#include <ctype.h>
#include <utmp.h>
#include <sgtty.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#if BSD
#include <sys/dir.h>
#else
#include <ndir.h>
#endif

/*  convenience  */
#define when break;case
#define otherwise break;default

int visual = 0, no_pub = 0, chk_home = 0, chk_grp = 0, chk_pub = 0,
    chk_uid = 0, chk_mail = 0, do_all = 1, do_more = 1;

char *unctrl_chars[] = {
    "^@", "^A", "^B", "^C", "^D", "^E", "^F", "^G", "^H", "^I", "^J", "^K",
    "^L", "^M", "^N", "^O", "^P", "^Q", "^R", "^S", "^T", "^U", "^V", "^W",
    "^X", "^Y", "^Z", "^[", "^\\", "^]", "^~", "^_", " ", "!", "\"", "#",
    "$",  "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", "0",  "1",
    "2",  "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">",  "?",
    "@",  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",  "M",
    "N",  "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",  "[",
    "\\", "]", "^", "_", "`", "a", "b", "c", "d", "e", "f", "g", "h",  "i",
    "j",  "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v",  "w",
    "x",  "y", "z", "{", "|", "}", "~", "^?"
};
#define unctrl(c)	unctrl_chars[(c) & 0177]

FILE *toefile;
long now;

struct passwd *entry;
struct Users {
    char *u_login;
    struct Users *u_next;
} *users = NULL;

main(argc, argv)
int argc;
char **argv;
{
    char *cmd = *argv;
    struct Users **list;
    int die();
    if (!isatty(0) || !isatty(1) || !isatty(2)) /* check for redirection */
	 do_more = 0;
    savetty(), noecho(), crmode();
    signal(SIGINT, die);
    signal(SIGQUIT, die);
    list = &users;
    while (*++argv)
	if (argv[0][0] == '-')
	    while(*(++argv[0]))
		switch(*argv[0]) {
		    when 'D' :  no_pub = 1;
		    when 'g' :  do_all = 0, chk_grp = 1;
		    when 'd' :  do_all = 0, chk_pub = 1;
		    when 'u' :  do_all = 0, chk_uid = 1;
		    when 'm' :  do_all = 0, chk_mail = 1;
		    when 'v' :  do_all = 0, visual = 1;
		    when 'h' :  do_all = 0, chk_home = 1;
		    when 'M' :  do_more = 0;
		    otherwise  :
			do_error(cmd);
		}
	else { /* create a linkd list of people to process */
	    *list = (struct Users *)malloc(sizeof(struct Users));
	    (*list)->u_login = argv[0];
	    list = &(*list)->u_next;
	}

    *list = NULL; /* terminate the list */
    if (!users) getusers();  /* do everyone logged in */
    do  if (!(entry = getpwnam(users->u_login)))
	    fprintf(stderr,"%s: No one by that name here.\n", users->u_login);
	else do_toe(entry);
    while(users = users->u_next);
    die();
}

do_error(arg)  /* user messed up command line */
{
    int count = 0;
    while(msg[count])
	printf(msg[count++], arg);
    die();
}

do_toe(entry)
struct passwd *entry;
{
    static int counter = 0;
    char c, file[50], grp[30], alias[1024];
    FILE *fp;
    if (counter++) {
	char c;
	printf("\n--next user--('q' to exit)");
	while((c = getchar()) != ' ' && c != '\n' && c != 'q');
	printf("\015                          \015");
	if (c == 'q') die();
	putchar('\n');
    }
    if (do_all) {
	sprintf(file, "%s/.toerc", (entry->pw_uid) ? entry->pw_dir : "");
	if (fp = fopen(file, "r"))
	    Fgets(alias, 1024, fp);
	else alias[0] = 0;
	printf("Login name: %-8s\tLogout name: %s\n",
			entry->pw_name, (fp) ? alias : "");
    }
    if (do_all || chk_mail) {
	if (mail(entry->pw_name))
	    printf("%s has mail.\t\t\t", entry->pw_name);
	else if (chk_mail) printf("%s has no mail", entry->pw_name);
	if (!do_all) putchar('\n');
    }
    if (do_all || chk_uid) printf("User Id: %d\n", entry->pw_uid);
    if (do_all || chk_home)
	printf("Where %s lives: %s\n", entry->pw_name, entry->pw_dir);
    if (do_all || chk_grp) {
	strcpy(grp, getgrgid(entry->pw_gid)->gr_name);
	printf("Login Group: %s (%d)\n", grp, entry->pw_gid);
	get_groups(entry, grp);
    }
    if ((do_all && !no_pub) || chk_pub) {
	printf("Public directories:\n");
	if (!enterdir(entry->pw_dir)) printf("\tHome directory not public\n");
    }
    if (do_all)
	if (fp) {
	    int lines = 0;
	    printf("Some extra stuff:\n");
	    while((c = getc(fp)) != EOF) {
		if (visual && !isspace(c))
		     printf("%s", unctrl(c));
		else putchar(c);
		if (do_more && c == '\n' && ++lines == 22) {
		    lines = 0;
		    printf("--more--");
		    while((c = getchar()) != ' ' && c != '\n' && c != 'n' &&
			   c != 'q');
		    printf("\015        \015");
		    if (c == 'n' || c == 'q') return;
		    if (c == '\n') lines = 21;
		}
	    }
	} else
	    printf("No toes.\n");
}

Fgets(string, LEN, fp)
char string[];
FILE *fp;
int LEN;
{
    int count = 0;
    char c, *temp = string;
    *temp = 0;
    while((c = getc(fp)) != EOF && c != '\n' && count < LEN)
	if (visual && !isspace(c))
	    strcat(temp, unctrl(c)), count += 1 + (iscntrl(c));
	else *temp++ = c, count++;
    *temp = 0;
}

mail(user)
char user[];
{
    struct stat mbox;
    char mail_path[40];
    sprintf(mail_path,"%s/%s", maildir, user);
    if (stat(mail_path, &mbox) != -1)
	if (mbox.st_size > 0) return 1;  /* has mail */
    return 0;  /* no mail */
}

enterdir(pathsofar)
char pathsofar[];
{
    FILE *fp;
    struct direct *dp;
#if BSD
    DIR *dirp;
#else
    struct DIR *dirp;
#endif
    struct stat stat_buf;
    char newpath[80];
    stat(pathsofar, &stat_buf);
    if (stat_buf.st_mode & S_IFDIR && (stat_buf.st_mode & 05) == 05)
	printf("\t%s\n", pathsofar + (pathsofar[1] == '/'));
    else return 0;
    if ((dirp = opendir(pathsofar)) == NULL)
	return -1;
    for (dp = readdir(dirp); dp; dp = readdir(dirp))
	if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
	    sprintf(newpath,"%s/%s",pathsofar,dp->d_name), enterdir(newpath);
    closedir(dirp);
    return 1;
}

getusers()
{
    struct utmp buf;
    struct Users **temp;
    FILE *fptr;

    temp = &users;
    if ((fptr = fopen(utmpfile, "r")) < 0)
        perror(utmpfile), exit(1);
    while (fread(&buf , sizeof (struct utmp), 1, fptr) > 0)
        if ( buf.ut_name[0] ) {
	    *temp = (struct Users *) malloc (sizeof(struct Users));
	    (*temp)->u_login = (char *) malloc(strlen(buf.ut_name) + 1);
	    strcpy((*temp)->u_login, buf.ut_name);
	    temp = &(*temp)->u_next;
	}
    close(fptr);
}

get_groups(entry, cur_group)
struct passwd *entry;
char cur_group[];
{
    short first_find = 0;
    struct group *Group;
    while(Group = getgrent())
	while(*(Group->gr_mem)) {
	    if (!strcmp(*Group->gr_mem, entry->pw_name) &&
				  strcmp(Group->gr_name, cur_group)) {
		if (!first_find++) printf("Other groups:\n");
		printf("\t%s (%d)\n", Group->gr_name, Group->gr_gid);
	    }
	    *Group->gr_mem++;
	}
    endgrent();
}

die()
{
    putchar('\n');
    resetty(); echo(); nocrmode(); exit(0);
}

/* stuff for tty attributes */

#define    _dosave_   if (!_savflgs) savetty()

struct sgttyb _tty;
int _savflgs = 0;
savetty()  { if (!gtty(0, &_tty)) _savflgs = _tty.sg_flags; }
resetty()  { _tty.sg_flags = _savflgs;   stty(0, &_tty);   }
crmode()   { _dosave_; _tty.sg_flags |= CBREAK;  stty(0, &_tty); }
nocrmode() { _dosave_; _tty.sg_flags &= ~CBREAK; stty(0, &_tty); }
echo()	   { _dosave_; _tty.sg_flags |= ECHO;    stty(0, &_tty); }
noecho()   { _dosave_; _tty.sg_flags &= ~ECHO;   stty(0, &_tty); }
