/*****************************************************************************
**
**  Program Name: priv.c
**
**  Author:	Eric Heilman  SECAD-APB
**		Balistics Research Laboratories
**		Aberdeen Proving Grounds, Maryland 21005
**
**  Date:	May 6, 1985
**
**
**  Revisions:
**
**	Date	|	Who	|  	   Description of change
**----------------------------------------------------------------------------
**
**
**
**  Function:	priv will accept and execute a UNIX command. Arguments given
**		to priv include a UNIX command and that command's argument
**		list. The command is compared to a command/program table and, 
**		if an entry exists for the command, information is extracted
**		from the table. Otherwise, an error is printed.
**
**		The table information is read into a string, which is further
**		parsed into fields using an array of pointers to isolate 
**		fields. No more then 50 fields may exist ( at this time ) in 
**		an information string.
**
**		All entries include the name of the command, the full execute
**		path of that command and instructions on what userid will be
**		used to execute the command. Two types of user id schemes 
**		exist :
**
**			1) With a variable amount of uids, for example:
**
**				date /bin/date 0:* 1:root
**
**			2) With an always used uid, for example:
**
**				ls /bin/ls *
**
**		Note : The "*" indicates that the caller's uid should be used.
**
**		The uid used in the variable setup reflects the number of com-
**		mand arguments given; ie. in the example above, if just the 
**		name "date" ( effectively no arguments ) is used, then the uid
**		would be "*" ( or uid of caller ).
**
**		There are error checks on an errant command name, path and in-
**		valid use of the priv program itself.
**
**		A "-u" option exists so that a user may specify a particular
**		user name.  The Unix command will execute using the specified
**		uid. 
**
**		A "-d" option exists that reports (at present) the effective
**		uid of the executing program ( both in number and in name ).
**
**		All options must appear in front of the Unix command to be run.
**
*****************************************************************************/

#include <pwd.h>
#include <stdio.h>
#include <strings.h>

#include "priv.h"

main(argc,argv)

int  argc;
char *argv[];

{
	int field_count, i;	   	 /* Indices */
	int debug;		   	 /* Used to give info on current uid */

	char *str;		         /* Holder fore string from the argv array */
	char *id_name;		  	 /* Variable to hold absolute uid from user */
	char *fields[MAX_FIELDS]; 	 /* Array of pointers for parsing */

	struct field *compare();	 /* Routine for command name location */
	register struct field *pptr;	 /* Pointer to field delimiter structure */

	infile = "/etc/priv.conf";

	id_name = NULL;
	debug   = 0;

	for(i=0;i<MAX_FIELDS;i++)
		*fields = NULL;

	/*
	** This code allows the user to specify flags to modify
	** the execution of priv.  This code is set up to handle 
	** the "-u" and the "-d" option.  Should more "-" options 
	** be desired, the switch statement will be enlarged.
	*/

	while( **(++argv) != NULL )  {

		str = *argv;

		if (*str == DELIMITER)
			switch ( *(++str) ) {

				case 'u' : argc -= 2;

					   if ( (id_name = *(++argv) ) == NULL){

					   	printf("\n\npriv : Attempt to use the \"-u\" option without a user name.\n\n");
					   	exit(-1);

					   }

					   break;

				case 'd' : --argc;
					   debug = 1;

					   break;

				default  : ;

			}

		else
			break;

	}

	if (argc <= 1){

		printf("\nUsage : priv [ -d ] [ -u user_name ] run_file [ run_file args ]\n\n");
		exit(-1);
	}

	if ( (pptr = compare(*argv)) == F_NULL){

		printf("\nPriv : Program \"%s\" not found in %s\n\n", *argv, infile);
		exit(-1);
	}

	fields[0]   = pptr->front;
	field_count = 1;

	/*
	** Using a field location routine, the following code
	** will parse the information string for further use.
	*/

	while( *(pptr->end) != NEW_LINE){

		/*
		** For each new field, the front pointer is 
		** moved to one past the rear of the last
		** defined field.
		*/

		pptr->front = ++(pptr->end); 
		fieldloc(pptr);
		fields[field_count++] = pptr->front;

	}

	if ( *fields[1] != '/'){

		printf("\nPriv : Path \"%s\" found for command \"%s\" has no beginning \"/\" in \"%s\"\n\n", 
		       fields[1], *argv, infile);
		exit(-1);
	}

	/* 
	** Locate and set the proper ID.
	*/

	rummage(fields, argc, id_name, field_count, debug);

	/* 
	** Code to activate the UNIX command, provided the command exists.
	** The fields variable contains the run path while the Unix command
	** arguments are given via the argv array.
	*/

	if ( execv( fields[1], argv) == UNUSED){

		printf("\nPriv : Invalid executable \"%s\" found in %s\n\n",fields[1], infile);
		exit(-1);
	}

}

/*****************************************************************************
**
**  Function Name: compare
**
**  Function: 	compare validates the command given in the call to priv against
**		the program table file. If the command is found in the program
**		table, compare will extract the information line and return
**		a pointer set to the command name in the information string.
**
**		Note: The program information file is /etc/priv.conf.
**
**		At present, table information strings can be a maximum of
**		MAX_LINE ( presently 256 ) characters in length.
**
**		If the program can't open the program table file, an error
**		message is produced giving the expected path of the file.
**
*****************************************************************************/

register struct field *compare(name)

char *name;				/* Name of command to be compared  */

{
	char *fgets();			/* Standard file string function   */

	register struct field *ptr;	/* Field delimiter pointer         */

	static char progline[MAX_LINE];	/* Real string of program info     */
	static struct field line_field; /* Real field delimiter structure  */

	FILE *fds, *fopen();		/* Standard file variables	   */

	ptr = &line_field;

	if ( (fds = fopen(infile, "r")) == NULL){

		printf("\nPriv : Can't find the program table file -- should exist as %s\n\n", infile);
		exit(-1);
	}

	/*
	** Check for possible standard file tampering.
	*/

	if ( fileno(fds) <= 2)
		exit(-1);

	/*
	** Below, file strings are read; the first field is then compared to 
	** the desired command entered by the user. If a match is found, then
	** a pointer delimiter structure is returned. This structure will 
	** point to the name field of the information string. However, if no 
	** match is found, then a null pointer is returned.
	*/

	while ( (ptr->front = fgets(progline, MAX_LINE, fds) ) != NULL){

		fieldloc(ptr);

		if ( strcmp(name,ptr->front) == 0 )
			return(ptr);

		}

	return(F_NULL);

}

/*****************************************************************************
**
**  Function Name: fieldloc
**
**  Function:	First, all comments and <CR> lines are skipped. Then, provided 
**		a proper info line is found, all white space is skipped. 
**
**		The field delimiter pointers are now initialized in order to
**		isolate a field within the information string. Next, the end
**		pointer is walked along the string until white space is en-
**		countered. At this point, a field has been isolated.
**
**		To end the field properly, the last phase of this code will
**		affix a null to the end of the field ( in the instruction
**		string permently ).
**
*****************************************************************************/

fieldloc(pptr)

register struct field *pptr;	/* Pointer to the pointer delimiter structure of an in-
				   formation string.	*/


{
	/* Skip comments and blank lines */

	if ( ( *(pptr->front) != COMMENT ) && ( *(pptr->front) != NEW_LINE) ){

		/* Skip white space */

		while( (*(pptr->front) == SPACE) || (*(pptr->front) == TAB) )
			pptr->front++;

		/* Initialize and isolate field */

		pptr->end = pptr->front;

		while( (*(pptr->end) != SPACE) && (*(pptr->end) != TAB) && (*(pptr->end) != NULL) && (*(pptr->end) != NEW_LINE) )
			pptr->end++;

		/* Affix null to end of field */

		if( *(pptr->end) != NEW_LINE)
			*(pptr->end) = NULL;

		/*
		** If a newline doesn't exist at the end of a line, put one
		** these as this is the only way the program can end execution.
		*/

		else{

			*(pptr->end) = NULL;
			pptr->end++;
			*(pptr->end) = NEW_LINE;

		}

	}

	return;

}

/*****************************************************************************
**
**  Function Name: rummage
**
**  Function:	This function finds and attempts to set uids. In order to do 
**		this, the second field is searched for a partition (:). Should
**		this field not contain a partition, it is the uid meant to be
**		used no matter the number of given arguments.
**
**		However, if a partition is found, the field corresponding to 
**		the number of arguments given to the UNIX command will be used
**		to find the uid. The actual uid will correspond to the right 
**		portion of this entry. 
**
**		A further check on the possibility of excessive command
**		arguments ( resulting in no uid determination ) will be
**		enacted when this program runs setuid root.  Also, if a
**		bad uid is found for a process in the program table, a 
**		message is given.
**
**		The debug option reports the uid number and name ( Note:
**		an "*" represents the uid of the present user. ).
**
**		An error message is generated if a "-u" option is attempted
**		on a Unix command which does not have a permission field.
**		The permission field consists of an "@" which will appear
**		as the last field in the priv.conf file.
**
*****************************************************************************/

rummage(flds, amt, abs_name, fcount, debug)

int amt;				/* The number of UNIX arguments */
int fcount;			  	/* The number of fields ( +1 )  */
int debug;				/* Used to give info on current uid */

char *abs_name;				/* Absolute user name */

char **flds;				/* Array of pointers to the fields in info string */

{

	int id;				/* Returned variable from getuid() */

	char *fptr;			/* Individual field pointer */

	struct passwd *getpwnam();	/* Used to find uid for a name */
	struct passwd *uptr;		/* Pointer to the returned structure */

	uptr = P_NULL;

	if (abs_name == NULL){

		/* initialize uid to self */

		id = getuid();

		/* Search the second field */

		fptr = flds[2];

		while( (*fptr != ID_PARTITION) && (*fptr != NULL) )
			fptr++;

		/* 
		** When the "-u" option enabler is the only id pointer, de-
		** fault to the caller's id.
		*/

		if ( *fptr != U_OPT_ENABLE){

			/* Only use if there is no partition in the second field */

			if ( ( *fptr != ID_PARTITION ) )
				fptr = flds[2];

			/* If there is a partition in field two, use the proper field */

			else{

				/* 
				** Note : the proper field corresponds to the number of 
				** arguments given by the user.
				*/

				fptr = flds[amt];

				if ( *fptr == NULL ){

					printf("\nPriv : Not enough id fields given in %s for the amount of\n",
			  			  "       arguments given in call.\n\n", infile);
					exit(-1);
				}

				while( (*fptr++ != ID_PARTITION) && (*fptr != NULL) )
					;


			}

		}

	}

	/*
	** Code to set the absolute uid for this run.  The last field is
	** checked for a "@"; which signifies that the command is executable
	** with "-u" option.
	*/

	else if ( *flds[--fcount] == U_OPT_ENABLE)
		fptr = abs_name;


	else{

		printf("\nPriv : Attempt to use \"%s\" with the \"-u\" is not allowed!!\n\n", flds[0]);
		exit(-1);
	}

	/*
	** The section below returns the proper uid, provide one exists.  An
	** '*' indicates that the user uid should be used.  
	** Any other id requires a setuid to that value -- privileged mode.
	*/

	if ( ( *(fptr) != CALLER ) ){

		uptr = getpwnam(fptr);
		id   = uptr->pw_uid;

	}

	if ( setuid(id) == UNUSED ){

	 	printf("\nPriv: Attempt to set unknown user id \"%s\" possibly in \"-u\" \n", fptr);
		printf("       or table file %s\n\n", infile);
		exit(-1);
	}

	/*
	** Code to report the uid number and name.
	*/

	if (debug)
		printf("\nPriv has set effective uid to %d, named %s\n\n",id, fptr);

	return;

}
