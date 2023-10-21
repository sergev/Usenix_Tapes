/************************************************************************/
/*  checkmail [-unaoq] [file]						*/
/*									*/
/*  This program lists the mail found in your mail spool file and 	*/
/*  tells you who it's from and the subject, one line per letter.	*/
/*									*/
/*  -u		causes it to only list the unread mail.			*/
/*  -n		causes it to only list the NEW mail.			*/
/*  -a		causes it to list all mail in the file. (default)	*/
/*  -o		causes it to skip outgoing mail (from yourself)		*/
/*  -q		causes it to work quietly, returns 0 exit status if	*/
/*		have mail and 1 if you have no mail (-n applies here)	*/
/*  file	causes it to use a file other that your spool file	*/
/*		use + in front of the name to indicate a folder		*/
/*									*/
/* Copyright 1986 Gary Morris.  Unlimited non-commercial use permitted  */
/* Author: ucbvax!sdcsvax!telesoft!gmorris				*/
/* $Header: RCS/checkmail.c,v 1.8 86/08/20  garym rel $ 		*/
/************************************************************************/

#include <stdio.h>
#include <strings.h>

/* Defines for Ada-like syntax */
#define then	{
#define or_else	} else {
#define elsif	} else if
#define begin   {
#define end	}
#define endif	}
#define loop	{
#define endloop	}
#define endswitch }
#define MAXLINE	1000
#define false	0
#define true	1

typedef enum { all_mail, new_mail, unread_mail } options;
options option = all_mail;

int show_outgoing = true;
int quiet = false;	/* set true if result reported via exit code */
char *USER;


main(argc,argv)
int argc;
char **argv;
begin
    FILE *fp, *fopen();
    extern char *getenv();
    extern int quiet;
    extern options option;
    extern char *USER;
    extern int show_outgoing;
    extern int optind;
    extern char *optarg;
    int argch;
    int have_mail = false;
    char mailbox[255];

    USER = getenv("USER");

    /*** default mailbox file ***/
    strcpy( mailbox, "/usr/spool/mail/" );
    strcat( mailbox, USER );

    while ( (argch=getopt(argc,argv,"anuqo")) != EOF ) loop
	switch (argch) begin
	    case 'o':
		show_outgoing = false;
		break;
	    case 'a':
		option = all_mail;
		break;
	    case 'n':
		option = new_mail;
	    	break;
	    case 'u':
		option = unread_mail;
	    	break;
	    case 'q':
		quiet = true;
		break;
	    case '?':
		fprintf( stderr, "Usage: %s [-qnuao] [filename]\n", *argv );
		exit(1);
	endswitch
    endloop
    if ( optind < argc ) then
	if ( argv[optind][0] == '+' ) then
	    strcpy( mailbox, getenv("folder") );
	    strcat( mailbox, "/" );
	    strdel( argv[optind], 0, 1 );
	    strcat( mailbox, argv[optind] );
	or_else
	    strcpy( mailbox, argv[optind] );
	endif
    endif

    if ( (fp=fopen(mailbox,"r")) != NULL ) then
	have_mail = HeaderCheck(fp);
    elsif ( ! quiet ) then
	/* ok to not find file, when -q, just no mail there since no file */
	fprintf( stderr, "Error: File not found: %s\n", mailbox );
	exit(1);
    endif

    exit( quiet && !have_mail );
end

int HeaderCheck(fp)		/* returns true if there is mail */
FILE *fp;
begin
    extern options option;
    extern int quiet;
    extern char *USER;
    char input[MAXLINE];	/* current input line */
    char output[MAXLINE];	/* current output line */
    char who_from[80];		/* user name mail is from */
    typedef enum { unknown = 0, in_header = 1, in_body = 2 } states;
    states state = unknown;
    char status = 'N';		/* statuses are: New, Old, Read */
    int have_mail = false;
    int outgoing = false;
    char* p1;
    char* p2;

    while ( fgets(input,MAXLINE,fp) != NULL ) loop
	switch (input[0]) begin
	case 'F':
	    if (strlen(input) >= 5) then
		if ( ((state==unknown) || (state==in_body)) &&
		     (input[1]=='r') && (input[2]=='o') &&
		     (input[3]=='m') && (input[4]==' ') ) then
		    state = in_header;
		    status = 'N';
		    strdel( input, strlen(input)-1, 1 );
		    /* delete any uucp path names in front of name */
		    while ( p1 = index( input, '!' ) ) loop
			strdel( input, 5, p1 - &input[5] + 1 );
		    endloop
		    /* delete any domain type stuff after name */
		    if ( p1 = index( input, '@' ) ) then
			p2 = index( p1, ' ' );
			if ( p2 == 0 ) p2 = index( p1, '\n' );
			if ( p2 != 0 ) strdel( input, p1 - input, p2 - p1 );
		    endif
		    /* see who it's from, if from self ignore it (maybe) */
		    p1 = index( &input[5], ' ' );
		    if ( p1 == 0 ) p1 = index( &input[5], '\n' );
		    strncpy( who_from, &input[5], p1 - &input[5] );
		    who_from[ p1-&input[5] ] = '\0';
		    outgoing = strcmp( who_from, USER ) == 0;
		    strcpy( output, input );
		endif
	    endif
	    break;
	case 'S':
	    if (strlen(input) >= 9) then
		switch (input[1]) begin
		case 'u':
		    if ( (state==in_header) &&
			 (input[1]=='u') && (input[2]=='b') &&
			 (input[3]=='j') && (input[4]=='e') &&
			 (input[5]=='c') && (input[6]=='t') &&
			 (input[7]==':') && (input[8]==' ') ) then
			strdel( input, 0, 9 ); 	/* del: Subject: */
		        strdel( input, strlen(input)-1, 1 );
			strcat( output, " \"" );
			strcat( output, input );
			strcat( output, "\"" );
		    endif
		    break;
		case 't':
		    if ( (state==in_header) &&
			 (input[1]=='t') && (input[2]=='a') &&
			 (input[3]=='t') && (input[4]=='u') &&
			 (input[5]=='s') && (input[6]==':') &&
			 (input[7]==' ') ) then
			status = input[8];
		    endif
		    break;
		endswitch
	    endif
	    break;
	case '\n':
	    if (state == in_header) then
		state = in_body;
	      if ( show_outgoing || !outgoing ) then
		if ( (option==new_mail && status=='N') ||
		     (option==unread_mail && (status=='N' || status=='O')) ||
		     (option==all_mail) ) then
		    /* there is some mail (new or otherwise) */
		    have_mail = true;
		    if ( quiet ) then
			return have_mail;
		    or_else
			strcat( output, "\n" );
			fputs( output, stdout );
		    endif
		endif
	      endif
	    endif
	    break;
	default:
	    break;
	endswitch
    endloop
    return have_mail;
end

strdel( s, index, length )
char *s;
int index;
int length;
begin
    char *src = s+index+length;
    char *dest= s+index;
    int count = (strlen(s)+1)-(index+length);
    for (; count>0; count--) loop
        *dest++ = *src++;
    endloop
end
