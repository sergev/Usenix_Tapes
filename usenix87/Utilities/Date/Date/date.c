#include <stdio.h>
#include <sys/time.h>

char *dayshort[] = {
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	"Sun"
};

char *monthshort[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

char *days[] = {
	"monday",
	"tuesday",
	"wednesday",
	"thursday",
	"friday",
	"saturday",
	"sunday"
};

char *months[] = {
	"january",
	"february",
	"march",
	"april",
	"may",
	"june",
	"july",
	"august",
	"september",
	"october",
	"november",
	"december"
};

char *jrs[] = {
	"lun",
	"mar",
	"mer",
	"jeu",
	"ven",
	"sam",
	"dim"
};

char *ms[] = {
	"jan",
	"fev",
	"mar",
	"avr",
	"mai",
	"juin",
	"juil",
	"aou",
	"sep",
	"oct",
	"nov",
	"dec"
};

char *jours[] = {
	"lundi",
	"mardi",
	"mercredi",
	"jeudi",
	"vendredi",
	"samedi",
	"dimanche"
};

char *mois[] = {
	"janvier",
	"fevrier",
	"mars",
	"avril",
	"mai",
	"juin",
	"juillet",
	"aout",
	"septembre",
	"octobre",
	"novembre",
	"decembre"
};

main( argc, argv)

int	argc;
char	**argv;

{	long	l;

	int 	i, j,
		fflg, lflg,
		iday, imonth;

	char	*word[7],
		format[80],
		*command,
		*date,
		day[15],
		month[15],
		daymonth[3],
		hour[3],
		minute[3],
		second[3],
		year[5];

/* get date				*/

	time(&l);
	date = ctime(&l);

/* print date and exit, if no argument	*/

	if (argc == 1) {
		printf( "%s", date);
		exit(0);
	}

/* save name of command			*/

	command = argv[0];

/* get options if any			*/

	argc--;	argv++;
	lflg = fflg = 0;
	while(argv[0][0] == '-' && argv[0][1]) {
	    for( i=1; argv[0][i]; i++)
		switch(argv[0][i]) {

		case 'f':
			fflg++;
			break;

		case 'l':
			lflg++;
			break;

		default:
			fprintf( stderr,
			"%s : bad option %c\n", command, argv[0][i]);
		}
	    argc--; argv++;
	}
	if (argv[0][0] == '-') {
	    argc--; argv++;
	}

/* initialize day, month, ...		*/

	substrcpy( date,  0,  2, day);
	substrcpy( date,  4,  6, month);
	if (date[8] == ' ') {
	    daymonth[0] = date[9];
	    daymonth[1] = '\0';
	    }
	else
	    substrcpy( date,  8,  9, daymonth);
	substrcpy( date, 11, 12, hour);
	substrcpy( date, 14, 15, minute);
	substrcpy( date, 17, 18, second);
	substrcpy( date, 20, 23, year);


/* compute iday, imonth			*/

	for( iday=0; iday<7; iday++) 
		if (!strncmp( dayshort[iday], day, 3)) break;
	for( imonth=0; imonth<12; imonth++) 
		if (!strncmp( monthshort[imonth], month, 3)) break;

/* look for french and/or long output	*/

	if (fflg)
	    if (lflg) {
		strcpy( day,   jours[iday]);
		strcpy( month, mois[imonth]);
		}
	    else {
		strcpy( day,   jrs[iday]);
		strcpy( month, ms[imonth]);
	    }
	else
	    if (lflg) {
		strcpy( day,   days[iday]);
		strcpy( month, months[imonth]);
	    }

/* check number of arguments		*/

	if (argc == 1 || argc > 8) {
		fprintf( stderr,
		"%s : wrong number of arguments\n", command);
		exit(1);
	}

	if (argc) {

/* translate C convention like \n, \t, ...
   into single char.			*/

	    for ( i=0, j=0; argv[0][i]; i++, j++) {
		format[j] = argv[0][i];
		if (argv[0][i] == '\\') 
			switch (argv[0][++i]) {

			case 'n':
				format[j] = '\n';
				break;

			case 't':
				format[j] = '\t';
				break;

			case 'b':
				format[j] = '\b';
				break;

			case 'r':
				format[j] = '\r';
				break;

			case 'f':
				format[j] = '\f';
				break;

			case '\\':
				break;

			default:
				format[++j] = argv[0][i];
			}
	    }
	    format[j] = '\0';
	    argc--; argv++;

/* order arguments			*/

	    for( i=0; i<7; i++) word[i] = '\0';
	    j = 0;
	    while (argc) {
		for( i=0; argv[0][i]; i++)
			switch(argv[0][i]) {

			case 'd':
			case 'j':
				word[j++] = day;
				break;

			case 'M':
				word[j++] = month;
				break;

			case 'D':
			case 'J':
				word[j++] = daymonth;
				break;

			case 'h':
				word[j++] = hour;
				break;

			case 'm':
				word[j++] = minute;
				break;

			case 's':
				word[j++] = second;
				break;

			case 'A':	
			case 'Y':	
				word[j++] = year;
				break;

			case ',':
				break;

			default:	
				fprintf( stderr,
				"%s : bad argument %c\n", command, argv[0][i]);
			}
		argc--;	argv++;
	    }

/* output date according to format	*/

	    printf( format, word[0], word[1], word[2],
		    word[3], word[4], word[5], word[6]);
	    }

	else {

	    strcpy( format, "%s %s %s %s   %s:%s:%s\n");
	    if (fflg)
		printf( format, day, daymonth, month, year, hour, minute, second);
	    else
		printf( format, day, month, daymonth, year, hour, minute, second);
	}
}


/* copy string str1[n..m] into str2	*/

substrcpy( str1, n, m, str2)

int	n, m;
char	*str1, *str2;

{
	m++;
	if ( n >= 0 && m >= n ) {
		while( n-- && *str1++ && m--);
		while( m-- && ( *str2++ = *str1++ ));
	}
	*str2='\0';
}
