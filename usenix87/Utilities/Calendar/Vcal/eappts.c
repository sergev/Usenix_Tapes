/*
 * Module:	eappts.c
 *
 * Purpose:	enters data into appointments file
 *
 * Author:	Mike Essex
 *
 * Date:	Sep. 16, 1986
 *
 * Includes:
 *	stdio.h
 * Discussion:
 *	Inputs data from standard input and writes it to ~/.appointments
 *	file
 * 
 *
 * Edit History
 * ==== =======
 *
 * Date      Who	What
 * ----      ---	----------------------------------------------
 * 12/1/86   me		added multifile capability
 * 12/1/86   me		changed 'home' from *home[80] to home[80]
 *
 */

#include <stdio.h>
#define		NL	'\010'


/*
 * Procedure:	main
 *
 * Function:	inputs data and writes to users .appointments file
 *
 * Discussion:
 *	Prompts users for required appointment information, inputs
 *	that info and then appends it to ~/.appointments file.
 */

main(argc,argv)

int	argc;
char	*argv[];

{
    FILE	*fptr;
    char	tmpbuf[80];
    int		i,j;
    char	*getenv();
    char	home[80];
    char	datedata[20];
    char	timedata[20];
    char	msgdata[40];
    int		month,day,year,time;
    int		index[5];

    if (argc == 2) {
	strcpy(home,argv[1]);
    }
    else {
	strcpy(home,getenv("HOME"));
	strcat(home,"/.appointments");
    }

    fptr = fopen(home,"a");
    if (fptr) {
	printf("What is the date of appointment?  (mm dd yyyy)  ");
	fgets(datedata,20,stdin);
	i = 0;
	j = 0;
	while(i < 20) {
	    while ((i < 20) && (datedata[i] == ' ')) i++;
	    index[j++] = i;
	    while ((i < 20) && (datedata[i] != ' ')) i++;
	}
	month = atoi(&datedata[index[0]]);
	day = atoi(&datedata[index[1]]);
	year = atoi(&datedata[index[2]]);
	if (year < 100) year += 1900;

	printf("What time (24 hour time) is the appointment?  (tttt)  ");
	fgets(timedata,20,stdin);
	time = atoi(timedata);
	printf("What is the message?  (32 characters max)  ");
	fgets(msgdata,34,stdin);
	msgdata[strlen(msgdata)-1] = NULL;
	fprintf(fptr,"%d,%d,%d,%4d,%s\n",month,day,year,time,msgdata);
    }
    else {
	printf("Error:  Cannot open %s file",argv[1]);
	abort();
    }
    fclose(fptr);
} /* main */
