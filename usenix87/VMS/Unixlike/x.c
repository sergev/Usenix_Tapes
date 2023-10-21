/*

    e(x)tend - David C. Albrecht 4/4/86

    Extend is called from the VMS shell and provides some of the
    functionality Unix (TM), namely i/o redirection, pipes and back-
    grounding of processes.  Normally the user won't need these facilities
    and can thus run in the more efficient vanilla VMS environment.  It is
    quite simple, however, when the need for the extensions arises to
    invoke extend for one extended command.

    Extend supports:

	i/o redirection:
	    >	redirect sys$output to a file.
	    <	get sys$input from a file AND put the file name as
		the last parameter of the next invoked routine.
	    >&  redirect sys$output to a file and send sys$error to
		the null device.

	pipes:
	    |   spawn two processes and attach a mailbox between them.
		works like > for the source process and like < for the
		receiving process.
	    |&	spawn two processes and attach a mailbox between them.
		works like >& for the source process and like < for the
		receiving process.

	multiple commands per line:
	    ;   a semicolon will separate multiple commands which are
		to be processed serially.

	backgrounding of processes:
	    &   will indicate that the preceeding command sequence is
		to be placed to run in the background.  & also acts
		as a command separator.

	process grouping
	    ()	will group jobs so that the previous commands will
		apply to them as a group.

    all these characters are treated as above unless escaped by a preceeding
    backslash character (\).

    A companion program (jobs) is available which will display current
    status of a background process.

    Note that two modes for background jobs is available:

	If a logical named 'notify' is defined in the DCL shell.
	    The subprocess will signal completion immediately by
	    writing a completion message to the terminal.

	If the logical is not set then.
	    The subprocess will complete silently and completion
	    will be signalled to the user when he runs either
	    jobs or extend.

*/

#define NULL		0
#define SHORT_STR	40
#define MAXSTRING	150	/* max single comand size */
#define MAXCMDS		20	/* max vms commands in one extend command */
/*
    flag bits for a command
    and character process modes
*/
#define ARGUMENT	0
#define IN_FILE		1
#define OUT_FILE	2
#define PIPE_IN		4
#define PIPE_OUT	8
#define BG		16
#define ERROR_OUT	32
#define CLEAR_IN	64
#define CLEAR_OUT	128
#define CLEAR_ERROR	256
/*
    spawn command flag bits
*/
#define NO_WAIT 0x1
#define NOTIFY 0x10
/*
    vms interface files
*/
#include <rms.h>
#include <stdio.h>
#include <stsdef.h>
#include <ssdef.h>
#include <descrip.h>

FILE *comfile;	/* file variable for VMS com file */

/* descriptors for passing to vms commands */
struct dsc$descriptor_s desc,desc1;

/*
    command description record
*/
struct cmd_desc_type {
    int state;		/* flag word indicating various info, see above */
    char *arg_str;	/* text of vms command */
    char *input;	/* input file */
    char *output;	/* output file */
    } cmd_desc[MAXCMDS];

int cmd_cnt;		/* number of vms commands */

int paren_in_process = -1;	/* cmd_cnt number for beginning of
				    paren block in process */

main (argc,argv)
    int argc;
    char **argv;

{   int i,j;
    int unix = FALSE;	/* unix style */
    int keep_comfile = FALSE;	/* delete comfile after execution */
    int flags;		/* flags to lib$spawn */
    int pid;		/* current process id */
    int job_sequence = FALSE;	/* flag indicating a serial job sequence */
    int next_job;	/* subprocess number of job */
    char str[MAXSTRING];	/* multi-purpose:
					extend arguments
					vms com file names
				*/
    char exec_str[SHORT_STR];	/* com file execution text */
    char jobstr[SHORT_STR];	/* current bg jobs indicator string */
    char title[MAXSTRING];	/* text of title for current stage of
				   background process
				*/
    char job_title[MAXSTRING];	/* text of title for completed background
				   process
				*/
    int backgrounded_process = FALSE;	/* backgrounded process flag */

    /*
	get process id
    */
    pid = getpid();

    /*
	get background job process string
    */
    if (getsymbol("JOB_STATUS",str)) {
	strcpy(jobstr,str);
	/*
	    update any completed background jobs
	*/
	update_job_status(jobstr,&next_job);
    }
    else {
	*jobstr = '\0';
	next_job = 1;
    }
    /*
	set updated job process string
    */
    if (*jobstr) {
	setsymbol("JOB_STATUS",jobstr);
    }
    else {
	delsymbol("JOB_STATUS");
    }

    if (argc <= 1) exit(SS$_NORMAL);
    /*
	concatenate arguments into one string
    */
    *str = '\0';
    i = 1;
    if (!strcmp(argv[i],"-u")) {
	i++;
	unix = TRUE;
    }
    if (!strcmp(argv[i],"-k")) {
	i++;
	keep_comfile = TRUE;
    }
    for (; i < argc; i++) {
	if (*str) {
	   strcat(str," ");
	}
	strcat(str,argv[i]);
    }
    /*
	process string into commands
    */
    if (process_arg(str)) {
	return(1);
    }
    /*
	init flag for sequential sequence of commands
    */
    for (i = 0; i < cmd_cnt; i++) {
	if (!job_sequence) {
	    /*
		open a new VMS com file
	    */
	    sprintf(str,"SYS$LOGIN:EXE%0d%0d.COM",pid,i);
	    comfile = fopen(str, "w");
	    if (comfile == NULL) {
		sprintf(str,"EXE%0d%0d.COM",pid,i);
		comfile = fopen(str, "w");
		if (comfile == NULL) {
		    fprintf(stderr,"unable to open process file %s\n",str);
		    return(1);
		}
	    }
	    /*
		error exit line
	    */
	    fprintf(comfile,"$ on error then goto exit\n");
	    /*
		null the job completion title
	    */ 
	    *job_title = '\0';
	}
	else {
	    job_sequence = FALSE;
	}
	/*
	    set title to name of current vms command
	*/
        strcpy(title,cmd_desc[i].arg_str);
	/*
	   append names of following sequential vms commands
	*/
	j = i + 1;
	while (j < cmd_cnt && !(cmd_desc[j-1].state & BG)) {
	    strcat(title," ; ");
	    strcat(title,cmd_desc[j].arg_str);
	    j++;
	}
	/*
	   first time, save title as the job completion title
	*/
	if (!*job_title) {
	    strcpy(job_title,title);
	}
	/*
	   if an asynchronous completing job then generate line to
	   update job status in the vms com file
	*/
	if (cmd_desc[cmd_cnt - 1].state & BG) {
	    fprintf(comfile,
		    "$ define/job/nol JOB_%d \"''f$getjpi(0,\"PID\")' %s\"\n",
		    next_job,title);
	}
	/*
	   unredirect sys$input
	*/
	if (cmd_desc[i].state & CLEAR_IN) {
	    fprintf(comfile,"$ deassign sys$input\n");
	    fprintf(comfile,"$ deassign pas$input\n");
	}
	/*
	   unredirect sys$output and sys$error
	*/
	if (cmd_desc[i].state & CLEAR_OUT) {
	    fprintf(comfile,"$ deassign sys$output\n");
	    fprintf(comfile,"$ deassign pas$output\n");
	}
	if (cmd_desc[i].state & CLEAR_ERROR) {
	    fprintf(comfile,"$ deassign sys$error\n");
	}
	/*
	    redirect output streams for output pipes or files
	*/
	if (cmd_desc[i].state & (OUT_FILE | PIPE_OUT)) {
	    fprintf(comfile,"$ define sys$output %s\n",cmd_desc[i].output);
	    fprintf(comfile,"$ define pas$output %s\n",cmd_desc[i].output);
	    /*
		when sys$error is redirected throw it away by
		sending it to the null device since sys$output
		is a superset of sys$error
	    */
	    if (cmd_desc[i].state & ERROR_OUT) {
		if (unix) {
		    fprintf(comfile,"$ define sys$error %s\n",
				    cmd_desc[i].output);
		}
		else {
		    fprintf(comfile,"$ define sys$error nl:\n");
		}
	    }
	}
	/*
	    redirect input streams for input pipes or file
	    in addition, put input file as last parameter
	    to associated vms command.
	*/
	if (cmd_desc[i].state & (IN_FILE | PIPE_IN)) {
	    fprintf(comfile,"$ define/nolog sys$input %s\n",cmd_desc[i].input);
	    fprintf(comfile,"$ define pas$input %s\n",cmd_desc[i].input);
	    if (unix) {
		fprintf(comfile,"$ %s\n",cmd_desc[i].arg_str);
	    }
	    else {
		fprintf(comfile,"$ %s %s\n",cmd_desc[i].arg_str,cmd_desc[i].input);
	    }
	}
	else {
	    fprintf(comfile,"$ %s\n",cmd_desc[i].arg_str);
	}
	/*
	    if job is not being backgrounded,
	    and there is a following job then,
	    set the job sequence flag
	*/
	if (!(cmd_desc[i].state & BG)) {
	    if (i+1 < cmd_cnt) {
		job_sequence = TRUE;
	    }
	}
	if (!job_sequence) {
	    /*
		finish off vms com file
	    */
	    fprintf(comfile,"$ exit:\n");
	    strcpy(exec_str,"@");
	    strcat(exec_str,str);
	    setdesc(&desc,exec_str,strlen(exec_str));
	    if (!(cmd_desc[i].state & BG)) {
		flags = NULL;
	    }
	    else {
		flags = NO_WAIT;
		if (!(cmd_desc[i].state & PIPE_OUT)) {
		    backgrounded_process = TRUE;
		    /*
			when asynchronous execution add file completion
			and status update lines to vms com file
		    */
		    fprintf(comfile,
	"$ if \"''f$logical(\"NOTIFY\")'\" .nes. \"\" then goto speak\n");
		    fprintf(comfile,"$ define/job/nol JOB_%D \"DONE %s\"\n",
				    next_job,job_title);
		    if (!keep_comfile) {
			fprintf(comfile,"$ delete %s;*\n",str);
		    }
		    fprintf(comfile,"$ exit\n");
		    fprintf(comfile,"$ speak:\n");
		    fprintf(comfile,"$ define/job/nol JOB_%D \"DONE\"\n",
				    next_job);
		    printf("[%d] %s\n",next_job ,job_title);
		    fprintf(comfile,"$ deassign sys$output\n");
		    fprintf(comfile,"$ write sys$output \"[%d] DONE %s\"\n",
				    next_job,job_title);
		    /*
			update background job status string
		    */
		    strcat(jobstr,"X");
		    next_job++;
		}
	    }
	    if (!keep_comfile) {
	 	fprintf(comfile,"$ delete %s;*\n",str);
	    }
	    fclose(comfile);
	    /*
		spawn the process for execution
	    */
	    lib$spawn(&desc,0,0,&flags,0,0,0,0,0,0,0,0);
	}
    }
    if (backgrounded_process) {
	/*
	    if asynchronous job execution then set the background
	    job status symbol on exit
	*/
	setsymbol("JOB_STATUS",jobstr);
    }
}
/*
    add_command
    add a command to the command description structure
*/
add_command(cmd_desc_pt,cmd_cnt_pt,state,arg_str,input,output)
struct cmd_desc_type (*cmd_desc_pt)[];
int *cmd_cnt_pt,*state;
char *arg_str,*input,*output;
{   struct cmd_desc_type *cmd_desc_entry;

    if ((*state & (IN_FILE | OUT_FILE | PIPE_IN | PIPE_OUT)) && !arg_str) {
	fprintf(stderr,"invalid i/o redirection\n");
	return(1);
    }
    if (!*arg_str) {
	return(0);
    }
    /*
	get a pointer to the current command descripion record
    */
    cmd_desc_entry = &((*cmd_desc_pt)[(*cmd_cnt_pt)++]);
    /*
	set the state, argument, input, and output fields and re-init
    */
    cmd_desc_entry->state = *state;
    *state = NULL;
    if (*arg_str) {
	cmd_desc_entry->arg_str = malloc(strlen(arg_str)+1);
	strcpy(cmd_desc_entry->arg_str,arg_str);
	*arg_str = '\0';
    }
    if (*input) {
	cmd_desc_entry->input = malloc(strlen(input)+1);
	strcpy(cmd_desc_entry->input,input);
	*input = '\0';
    }
    if (*output) {
	cmd_desc_entry->output = malloc(strlen(output)+1);
	strcpy(cmd_desc_entry->output,output);
	*output = '\0';
    }
    /*
	initialize the next description record
    */
    init_cmd_desc(&((*cmd_desc_pt)[*cmd_cnt_pt]));
    return(0);

}

/*
    check_mode
    check mode checks the current and next mode,
    command state, input and output, and validates and sets
    the new mode.
*/
check_mode(curr_mode,new_mode,state,input,output)
int curr_mode,new_mode,*state;
char *input,*output;
{
    if (curr_mode == IN_FILE) {
	if (!*input) {
	    fprintf(stderr,"null redirection attempted\n");
	    return(-1);
	}
	else if (paren_in_process >= 0) {
	    cmd_desc[paren_in_process].input = malloc(strlen(input)+1);
	    strcpy(cmd_desc[paren_in_process].input, input);
	    *input = '\0';
	}
    }
    if (curr_mode == OUT_FILE) {
	if (!*output) {
	    fprintf(stderr,"null redirection attempted\n");
	    return(-1);
	}
	else if (paren_in_process >= 0) {
	    cmd_desc[paren_in_process].output = malloc(strlen(output)+1);
	    strcpy(cmd_desc[paren_in_process].output, output);
	    *output = '\0';
	}
    }
    if (new_mode & (IN_FILE | PIPE_IN)) {
	if (*state & (IN_FILE | PIPE_IN)) {
	    fprintf(stderr,"multiple redirection attempted\n");
	    return(-1);
	}
	else {
	    *state |= new_mode;
	}
    }
    else if (new_mode & (OUT_FILE | PIPE_OUT)) {
	if (*state & (OUT_FILE | PIPE_OUT)) {
	    fprintf(stderr,"multiple redirection attempted\n");
	    return(-1);
	}
	else {
	    *state |= new_mode;
	}
    }
    return(new_mode);
}

/*
    dellogical
    delete a logical from the job table
*/
dellogical(logical_name)
char *logical_name;
{   char job_table[20];

    setdesc(&desc,logical_name,strlen(logical_name));
    sprintf(job_table,"LNM$JOB_%d",getuid());
    setdesc(&desc1,job_table,strlen(job_table));
    lib$delete_logical(&desc,desc1);
}

/*
    delsymbol
    delete a symbol from the global level
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
    get a logical translation
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
    get a symbol value
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
   get_pipe
   create a pipe_name and open a mailbox under that name
*/
get_pipe(pipe_name,pipeno)
char *pipe_name;
int pipeno;
{   int i,chan;

    i = getpid();
    sprintf(pipe_name,"COM%0d%0d",i,pipeno);
    setdesc(&desc,pipe_name,strlen(pipe_name));
    sys$crembx(0,&chan,1024,2048,0xff0f,0,&desc);
}

/*
    init_cmd_desc
    initialize a command description entry
*/
init_cmd_desc(cmd_desc_entry)
struct cmd_desc_type *cmd_desc_entry;
{
    cmd_desc_entry->state = NULL;
    cmd_desc_entry->arg_str = NULL;
    cmd_desc_entry->input = NULL;
    cmd_desc_entry->output = NULL;
}

/*
    process_arg
    split the extend argument string and process into separate commands.
*/
process_arg(str)
char *str;
{   /*
	state,arg_str,input,output
	are local temporaries for the corresponding fields in a
	command description record.
    */
    int state = NULL;
    char arg_str[MAXSTRING];
    char input[MAXSTRING];
    char output[MAXSTRING];
    int old_state = NULL;	/* old state values */
    char cstr[2],c;	/* temporary character manipulation storage */
    int char_mode = ARGUMENT;	/* next character addition mode */

    int parens = 0;		/* paren levels saved */
    int paren_levels[4];	/* paren level storage */

    int i;

    /*
	init the command description temporaries
    */
    *arg_str = '\0';
    *input = '\0';
    *output = '\0';
    /*
	zero the number of commands and init the first command
	description record
    */
    cmd_cnt = 0;
    init_cmd_desc(&cmd_desc[cmd_cnt]);

    while (c = *str++) {
	/* command separator */
	if (c == ';') {
	    if ((char_mode =
		    check_mode(char_mode,ARGUMENT,&state,input,output)) < 0) {
		return(1);
	    }
	    old_state = state;
	    if (add_command(cmd_desc,&cmd_cnt,&state,arg_str,input,output)) {
		return(1);
	    }
	    if (old_state & OUT_FILE) state |= CLEAR_OUT;
	    if (old_state & IN_FILE) state |= CLEAR_IN;
	    if (old_state & ERROR_OUT) state |= CLEAR_ERROR;
	    paren_in_process = -1;
	}
	/* pipe */
	else if (c == '|') {
	    if (paren_in_process < 0) {		
		if ((char_mode =
			check_mode(char_mode,PIPE_OUT,&state,input,output)) < 0) {
		    return(1);
		}
		get_pipe(output);
		if (*str == '&') {
		    str++;
		    state |= ERROR_OUT;
		}
		if (add_command(cmd_desc,&cmd_cnt,&state,arg_str,input,output)) {
		    return(1);
		}
		cmd_desc[cmd_cnt - 1].state |= BG;
		/*
		    set up pipe input to next command
		*/
		if ((char_mode =
			check_mode(char_mode,PIPE_IN,&state,input,output)) < 0 ) {
		    return(1);
		}
		cmd_desc[cmd_cnt].input = cmd_desc[cmd_cnt - 1].output;
		char_mode = check_mode(char_mode,ARGUMENT,&state,input,output);
	    }
	    else {
		if ((char_mode =
			check_mode(char_mode,PIPE_OUT,
					&cmd_desc[paren_in_process].state,
					cmd_desc[paren_in_process].input,
					cmd_desc[paren_in_process].output)) < 0) {
		    return(1);
		}
		for (i = paren_in_process + 1; i < cmd_cnt; i++) {
		    if (cmd_desc[i].state & (OUT_FILE | PIPE_OUT)) {
			fprintf(stderr, "multiple redirection attempted\n");
			return(1);
		    }
		}
		if (state & OUT_FILE) {
		    fprintf(stderr, "multiple redirection attempted\n");
		    return(1);
		}
		get_pipe(output);
		cmd_desc[paren_in_process].output = malloc(strlen(output)+1);
		strcpy(cmd_desc[paren_in_process].output, output);
		*output = '\0';
		if (*str == '&') {
		    str++;
		    cmd_desc[paren_in_process].state |= ERROR_OUT;
		}
		if (add_command(cmd_desc,&cmd_cnt,&state,arg_str,input,output)) {
		    return(1);
		}
		cmd_desc[cmd_cnt - 1].state |= BG;
		/*
		    set up pipe input to next command
		*/
		if ((char_mode =
			check_mode(char_mode,PIPE_IN,&state,input,output)) < 0) {
		    return(1);
		}
		cmd_desc[cmd_cnt].input = cmd_desc[paren_in_process].output;
		char_mode = check_mode(char_mode,ARGUMENT,&state,input,output);
		paren_in_process = -1;
	    }
	}
	/* background */
	else if (c == '&') {
	    if ((char_mode =
		    check_mode(char_mode,ARGUMENT,&state,input,output)) < 0) {
		return(1);
	    }
	    if (add_command(cmd_desc,&cmd_cnt,&state,arg_str,input,output)) {
		return(1);
	    }
	    cmd_desc[cmd_cnt - 1].state |= BG;
	    paren_in_process = -1;
	}
	/* redirect output */
	else if (c == '>') {
	    if (paren_in_process < 0) {
		if ((char_mode =
			check_mode(char_mode,OUT_FILE,&state,input,output)) < 0) {
		    return(1);
		}
		if (*str == '&') {
		    str++;
		    state |= ERROR_OUT;
		}
	    }
	    else {
		if ((char_mode =
			check_mode(char_mode,OUT_FILE,
					&cmd_desc[paren_in_process].state,
					cmd_desc[paren_in_process].input,
					cmd_desc[paren_in_process].output)) < 0) {
		    return(1);
		}
		for (i = paren_in_process + 1; i < cmd_cnt; i++) {
		    if (cmd_desc[i].state & (OUT_FILE | PIPE_OUT)) {
			fprintf(stderr, "multiple redirection attempted\n");
			return(1);
		    }
		}
		if (state & OUT_FILE) {
		    fprintf(stderr, "multiple redirection attempted\n");
		    return(1);
		}
		if (*str == '&') {
		    str++;
		    cmd_desc[paren_in_process].state |= ERROR_OUT;
		}
	    }
	}
	/* redirect input */
	else if (c == '<') {
	    if (paren_in_process < 0) {
		if ((char_mode =
			check_mode(char_mode,IN_FILE,&state,input,output)) < 0) {
		    return(1);
		}
	    }
	    else {
		if ((char_mode =
			check_mode(char_mode,IN_FILE,
					&cmd_desc[paren_in_process].state,
					cmd_desc[paren_in_process].input,
					cmd_desc[paren_in_process].output)) < 0) {
		    return(1);
		}
		for (i = paren_in_process + 1; i < cmd_cnt; i++) {
		    if (cmd_desc[i].state & (IN_FILE | PIPE_IN)) {
			fprintf(stderr, "multiple redirection attempted\n");
			return(1);
		    }
		}
		if (state & (IN_FILE | PIPE_IN)) {
		    fprintf(stderr, "multiple redirection attempted\n");
		    return(1);
		}
	    }
	}
	/* job group start */
	else if (c == '(') {
	    if (paren_in_process >= 0 || char_mode != ARGUMENT || *arg_str) {
		fprintf(stderr,"invalid job grouping\n");
		return(1);
	    }
	    paren_levels[parens++] = cmd_cnt;
	}
	/* job group end */
	else if (c == ')') {
	    if (!parens) {
		fprintf(stderr,"mis-matched parenthisis\n");
		return(1);
	    }
	    if (char_mode != ARGUMENT || !(*arg_str)) {
		fprintf(stderr,"invalid job grouping\n");
		return(1);
	    }
	    paren_in_process = paren_levels[--parens];
	}
	else {
	    /* escaping of next character */
	    if (c == '\\') {
		if (!(*cstr = *str++)) {
		    return(1);
		}
	    }
	    /* any other character */
	    else {
		/*
		    space or tab delimits a file name in file redirection
		    modes, reset the character mode to ARGUMENT input.
		*/
		if (c == ' ' || c == '\t') {
		    if ((char_mode == IN_FILE && *input)
		     || (char_mode == OUT_FILE && *output)) {
			if ((char_mode =
				check_mode(char_mode,ARGUMENT,
					   &state,input,output)) < 0) {
			    return(1);
			}
		        continue;
		    }
		}
		*cstr = c;
	    }
	    *(cstr+1) = '\0';
		
	    /*
		three character modes are available for normal character
		ARGUMENT - part of a vms command
		IN_FILE  - part of a input redirection file name
		OUT_FILE - part of an output redirection file name
	    */
	    switch(char_mode) {

		case ARGUMENT:
		    strcat(arg_str,cstr);
		    break;

		case IN_FILE:
		    strcat(input,cstr);
		    break;

		case OUT_FILE:
		    strcat(output,cstr);
		    break;

	    }	    
	}
    }
    /*
	exit cleanup
    */
    if (check_mode(char_mode,ARGUMENT,&state,input,output) < 0) {
	return(1);
    }
    if (add_command(cmd_desc,&cmd_cnt,&state,arg_str,input,output)) {
	return(1);
    }
    return(0);
}
/*
    setdesc
    set descriptor to a standard ascii string
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
    set a symbol in the global table
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
	jobs currently still outstanding.
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

