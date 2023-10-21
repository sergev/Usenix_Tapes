/*
 * savenews filename [filename ...]
 *
 * Savenews is a program designed to clean up and compact a
 * usenet archive. It will take the filename(s) given to it as arguments
 * and save them in a netnews archive (defined by SAVENEWS, default is
 * /usr/spool/savenews).
 *
 * This program was set up to do two main things:
 *
 * 1) compact out the useless parts of the message, specifically the lines
 *    in the header that don't serve a useful purpose in an archive. This 
 *    is done by removing all but the following header lines: From, Date,
 *    Newsgroups, Subject, and Message-ID, and seems to save an average of
 *    500 bytes an article.
 *
 * 2) keep the quadratic nature of unix(TM AT&T Bell labs) directory searches
 *    from making your life miserable. Storing a raw archive of
 *    net.unix-wizards is a silly thing to do, for example. What I do is
 *    create a one level subdirectory set to keep any one directory from
 *    getting too large, but this program is currently set so that there
 *    are enough directories to keep the total number of files in any one
 *    directory below about 150 in the largest parts of my archive. The
 *    algorithm I use is abs(atoi(Message-ID)%HASHVAL)) with HASHVAL being
 *    prime. This quick and dirty hash gives you directories with the
 *    numbers 0 to HASHVAL-1, and about the same number of files in each
 *    given a random distribution of Message-ID numbers (not bad, in
 *    reality)
 *
 * The program will add the name of the file and the subject line of the
 * article in a logfile in subdirectory LOGS, the filename being the 
 * newsgroup.
 *
 * As currently written, an article will be saved only to the first 
 * newsgroup in the Newsgroups header line. This means that something
 * posted to 'net.source,net.flame' will end up in net.sources, but that
 * somethine posted to 'net.flame,net.sources' will end up in net.flame.
 * I consider this a feature. Others may disagree.
 *
 * If an article is saved that has a duplicate message-ID of one already
 * in the archive, then it will be saved by adding the character '_' and
 * some small integer needed to make the filename unique. You can then
 * use ls or find to look for these and see if they are duplicates (and
 * remove them) or if they are simply botches by some other site (it does
 * happen, unfortunately).
 *
 * This program will do intelligent things if given a non-news article,
 * such as nothing. Don't push it, though -- I haven't tried it on
 * special devices, symbolic links, and other wierdies and it is likely
 * to throw up on some of them since I didn`t feel like protecting someone
 * from trying to archive /dev (if tar can consider this a feature, so can
 * I...)
 *
 * This program uses the 4.2 Directory routines (libndir). If you don't
 * run 4.2, get ahold of a copy of the compatibility library for your
 * system and use it, or hack up do_dir and is_dir to get around it
 * if you believe in messing around with primitive hacks (I LIKE libndir)
 *
 * General usage: every so often run the program with 
 * 'savenews /usr/spool/oldnews'. Look through /usr/spool/savenews
 * for duplicated articles and remove them, and then copy all of the
 * stuff to tape. Remove everything except the LOGS directory, so that
 * people can use grep to look for things in the archive. It should be
 * easy to get things back off of tape and make the archive useful this
 * way. Thinking about it, if you can't use the archive, you might as well
 * not have it, which is why this program got written (I needed something
 * out of my archive, and it took me a week to find it).
 *
 * This program is designed to run under 2.10.2, but should work under any
 * B news system. Anyone else is on their own. This is in
 * the public domain by the kindness of my employer, national
 * semiconductor, but neither I nor national make any guarantee that it
 * will work, that we will support this program, or even admit that it
 * exists. This is called a disclaimer, and means that if you use this 
 * program, you are on your own. It DOES, however, pass lint cleanly, which
 * is more than I can say for most stuff posted to the net. Feel free to 
 * fix, break, enhance, change, or do anything to this program except
 * claim it to be your own (unless, of course, you break it...). Passing
 * enhancements back to me would be nice, too.
 *
 *	chuq von rospach, national semiconductor (nsc!chuqui)
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <ctype.h>

#define FALSE		0
#define TRUE		1
#define HASHVAL		37	/* hash value for sub-dirs. Prime number! */
#define NUMDIRS		1024	/* number of dirs that can be pushed */
#define SAVENEWS	"/usr/spool/savenews" /* home of the archive */
#define LOGFILE		"LOGS"  /* subdir in SAVENEWS to save logs in */
#define JOBLOG		"joblog" /* where log of this job is put */
#define DIRMODE		0755    /* mkdir with this mode */
#define COPYBUF		8192    /* block read/write buffer size */

char *Progname;			/* name of the program for Eprintf */
char line[BUFSIZ];		/* general purpose line buffer */

#define NUM_HEADERS	5	/* number of headers we are saving */
#define GROUP_HEADER	1	/* where Newsgroup will be found */
#define SUBJECT_HEADER	2	/* where Subject will be found */
#define MESSAGE_HEADER	3	/* where Message-ID will be found */
char header_data[NUM_HEADERS][BUFSIZ];
char *headers[NUM_HEADERS] =
{
    "From:",
    "Newsgroups:",
    "Subject:",
    "Message-ID:",
    "Date:"
};

long num_saved = 0;		/* number of articles saved */
FILE *logfp;			/* file pointer to joblog file */

char *rindex(), *strcat(), *pop_dir(), *strcpy(), *strsave(), *index();

main(argc,argv)
int argc;
char *argv[];
{
    register int i;
    char joblogfile[BUFSIZ];
    char *dirname;
    
    /*
     * This removes and preceeding pathname so that
     * anything printed out by Eprintf has just the 
     * program name and not where it came from
     */
    if ((Progname = rindex(argv[0],'/')) == NULL)  
	Progname = argv[0];			   
    else
	Progname++;				  

    if (argc == 1) {
	fprintf(stderr,"Usage: %s file [file ...]\n",Progname);
	exit(1);
    }

    sprintf(joblogfile,"%s/%s",SAVENEWS,JOBLOG);
    if ((logfp = fopen(joblogfile,"w")) == NULL)
	fprintf(stderr,"Can't open %s, logging suspended\n",joblogfile);

    for (i = 1 ; i < argc; i++) {	/* process each parameter */
	register int rc;
	if ((rc = is_dir(argv[i])) == -1)
	    continue;
	else if (rc == TRUE)
	    do_dir(argv[i]);
	else
	    save_file(argv[i]);
    }
    while((dirname = pop_dir()) != NULL) {
	do_dir(dirname);	/* process whatever is left on dirstack */
    }
    printf("Total articles saved was %d\n",num_saved);
    exit(0);
}

do_dir(dname) /* process a directory, push other directories on stack */
	      /* to be handled recursively later */
char *dname;
{
    DIR *dirp;
    struct direct *dp;
    char fullname[BUFSIZ];

    if ((dirp = opendir(dname)) == NULL) {
	Eprintf("can't opendir %s\n",dname);
	return;
    }

    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	register int rc;

	if(dp->d_namlen == 2 && !strcmp(dp->d_name,"..") 
	|| (dp->d_namlen == 1 && !strcmp(dp->d_name,".")))
	    continue; /* skip . and .. */

	sprintf(fullname,"%s/%s",dname,dp->d_name);
	if((rc = is_dir(fullname)) == -1)
	    continue;
	else if (rc == TRUE)
	    push_dir(fullname);
	else
	    save_file(fullname);
    }
    closedir(dirp);
}

is_dir(name)
char *name;
{
    struct stat sbuf;

    if (stat(name,&sbuf) == -1) {
	Eprintf("can't stat '%s'\n",name);
	return(-1);
    }
    return((sbuf.st_mode & S_IFDIR) ? TRUE : FALSE);
}

/* VARARGS */
Eprintf(s1,s2,s3,s4,s5,s6,s7,s8,s9)
char *s1,*s2,*s3,*s4,*s5,*s6,*s7,*s8,*s9;
{
    if (logfp == NULL)
	return;
    fprintf(logfp,"%s: ",Progname);
    fprintf(logfp,s1,s2,s3,s4,s5,s6,s7,s8,s9);
    fflush(logfp);
}

/*
 * quick and dirty stack routines.
 *
 * push_dir(name) char *name; 
 *	stores the given string in the stack
 * char *pop_dir()
 *	returns a string from the stack, or NULL if none.
 */

static char *dirstack[NUMDIRS];
static int lastdir = 0;
static char pop_name[BUFSIZ];

push_dir(name)
char *name;
{
    if (lastdir >= NUMDIRS) {
	Eprintf("push_dir overflow!\n");
	return;
    }
    dirstack[lastdir] = strsave(name);
    if (dirstack[lastdir] == NULL)
    {
	Eprintf("malloc failed!\n");
	return;
    }
    lastdir++;
}

char *pop_dir()
{
    if(lastdir == 0)
	return(NULL);
    lastdir--;
    strcpy(pop_name,dirstack[lastdir]);
    dirstack[lastdir] = NULL;
    free(dirstack[lastdir]);
    return(pop_name);
}

char *strsave(s)
char *s;
{
    char *p, *malloc();

    if ((p = malloc((unsigned)strlen(s)+1)) != NULL)
	strcpy(p,s);
    return(p);
}

save_file(name)		/* save the article in the archive */
char *name;
{
    FILE *fp, *ofp, *fopen(), *output_file();
    register int i, nc;
    char diskbuf[COPYBUF];

    Eprintf("saving '%s'\n",name);
    if ((fp = fopen(name,"r")) == NULL) {
	Eprintf("can't open\n");
	return;
    }

    if ((fgets(line,BUFSIZ,fp) == NULL)) {
	Eprintf("0 length file\n");
	fclose(fp);
	return;
    }
    if (!start_header(line)) {
	Eprintf("not a news article\n");
	fclose(fp);
	return;
    }
    read_header(fp);
    if ((ofp = output_file()) == NULL) {
	Eprintf("Can't save\n");
	fclose(fp);
	return;
    }

    for (i = 0; i < NUM_HEADERS; i++)
	fprintf(ofp,"%s\n",header_data[i]);
    fputc('\n',ofp);

    while ((nc = fread(diskbuf,sizeof(char),COPYBUF,fp)) != 0)
	fwrite(diskbuf,sizeof(char),nc,ofp);	/* copy body of article */
    fclose(ofp);
    fclose(fp);
    num_saved++;
    return;
}

start_header(s) /* see if this is the start of a news article */
char *s;
{
    /*
     * If this is coming from B news, the first line will 'always' be
     * Relay-Version (at least, on my system). Your mileage my vary.
     */
    if (!strncmp(s,"Relay-Version:",14))
	return(TRUE);
    /*
     * If you are copying a section of archive already archived by 
     * sendnews, then the first line will be From (unless you changed
     * the headers data structure, then its up to you...)
     */
    if (!strncmp(s,"From:",5))
	return(TRUE);
    return(FALSE);
}

/* 
 * By the time we get here, the first line will already be read in and
 * checked by start_header(). If we are re-copying a savenews archive
 * (which happens when you decide to play with HASHVAL, trust me) then
 * we need to save the From line, so we can't just throw it away. Hence
 * the funky looking do-while setup instead of something a bit more
 * straightforward
 */
read_header(fp)
FILE *fp;
{
    register int i;

    for (i = 0; i < NUM_HEADERS; i++)
	header_data[i][0] = '\0';		/* remove last articles data */

    do {
	char *cp;

	if (line[0] == '\n')	/* always be a blank line after the header */
	    return;

	for (i = 0 ; i < NUM_HEADERS; i++) {
	    if (!strncmp(headers[i],line,strlen(headers[i]))) {
		strcpy(header_data[i],line);
		if (cp = index(header_data[i],'\n'))
		    *cp = '\0';				/* eat newlines */
	    }
	}
    } while (fgets(line,BUFSIZ,fp) != NULL);
}

FILE *output_file() /* generate the name in the archive */
{
    int hashval, copy = 0;
    FILE *fp, *fopen();
    char *p, newsgroup[BUFSIZ], message_id[BUFSIZ];
    char shortname[BUFSIZ], filename[BUFSIZ], filename2[BUFSIZ];

    /* get the first newsgroup */
    p = index(header_data[GROUP_HEADER],':'); /* move past Newsgroups */
    if (!p) {
	Eprintf("Invalid newsgroups\n");
	return(NULL);
    }
    p++;	/* skip the colon */
    while (isspace(*p))
	p++;	/* skip whitespace */
    strcpy(newsgroup,p);
    if (p = index(newsgroup,','))
	*p= '\0';	/* newsgroup now only has one name in it */
    
    /* get the message-id */
    p = index(header_data[MESSAGE_HEADER],':');
    if (!p) {
	Eprintf("Invalid message-id\n");
	return(NULL);
    }
    p++;	/* skip the colon */
    while (isspace(*p))
	p++;	/* skip whitespace */
    if (*p == '<' || *p == '(')
	p++;
    if (*p == '-') /* make negative article id numbers positive (hack) */
	p++;
    strcpy(message_id,p);
    if (p = index(message_id,'.')) /* trim off the .UUCP if any */
	*p = '\0';
    else if (p = index(message_id,'>'))  /* or get the closing bracket */
	*p = '\0';
    else if (p = index(message_id,')')) /* or get the closing paren */
	*p = '\0';
    if (p = index(message_id,'@'))	/* change nnn@site */
	*p = '.';			/* to nnn.site */

    /* generate the hash value for the subdirectory */
    hashval = atoi(message_id) % HASHVAL;

    /* setup the filename to save to */
    sprintf(shortname,"%s/%d/%s",newsgroup,hashval,message_id);
    sprintf(filename,"%s/%s",SAVENEWS,shortname);
    while (exists(filename)) {	/* make it unique if neccessary */

	sprintf(shortname,"%s/%d/%s_%d",newsgroup,hashval,message_id,++copy);
	sprintf(filename,"%s/%s",SAVENEWS,shortname);
    }
    
    strcpy(filename2,filename);			/* must chop off the filename */
    if (p = rindex(filename2,'/'))		/* since we don't want to */
	*p = '\0';				/* to makeparents */
    makeparents(filename2);

    if ((fp = fopen(filename,"w")) == NULL) {
	Eprintf("Can't open %s for output\n",filename);
	return(NULL);
    }
    log(newsgroup,shortname);
    return(fp);
}

exists(name)
char *name;
{
    struct stat sbuf;

    if (stat(name,&sbuf) == -1) {
	return(FALSE);
    }
    return(TRUE);
}

makeparents(name) /* recursively make parent directories */
char *name;
{
    char *p, buf[BUFSIZ];

    if (exists(name))
	return;
    strcpy(buf,name);
    if (!(p = rindex(buf,'/'))) {
	Eprintf("makeparents failed!\n");
	return;
    }
    *p = '\0';
    makeparents(buf);
    mkdir(name,DIRMODE);
}

log(group,name) /* write to the logfile */
char *group, *name;
{
    char *subject, logfile[BUFSIZ];
    FILE *ofp, *fopen();

    /* get the subject */
    subject = index(header_data[SUBJECT_HEADER],':');
    if (!subject) {
	Eprintf("Invalid subject, no log entry\n");
	return;
    }
    subject++;	/* skip the colon */
    while (isspace(*subject))
	subject++;	/* skip whitespace */

    /* generate the place where it goes */
    sprintf(logfile,"%s/%s",SAVENEWS,LOGFILE);
    makeparents(logfile);
    strcat(logfile,"/");
    strcat(logfile,group);

    if ((ofp = fopen(logfile,"a")) == NULL)
    {
	Eprintf("open failed on %s\n",logfile);
	return;
    }
    fprintf(ofp,"%s\t%s\n", name, subject);
    fclose(ofp);
}

