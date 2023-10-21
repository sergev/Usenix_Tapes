/*
**
** Program:	sac.c
**
** Author:	Grenville Whelan Aug-1986, (gdw@uk.co.ssl-macc)
**
** Description:	Sac is  based off the  standard Unix command  ac(8), with
** 		some  enhancements. There were  two reasons for writing a
**		new utility; (1) We  like to  re-create  the 'wtmp' login
**		accounting file every-day, but once 'wtmp' is re-created,
**		existing log-ins are lost. Sac gives  the user the option
**		of automatically re-creating a 'wtmp' file, consisting of
**		only logged-in  users. (2) Ac also accounts for users who
**		have not logged out; sac only  accounts logged-out users.
**		The  options on  sac are the  same as ac, apart  from the
**		'-d' option, which we never use. There is also a new flag
**		'-n' which, if set, will re-create the wtmp file.
**
** Modifications:
**
*/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/dir.h>
#include <stdio.h>

#define WTMP_FILE "/usr/adm/wtmp"		/* Default wtmp file */
#define MODE 422				/* Mode of new wtmp file */

#define MAXUSERS 1000				/* Max no. of users */
#define MAXTTYS 1000				/* Max no. of ttys */

struct wtmp_info {
			char w_tty[8];		/* tty name of login */
			char w_use[8];		/* user name of login */
			char w_hos[16];		/* host name of login */
			long w_log;		/* login time of login */
		};

struct user_info {
			char u_use[8];		/* user name */
			long u_tim;		/* secs logged in */
		};

main(argc,argv)

int argc;
char **argv;

{
	struct wtmp_info wt;
	struct user_info us[MAXUSERS];
	struct wtmp_info lo[MAXTTYS];

	extern int optind;
	extern char *optarg;

	int fd;				/* file descriptor */
	int d_flag=0;			/* gen. purpose flag */
	int i=0, j=0;			/* gen. purpose counters */
	int num_u=0;			/* number of different users */
	int num_t=0;			/* number of different ttys */
	int e_tty=0;			/* tty being dealt with */
	int bytes_read=0;		/* no. bytes read from file */
	int RC_FLAG=0;			/* if flag set, create new wtmp */
	int PU_FLAG=0;			/* if flag set, print all users */

	float time=0;			/* login hours */
	float total=0;			/* total login hours */

	char dum[36];			/* dummy string */
	char WTMP[MAXNAMLEN];		/* wtmp file name */
	char *gen_ptr;			/* gen. purpose pointer */

	/*
	** Make wtmp file default of WTMP_FILE
	*/

	strcpy(WTMP,WTMP_FILE);

	/* 
	** Get command arguments
	*/

	while((i=getopt(argc,argv,"npw:"))!=EOF)
		switch(i) {
			case 'n':
				RC_FLAG=1;
				break;
			case 'p':
				PU_FLAG=1;
				break;
			case 'w':
				strcpy(WTMP,optarg);
				break;
			case '?':
				usage(*argv);
				break;
			default:
				break;
			}

	/*
	** Attempt to open wtmp file
	*/

	if((fd=open(WTMP,O_RDONLY))==EOF) {
		fprintf(stderr,"%s: cannot open %s\n",*argv,WTMP);
		exit(-1); }

	/*
	** Read file until bytes_read != 0, ie EOF or corrupt file
	*/

	while((bytes_read=read(fd,dum,36))==36)
	{
		strncpy(wt.w_tty,dum,8);
		strncpy(wt.w_use,&dum[8],8);
		strncpy(wt.w_hos,&dum[16],16);

		/*
		** Convert char login time to long
		*/

		gen_ptr=(char *)&(wt.w_log);
		for(i=32; i<=35; ++i)
			*gen_ptr++=dum[i];

		d_flag=0;

		/* Pseudo code to build array of username
		***************************************************************
		**
		** if(not first user and not NULL)
		**	set flag if user already in array;
		**
		** if(flag not set and user not NULL)
		**	add user to array;
		**
		**************************************************************/

		if((num_u)&&(strlen(wt.w_use)))
			for(i=0; i<num_u; ++i)
				if(!strcmp(us[i].u_use,wt.w_use))
					d_flag=1;

		if((d_flag==0)&&(strlen(wt.w_use)))
			strcpy(us[num_u++].u_use,wt.w_use);

		d_flag=0;
		e_tty=0;

		/* Pseudo code to build up array of ttys
		***************************************************************
		**
		** if(not first tty) {
		**	set flag if tty already in array;
		**	set variable to keep track of tty; }
		**
		** if(flag set) {
		**	add tty to array;
		**	set variable to keep track of tty; }
		**
		**************************************************************/

		if(num_t)
			for(i=0; i<num_t; ++i)
				if(!strcmp(lo[i].w_tty,wt.w_tty)) {
					d_flag=1;
					e_tty=i; }

		if(d_flag==0) {
			strcpy(lo[num_t++].w_tty,wt.w_tty);
			e_tty=(num_t-1); }


		/* Pseudo Code to compile users login times
		***************************************************************
		**
		** if(this is a login)
		** {
		**	if(user already logged in on this tty)
		**	{
		**		find user;
		**		set user's logout on tty to this users log-in;
		**	}
		**
		**	Copy username to user new user of tty;
		**	Copy users' login time to login of tty
		** }
		** else(is a log-out)
		**	{
		**		find user logged in to tty;
		**		add login length of this tty to user total;
		**		Make username of tty NULL;
		**		Make login time of tty 0;
		**	}
		**
		**************************************************************/

		if(strlen(wt.w_use))
		{
			if(lo[e_tty].w_log)
				for(i=0; i<num_u; ++i)
					if(!strcmp(lo[e_tty].w_use,us[i].u_use)) {
						us[i].u_tim += (wt.w_log - lo[e_tty].w_log);
						strcpy(lo[e_tty].w_use,"\0");
						lo[e_tty].w_log=0; }

			strcpy(lo[e_tty].w_use,wt.w_use);
			strcpy(lo[e_tty].w_hos,wt.w_hos);
			lo[e_tty].w_log=wt.w_log; }
		else
			for(i=0; i<num_u; ++i)
				if(!strcmp(lo[e_tty].w_use,us[i].u_use))
				{
					us[i].u_tim += (wt.w_log - lo[e_tty].w_log);
					strcpy(lo[e_tty].w_use,"\0");
					strcpy(lo[e_tty].w_hos,"\0");
					lo[e_tty].w_log=0;
				}
	}

	/*
	** If not end of file, wtmp file corrupt
	*/

	if(bytes_read!=0) {
		fprintf(stderr,"%s: fatal error, %s corrupt\n",*argv,WTMP);
		close(fd);
		exit(-1); }

	close(fd);

	/* Pseudo Code to total up and output results
	***********************************************************************
	**
	** for(count = 1 to number of users)
	** {
	**	set flag to 0;
	**		if(any people arguments)
	**			if(user in people args)
	**				set flag;
	**		else
	**			set flag;
	**	if(flag set)
	**		calc time and add to total;
	**
	**	if(flag set and print all flag set)
	**		output user + users total;
	**	}
	**	output total;
	**
	*********************************************************************/

	for(i=0; i<num_u; ++i)
	{
		d_flag=0;

		if(argc>=(optind+1)) {
			for(j=optind; j<argc; ++j)
				if(!strcmp(*(argv+j),us[i].u_use))
					d_flag=1; }
		else
			d_flag=1;

		if(d_flag) {
			time=us[i].u_tim;
			total += time; }

		if((PU_FLAG)&&(d_flag))
			printf("\t%-8s%6.2f\n",us[i].u_use,(time/3600));
	}

		printf("\ttotal%9.2f\n",(total/3600));

	/* Pseudo code to Recreate wtmp file
	***********************************************************************
	**	
	** if(recreate wtmp flag is set)
	** {
	**	if(can't open wtmp for writing)
	**		error and exit;
	**
	**	for(count = 1 to number of ttys)
	**		if(this tty not logged out)
	**		{
	**			Convert login time of tty back to char;
	**			Recreate string with all wtmp data;
	**
	**		write this string to file;
	**		if(write failed)
	**			error and exit;
	**
	**		close file
	**		}
	** }
	**
	**********************************************************************/

	if(RC_FLAG)
	{

		if((fd=open(WTMP,(O_TRUNC|O_CREAT|O_WRONLY),MODE))==EOF) {
			fprintf(stderr,"%s:cannot open %s\n",*argv,WTMP);
			exit(-1); }

		for(i=0; i<num_t; ++i)
			if(lo[i].w_log!=0) {
				for(j=0; j<=35; ++j)
					strcpy(&dum[j],"\0");

				strncpy(dum,lo[i].w_tty,8);
				strncpy(&dum[8],lo[i].w_use,8);
				strncpy(&dum[16],lo[i].w_hos,16);

				gen_ptr=(char *)&(lo[i].w_log);

				for(j=32; j<=35; ++j)
					dum[j]= *gen_ptr++;

				if(write(fd,dum,36)!=36) {
					fprintf(stderr,"%s: fatal error, %s corrupt\n",*argv,WTMP);
					close(fd);
					exit(-1); }
			}

		close(fd);
	}

}

/*
** Define command usage
*/

usage(command)

char *command;
{
	fprintf(stderr,"usage: %s [-np] [-w file] people...\n",command);
	exit();
}
