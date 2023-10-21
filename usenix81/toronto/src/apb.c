/*
 * apb -- all points bulletin
 *
 * Write arguments on every user's teletype.
 *
 * This program should be owned by the super-user and run set-user-id.
 *
*/

#include <stdio.h>
#include <pwd.h>
#include <sgtty.h>
#include <utmp.h>
#include <sys/types.h>
#include <sys/stat.h>

#define YES	1
#define NO	0

#define ERROR	-1
#define ROOTUID	0
#define NAMELEN	8

char utmpfile[] "/etc/utmp";

int itsa2741;

main(argc, argv)
char *argv[];
       {struct utmp ubuf;
	register i;
	char mesg[50];
	register char *sender;
	FILE *utptr, *tptr;

	if((sender = getlogin()) != NULL)
		sprintf(mesg,"\07\n\07\napb from %s: ",sender);
	else sprintf(mesg,"\07\n\07\napb from %s: ",getpwuid(getuid())->pw_name);
	if((utptr = fopen(utmpfile,"r")) == NULL)
		{printf("Can't open %s\n", utmpfile);
		 exit(1);
		}
	for(;;)
		{if(fread(&ubuf,sizeof(ubuf),1,utptr) == 0)
			break;
		if(ubuf.ut_name[0])
			{if((tptr = getfp(ubuf.ut_name,ubuf.ut_line)) == ERROR)
				printf("Can't write to %.8s on %s\n", ubuf.ut_name, ubuf.ut_line);
			else
				{fprintf(tptr,"%s",mesg);
				 for(i=1;i<argc;i++)
					fprintf(tptr,"%s ",argv[i]);
				 fprintf(tptr,"\n");
				 fclose(tptr);
				}
			}
		}
	exit(0);
	}

/*
 * getfd(user, ttyname)
 *
 * Return a file descriptor which will allow writing to user
 * "user" on teletype "ttyname".
 *
 * The argument "user" is used only if the recipient teletype
 * is a 2741, in order to locate the appropriate ".write#" file.
 *
 * Assumes that a 2741 will always be protected against writing
 * by others, and that permission to write to a 2741 is determined
 * from the permissions of its ".write#" file, as described
 * in write(V).
 *
*/

getfp(user, tty)
char *user;
char *tty;
	{
	int uid, gid, ttyfd;
	struct sgttyb gttybuf;
	char ttyfile[14];
	FILE *ttyptr;

	/* And for 2741
	 * struct stat statbuf;
	 * struct passwd info;
	 * char writefn[100];
	 */

	sprintf(ttyfile,"/dev/%s",tty);
	/*
	 * Get real uid and gid.
	*/
	uid = getuid();
	gid = getgid();
	if((ttyptr = fopen(ttyfile,"a")) == NULL)
		return(ERROR);
	ttyfd = fileno(ttyptr);
	if(gtty(ttyfd, &gttybuf) == ERROR)
		{fclose(ttyptr);
		 close(ttyfd);
		 return(ERROR);
		}
	itsa2741 = (gttybuf.sg_ispeed == B134) || (gttybuf.sg_ospeed == B134);
	if(!itsa2741)
		{if(writeable(uid, gid, ttyfile))
			return(ttyptr);
		 else
			{fclose(ttyptr);
			 close(ttyfd);
			 return(ERROR);
			}
		}
	/* It's a 2741. Code needs conversion to v7
	close(tfd);
	if(getpwn(user, pwline) != 0)
		return(ERROR);
	if(parsepw(pwline, &info) == ERROR)
		return(ERROR);
	concatn(writefn, info.homedir, "/.write", tty, 0);
	tfd = open(writefn, WRITE);
	if(tfd == ERROR)
		return(ERROR);
	if(fstat(tfd, &statbuf) == ERROR)
		{
		close(tfd);
		return(ERROR);
		}
	if(((statbuf.flags.ownerperm & WRITEPERM) == 0) && (uid != ROOTUID))
		{
		close(tfd);
		return(ERROR);
		}
	seek(tfd, 0, END);
	return(tfd);
	*/
	return(ERROR);
	}

writeable(uid, gid, filename)
int uid, gid;
char *filename;
       {int perm;
	struct stat statbuf;

	if(stat(filename, &statbuf) == ERROR)
		return(NO);
	if((statbuf.st_mode & S_IFDIR) != 0)
		return(NO);
	if(uid == ROOTUID)
		return(YES);
	if(uid == statbuf.st_uid)
		perm = statbuf.st_mode;
	else if (gid == statbuf.st_gid)
		perm = (statbuf.st_mode << 3);
	else
		perm = (statbuf.st_mode << 6);
	return((perm & S_IWRITE) != 0);
	}
