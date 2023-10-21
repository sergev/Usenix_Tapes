/*
    kill - David C. Albrecht 8/20/86

    kill provides a facility for aborting jobs issued
    into the background by the E(x)tend utility.  It will also
    update the status of finished jobs and print completion for
    when the mode is not set to immediate notify.
*/

#define MAXSTRING 256
#define NULL 0

/*
    vms interface files
*/
#include <rms.h>
#include <stdio.h>
#include <stsdef.h>
#include <ssdef.h>
#include <descrip.h>

/* descriptors for passing to vms commands */
struct dsc$descriptor_s desc,desc1;

main (argc,argv)
    int argc;
    char **argv;

{   int i;
    int next_job;	/* subprocess number of job */
    int job_status;	/* flag for existence of job_status symbol */
    char str[MAXSTRING];/* multi-purpose:
				get job_status symbol value
				get job logical value
			*/
    char jobstr[15];	/* current bg jobs indicator string */
    char joblogical[15];/* name of individual job logical */
    long pid1, pid2;	/* process id variables */

    /*
	get background job process string
    */
    if (job_status = getsymbol("JOB_STATUS",str)) {
	/*
	    update any completed background jobs
	*/
	strcpy(jobstr,str);
	update_job_status(jobstr,&next_job);
    }
    else {
	*jobstr = '\0';
	next_job = 1;
    }
    /*
	there is an 'X' in the job status string for all background
	jobs currently still outstanding
    */
    if (!argv[1]) {
	fprintf(stderr, "kill {pid | %jobno}\n");
	exit(1);
    }
    if (*argv[1] == '%') {
	if (sscanf(argv[1] + 1, "%d", &i) < 1) {
	    fprintf(stderr, "kill {pid | %jobno}\n");
	    exit(1);
	}
	if (i <= strlen(jobstr) && jobstr[i - 1] != ' ') {
	    /*
		each job maintains a JOB_'number' logical which
		indicates current execute status of the job
	    */
	    sprintf(joblogical,"JOB_%d",i);
	    getlogical(joblogical,str);
	    /*
		kill the job and update the job status string
	    */
	    kill_pid(str);
	    jobstr[i - 1] = ' ';
	    printf("[%d] %s killed\n", i, str);
	}
	else {
	    printf("%s not found\n", argv[1]);
	}
    }
    else {
	if (sscanf(argv[1], "%lx", &pid1) < 1) {
	    fprintf(stderr, "invalid pid %s\n", argv[1]);
	    exit(1);
	}
	for (i = 0; i < strlen(jobstr); i++) {
	    if (jobstr[i] != ' ') {
		/*
		    each job maintains a JOB_'number' logical which
		    indicates current execute status of the job
		*/
		sprintf(joblogical,"JOB_%d",i+1);
		getlogical(joblogical,str);
		if (sscanf(str, "%lx", &pid2) >= 1) {
		    if (pid1 == pid2) {
			jobstr[i] = ' ';
			break;
		    }
		}
	    }
	}
	kill_pid(argv[1]);
	if (i < strlen(jobstr)) {
	    printf("[%d] %s killed\n", i + 1, str);
	}
	else {
	    printf("%s killed\n", argv[1]);
	}
    }
    /*
	set or delete updated job process string
    */
    if (*jobstr) {
	setsymbol("JOB_STATUS",jobstr);
    }
    else if (job_status) {
	delsymbol("JOB_STATUS");
    }
}
/*
    dellogical
    delete a logical from the job space
*/
dellogical(logical_name)
char *logical_name;
{

    setdesc(&desc,logical_name,strlen(logical_name));
    lib$delete_logical(&desc,0);
}
/*
    delsymbol
    delete a symbol from the global table
*/
delsymbol(symbol_name)
char *symbol_name;
{   int tbl;

    setdesc(&desc,symbol_name,strlen(symbol_name));
    tbl = 2;
    lib$delete_symbol(&desc,&tbl);
}
/*
    getlogical
    get a logical's translation
*/
getlogical(logical_name,value)
char *logical_name,*value;
{   int valuelen;

    setdesc(&desc,logical_name,strlen(logical_name));
    setdesc(&desc1,value,MAXSTRING-1);
    valuelen = 0;
    lib$sys_trnlog(&desc,&valuelen,&desc1,0,0,0);
    value[valuelen] = '\0';
    if (!strcmp(logical_name,value)) {
	return(0);
    }
    return(1);

}

/*
    getsymbol
    get a symbol's value
*/
getsymbol(symbol_name,value)
char *symbol_name,*value;
{   int valuelen,status;

    setdesc(&desc,symbol_name,strlen(symbol_name));
    setdesc(&desc1,value,MAXSTRING-1);
    valuelen = 0;
    status = lib$get_symbol(&desc,&desc1,&valuelen,0);
    if (status & STS$M_SUCCESS) {
	value[valuelen] = '\0';
	return(1);
    }
    else {
	return(0);
    }
}

/*
    killpid
    kill a process id specified by a hex number string
*/
kill_pid(str)
char *str;
{   long pid;
    int i;

    if (sscanf(str, "%lx", &pid) < 1) {
	fprintf(stderr, "invalid process id %s\n", str);
	exit(1);
    }
    i = SYS$DELPRC(&pid,0);
    switch (i) {

	case SS$_NORMAL:
	    break;

	case SS$_ACCVIO:
	    fprintf(stderr, "access violation, process not killed\n");
	    exit(1);

	case SS$_NONEXPR:
	    fprintf(stderr, "invalid process specification, process not killed\n");
	    exit(1);

	case SS$_NOPRIV:
	    fprintf(stderr, "no privledge for attempted operation, process not killed\n");
	    exit(1);

	default:
	    fprintf(stderr, "error killing process, return = %d, process not killed",i);
	    exit(1);

    }
}
/*
    setdesc
    set a descriptor entry for a standard ascii string
*/
setdesc(descr,str,strlen)
struct dsc$descriptor_s *descr;
char *str;
int strlen;
{
    descr->dsc$w_length = strlen;
    descr->dsc$a_pointer = str;
    descr->dsc$b_class = DSC$K_CLASS_S;	/* String desc class */
    descr->dsc$b_dtype = DSC$K_DTYPE_T;	/* Ascii string type */
}

/*
    setsymbol
    set a symbol in the global space
*/
setsymbol(symbol_name,value)
char *symbol_name,*value;
{   int tbl;

    setdesc(&desc,symbol_name,strlen(symbol_name));
    setdesc(&desc1,value,strlen(value));
    tbl = 2;
    lib$set_symbol(&desc,&desc1,&tbl);
}
/*
    update_job_status
    update the status of any completed background jobs.
    print completion code to the screen for any non-immediate
    notification style jobs.
*/
update_job_status(active_jobs,next_job)
char *active_jobs;
int *next_job;
{   int i;
    char joblogical[MAXSTRING],status_info[MAXSTRING],*active;

    /*
	there is an 'X' in the active_jobs string for all background
	jobs currently outstanding.
    */
    for (i = 0; i < strlen(active_jobs); i++) {
	if (active_jobs[i] != ' ') {
	    /*
		each job maintains a JOB_'number' logical which
		indicates current execute status of the job
	    */
	    sprintf(joblogical,"JOB_%d",i+1);
	    getlogical(joblogical,status_info);
	    /*
		a symbol prefaced with DONE means the job is complete
	    */
	    if (!strncmp(status_info,"DONE",4)) {
		if (strcmp(status_info,"DONE")) {
		    /*
			if DONE is followed by additional info then
			the job was not immediately notified so its
			completion is signaled now
		    */
		    printf("[%d] %s\n",i+1,status_info);
		}
		/*
		    remove the completed job's logical
		*/
		dellogical(joblogical);
		/*
		    remove the active indicator from the active_jobs
		*/
		active_jobs[i] = ' ';
	    }
	}
    }
    /*
	update active_jobs string to remove trailing blanks
    */
    active = active_jobs + strlen(active_jobs) - 1;
    while (active >= active_jobs && *active == ' ') *(active--) = '\0';
    /*
	set number of next_job sub process
    */
    *next_job = strlen(active_jobs)+1;
}
/*
    upshift
    uppercase a character string
*/
upshift(upname,lwname)
char *upname,*lwname;
{
    while (*(upname++) = toupper(*(lwname++)));
}

