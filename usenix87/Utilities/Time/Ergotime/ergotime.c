#include <stdio.h>
#include <sys/time.h>
#define LOADTIMESEQ "t1"

/* This Program is a quickie to load the time of day onto an */
/* ergo 201 terminal and perhaps other 925-compatibles.      */
/* With no command arguments, the present time of day is     */
/* loaded.  If there is a command argument, and it is 4      */
/* chars long and numeric, then that is used as the time     */
/* instead of the present time of day.  This would be useful */
/* for timing a terminal session, in some circumstances.     */
/* If the argument is improper, a snotty message is printed  */
/* and no time load is attempted.                            */
/* A switch of `-e' specifies a different escape sequence    */
/* than the old dec-20 standard of '^[t' is used.            */
/*                                                           */
/*  The Unix standard library routines gettimeofday (2) and  */
/*   ctime (2) are used.         			     */

 main(argc,argv)     
   int argc;
   char *argv[];

 {
     struct timeval enctim;   /* time in encoded form */
     struct timezone waste;   /* unused */ 
     struct tm      *exptim, *localtime(); /* expanded time */

     int i,j,k;  
     char outtim[13], *curp, *inp, *progname;

     /* first remember the command name, for error messages */
     progname = argv[0];


     /* now set up for output */
     curp = outtim;
     *curp++ = '\033';
     strcat(curp,LOADTIMESEQ);
     curp = curp + strlen(LOADTIMESEQ);

     /* interpret command arguments, if any , and 
        set program parameter from them if possible.  */

     inp = *++argv; argc--;  
     if (argc > 1) {    /* check for escape sequence re-specification */
           if (strcmp(inp,"-e") == 0) {
               if (argc > 0)  {
		        strcpy(&outtim[1],*++argv) ; argc--;
                        curp = outtim + strlen(outtim);
                        inp = *++argv;  argc--;
               }
               else fprintf(stderr,"%s: missing argument\n",progname);
           }
     }
     if (argc > 0) {      /* check for load literal time. */
           if (*inp == 'A'  ||  *inp == 'P') *curp++ = *inp++;
           else *curp++ = 'A';
  
           for(i=0; i < 4; i++) {
                  if (inp[i] > '9'  ||  inp[i] < '0')  { 
                       fprintf(stderr,
                        "%s: bad char in input time: %c\n",
                         progname,inp[i]);
                       exit(1);
                  } 
                  *curp++ = inp[i];
           }

           *curp = '\0';
      }
      else  {          /* otherwise load system time */
               if ( gettimeofday(&enctim, &waste) == -1) {
                            fprintf(stderr,"%s: syscall failure\n",
                             progname);
      			    exit(1);
               }
  	       exptim = localtime( &(enctim.tv_sec) );
  				/* remember to send a pointer to time */
				/* routines: see ctime(2) and         */
                                /* gettimeofday(2).                   */ 
               
               if (exptim->tm_hour >= 12) {
                             *curp++ = 'P'; exptim->tm_hour -= 12; }
               else *curp++ = 'A';

	       if (exptim->tm_hour > 9)  {     /* hours */
			sprintf(curp++,"%2d",exptim->tm_hour);
			curp++;
	       }   
               else {
			*curp++ = '0';
			sprintf(curp++,"%1d",exptim->tm_hour);
               }
	       if (exptim->tm_min > 9) {       /* minutes */
			sprintf(curp++,"%2d",exptim->tm_min);
			curp++;
	       }
               else {
			*curp++ = '0';
			sprintf(curp++,"%1d",exptim->tm_min);
               }
               
               *curp = '\0';
	}
        
        printf("\n%s",outtim);
   }
