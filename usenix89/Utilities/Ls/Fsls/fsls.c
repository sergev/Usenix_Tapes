/* fsls : list all files on one disk partition */
/* '-s<n>' = size option: min no of blocks rqd */
/*           to qualify for outputting.        */
/* arg1 = file system mount name, e.g. /usr    */
/* arg2 = file to put the output in {terminal} */

/* >>>>>> Install with set-uid to root !!!!!!! */

#include <stdio.h>
#include <sys/file.h>
#include <fstab.h>
#include <mtab.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

#define TRUE 1
#define FALSE 0

/* Define all possible exit values */
#define FOUND    0
#define BAD_CAL  1
#define NO_ROOT  2
#define NO_MTAB  3
#define NOT_FND  4
#define SYS_ERR  5
#define NCK_ERR  6

#define EXECFAIL -1001
#define KILLED   -1002

/* System library functions */
struct fstab *getfsfile() ;
char *malloc() ;

/* Local functions */
char *mypwuid() ;
char *mygrgid() ;

int debug = FALSE ;

main(argc,argv)
int argc; char **argv;
{
    long size = 0L ;            /* Min no of blocks reqd to */
				/* qualify for output (0=no limit) */

    int fd ;                    /* stream for /etc/mtab */
    int n ;                     /* number of bytes read */
    int go_on = TRUE ;          /* loop control in mtab */

    struct fstab *fs ;          /* Pointer to returned fstab entry */
    struct mtab   mt ;          /* Buffer for mtab entry */

    char fs_dev[32] ,           /* Block device found in fstab */
	 mt_dev[32] ,           /* Block device found in  mtab */
	 prefix[32] ;           /* Mount name to prefix filename */

    long pid ;                  /* For making tempfile name */
    char tmpname[20] ;          /* For holding tempfile name */
    FILE *fp ;                  /* Stream for temporary file */

    int rc ;                    /* Return code from ncheck */

    struct stat buffer,         /* For call of stat (2) */
	   *buf = &buffer ;

    char line[122] ;            /* For building output line */
    char filename[100] ;        /* Name of current file     */
    register char *name ;       /* Pointer to name in line  */

    u_short prot ;              /* All for getting info about file */
    u_short mode ;
    char *uid ;
    char *gid ;


    while (--argc > 0 && (*++argv) [0] == '-')
       switch ( *(argv[0] + 1) )
	  {
	  case 'd' : debug = TRUE ;
		     break;

	  case 's' : sscanf(argv[0]+2,"%ld",&size) ;
		     if (size < 0L)
			{
			fprintf(stderr,"fsls: bad '-s' value\n") ;
			exit(BAD_CAL) ;
			}
		     else ;
		     break;

	  default  :  fprintf(stderr,"fsls: bad option '%s'\n", argv[0]) ;
		      exit(BAD_CAL) ;
	  }

    /* Check correct usage (argc now actual no. of arguments) */
    if (argc == 0 || argc > 2)
	{
	fprintf(stderr,
	      "Usage: fsls [-s<n>] file_system [output_file]\n") ;
	exit(BAD_CAL) ;
	}
    else ;

    /* Mount name is prefixed to filename unless '/' */
    if (strcmp(*argv,"/") == 0)
	*prefix = '\0' ;
    else
	strcpy(prefix,*argv) ;

    /* First get the block device of the mount name from fstab */
    /* Best to do this first, as '/' has no entry in mtab */
    setfsent() ;
    if ((fs=getfsfile(*argv)) == (struct fstab *)0)
	{
	/* Didn't find it in fstab */
	/* Trap out if '/' (which isn't in mtab) */
	if (*prefix == '\0')
	    {
	    fprintf(stderr,"fsls: Cant find '/' in /etc/fstab\n") ;
	    exit(NO_ROOT) ;
	    }
	else
	    /* Hope to find it in mtab */
	    *fs_dev = '\0' ;
	}
    else
	strcpy(fs_dev,fs->fs_spec) ;

    endfsent() ;

    /* If the file-system isn't '/' cross-check in mtab */
    if (*prefix == '\0')
	{
	strcpy(mt_dev,fs_dev) ;
	go_on = FALSE ;
	}
    else
	/* Open mtab */
	if ((fd=open("/etc/mtab",O_RDONLY,0)) < 0)
	    {
	    fprintf(stderr,"fsls: Cant open /etc/mtab\n") ;
	    exit(NO_MTAB) ;
	    }
	else ;

    /* Loop to read one mtab entry and check if name found */
    while ( go_on )
	{
	/* Read the next entry into the structure */
	n = read(fd,&mt,sizeof(struct mtab)) ;
	if (n < sizeof(struct mtab))
	    {
	    /* End of file (or incomplete last entry) */
	    fprintf(stderr,"fsls: %s - not mounted\n",*argv) ;
	    exit (NOT_FND) ;
	    }
	else
	    {
	    /* Entry is there : check the mount name */
	    if (strcmp(mt.m_path,*argv) == 0)
		{
		/* Found it - note name & end loop */
		sprintf(mt_dev,"/dev/%s",mt.m_dname) ;
		go_on = FALSE ;
		}
	    else
		/* Didnt find it - let the loop continue */
		;
	    }   /* End of 'entry is there' */

	}       /* End of mtab loop */


    /* Warn caller about anything odd */
    if (*prefix == '\0')
	{
	fprintf(stderr,
	       "fsls: Warning - '/' not in mtab, using fstab entry (%s)\n",
	       mt_dev) ;
	}
    else
    if (*fs_dev == '\0')
	{
	fprintf(stderr,
	       "fsls: Warning - no fstab entry, using mtab entry (%s)\n",
	       mt_dev) ;
	}
    else
	if (strcmp(mt_dev,fs_dev) != 0)
	    {
	    fprintf(stderr,
		   "fsls: Warning - mtab (%s) overrides fstab (%s)\n",
		   mt_dev,fs_dev) ;
	    }
	else ;

    /* Call ncheck(8) with stdout diverted to temp file */
    pid = getpid() ;
    sprintf(tmpname,"/tmp/fsls%ld",pid) ;

    if (freopen(tmpname,"w",stdout) == NULL)
	{
	fprintf(stderr,"fsls: Cant reopen stdout\n") ;
	exit(SYS_ERR) ;
	}
    else ;

    rc = runcmd("/etc/ncheck",mt_dev,(char *)0) ;
    if (rc != 0)
	{
	fprintf(stderr,"fsls: Bad exit value '%d' from ncheck\n",rc) ;
	unlink(tmpname) ;
	exit(NCK_ERR) ;
	}
    else ;

    if (argc == 1)
	/* No output file given - use terminal */
	freopen("/dev/tty","w",stdout) ;
    else
	{
	if (freopen(argv[1],"w",stdout) == NULL)
	    {
	    fprintf(stderr,"fsls: Cant open output file '%s'\n",argv[1]) ;
	    unlink(tmpname) ;
	    exit(SYS_ERR) ;
	    }
	else ;
	}

    /* Reopen the temp file for reading */
    fp = fopen(tmpname,"r") ;
    if (fp == NULL)
	{
	fprintf(stderr,"fsls: Cant open tempfile %s for reading\n",tmpname) ;
	unlink(tmpname) ;
	exit(SYS_ERR) ;
	}
    else ;

    /* Skip the first line (names the block device) */
    fgets(line,120,fp) ;

    /* Read the temp file line by line */
    while (fgets(line,120,fp) != NULL)
	{
	/* isolate the file name - skip i-node, delete \n */
	line[strlen(line)-1] = '\0' ;
	name = line ;
	while (*name != '/' && *name != '\0') name++ ;
	if (*name == '\0')
	    /* Not expected, but you never know ... */
	    printf("Empty line\n") ;
	else
	    {
	    /* Build the absolute pathname and call stat */
	    sprintf(filename,"%s%s",prefix,name) ;
	    if (stat(filename,buf) == -1)
		printf("Can't read %s\n",filename) ;
	    else
		{
		if (size > 0L && buf->st_blocks < size)
		    /* Skip output - file too small */
		    ;
		else
		    {
		    /* Find what we want to know about the file */
		    uid  = mypwuid((int)buf->st_uid) ;
		    gid  = mygrgid((int)buf->st_gid) ;
		    prot = buf -> st_mode & S_IFMT ;
		    mode = buf -> st_mode & 07777 ;

		    /* Output the info according to file type */
		    switch(prot) {

		    case S_IFDIR :
			printf("Directory %4o %9s %9s %5ld %s\n",
				mode,uid,gid,buf->st_blocks,filename) ;
			break ;

		    case S_IFCHR :
			printf("Char spec %4o %9s %9s %5ld %s\n",
				mode,uid,gid,buf->st_blocks,filename) ;
			break ;

		    case S_IFBLK :
			printf("Blck spec %4o %9s %9s %5ld %s\n",
				mode,uid,gid,buf->st_blocks,filename) ;
			break ;

		    case S_IFREG :
			printf("Reg. file %4o %9s %9s %5ld %s\n",
				mode,uid,gid,buf->st_blocks,filename) ;
			break ;

		    case S_IFLNK :
			printf("Symb link %4o %9s %9s %5ld %s\n",
				mode,uid,gid,buf->st_blocks,filename) ;
			break ;

		    case S_IFSOCK :
			printf("Socket... %4o %9s %9s %5ld %s\n",
				mode,uid,gid,buf->st_blocks,filename) ;
			break ;

		    }   /* End of switch */
		    }   /* End of else 'not too small' */
		}       /* End of else 'stat was ok'   */
	    }           /* End of else 'line not empty' */
	}               /* End of while  */

    /* Tidy up */
    fclose(fp) ;
    unlink(tmpname) ;
    endpwent() ;
    endgrent() ;

    /* Any files created belong to the effective uid (root) */
    if (argc == 2)
	/* Set the file owner to the real caller */
	chown(argv[1],getuid(),getgid()) ;
    else ;

    exit(FOUND) ;
}

/* runcmd.c */
/* Performs a fork + exec , runs the given program with the */
/* given arguments in the son , waits for the son and       */
/* returns its exit value.                                  */

runcmd(cmd,va_list)
char *cmd ; int va_list ;
    {

    /* cmd is a pointer to the name of the cmd to run */
    /* va_list is a dummy used to get the address of  */
    /* the actual parameter list. you can put as many */
    /* actual pars. as you like, but end with (char *)0 !!  */

    /* These macros extract lower / higher byte from an integer */
#   define lobyte(s) s&0377
#   define hibyte(s) (s>>8)&0377

    int pid,                    /* pid returned by fork     */
	i,                      /* counter                  */
	w,                      /* returnvalue from wait    */
	status,                 /* returnstatus from wait   */
	term_status,            /* termination value of son */
	son_status ;            /* exit-value of son        */

    char *argv[20] ,            /* For actual parameters */
	 *ap ;                  /* Ptr to actual params. */

    /* Set the ptr ap to the actual parameter list */
    ap = (char *) &va_list ;

    /* Copy act. par. ptrs into argv */
    /* argv[0] = cmd name */
    argv[0] = cmd ;
    i = 1 ;
    do
	{
	argv[i] = ((char **) (ap += sizeof(char *))) [-1] ;
	}
    while (argv[i++] != (char *)0 ) ;

    /* Duplicate this process */
    pid = fork () ;

    if (pid == 0)
	{
	/* Son process : run command */
	/* execvp looks for cmd in the 'path' directories */
	execvp(cmd,argv) ;
	/* Exec failed : error message and return */
	fprintf(stderr,"runcmd: cant execute program '%s'\n",cmd) ;
	exit(EXECFAIL) ;
	}
    else ;

    /* Parent process : wait for son */
    while ((w=wait(&status)) != pid && w != -1)
	;

    /* Split up status into two bytes */
    term_status = lobyte(status) ;
    son_status  = hibyte(status) ;

    /* Return son exit-value if normal termination */
    if (term_status == 0)
	return(son_status) ;
    else
	{
	/* Abnormal termination, e.g. killed */
	fprintf(stderr,
		"runcmd: abnormal termination of program '%s'\n",cmd) ;
	return(KILLED) ;
	}

    }

    /* This structure is used in mypwuid and mygrgid */
    struct pwgr {
		int pg_int ;
		char pg_chr[10] ;
		struct pwgr *pg_nxt ;
		} ;

char *mypwuid(id)
int  id;
{
    /* Returns a pointer to the char. form of the uid */
    /* as found in /etc/passwd, decimal form if not found */

    /* Manages a list of those values already found  */
    /* to speed things up - saves lots of getpwuid's */

    struct pwgr *pg ;
    static struct pwgr *pg_head = (struct pwgr *)0 ,
		       *pg_tail = (struct pwgr *)0 ;

    struct passwd *pw ;

    /* First search the list of known uid's */
    pg = pg_head ;
    while (pg != (struct pwgr *)0)
	if (pg->pg_int == id) break; else pg = pg->pg_nxt ;

    /* Return if we found it */
    if (pg != (struct pwgr *)0)
	return(pg->pg_chr) ;
    else ;

    /* Look for it in passwd and add result to chain */

    /* First of all, add the new chain element */
    pg = (struct pwgr *) malloc(sizeof (struct pwgr)) ;
    if (pg_head == (struct pwgr *)0)
	{
	/* First element */
	pg_head = pg ;
	pg_tail = pg ;
	}
    else
	pg_tail->pg_nxt = pg ;

    pg->pg_int = id ;
    pg->pg_nxt = (struct pwgr *)0 ;

    /* And now look for the character name */
    pw = getpwuid(id) ;
    if (pw == (struct passwd *)0)
	/* Didn't find it - use decimal representation */
	sprintf(pg->pg_chr,"<%d>",id) ;
    else
	{
	/* Did find it - copy it in */
	strncpy(pg->pg_chr,pw->pw_name,9) ;
	pg->pg_chr[9] = '\0' ;
	}

    /* Don't forget to rewind the file ! */
    setpwent() ;

    return(pg->pg_chr) ;
    }

char *mygrgid(id)
int  id;
{
    /* Returns a pointer to the char. form of the gid */
    /* as found in /etc/group, decimal form if not found */

    /* Manages a list of those values already found  */
    /* to speed things up - saves lots of getgrgid's */

    struct pwgr *pg ;
    static struct pwgr *pg_head = (struct pwgr *)0 ,
		       *pg_tail = (struct pwgr *)0 ;

    struct group *gr ;

    /* First search the list of known uid's */
    pg = pg_head ;
    while (pg != (struct pwgr *)0)
	if (pg->pg_int == id) break; else pg = pg->pg_nxt ;

    /* Return if we found it */
    if (pg != (struct pwgr *)0)
	return(pg->pg_chr) ;
    else ;

    /* Look for it in group and add result to chain */

    /* First of all, add the new chain element */
    pg = (struct pwgr *) malloc(sizeof (struct pwgr)) ;
    if (pg_head == (struct pwgr *)0)
	{
	/* First element */
	pg_head = pg ;
	pg_tail = pg ;
	}
    else
	pg_tail->pg_nxt = pg ;

    pg->pg_int = id ;
    pg->pg_nxt = (struct pwgr *)0 ;

    /* And now look for the character name */
    gr = getgrgid(id) ;
    if (gr == (struct group *)0)
	/* Didn't find it - use decimal representation */
	sprintf(pg->pg_chr,"<%d>",id) ;
    else
	{
	/* Did find it - copy it in */
	strncpy(pg->pg_chr,gr->gr_name,9) ;
	pg->pg_chr[9] = '\0' ;
	}

    /* Don't forget to rewind the file ! */
    setgrent() ;

    return(pg->pg_chr) ;
    }
