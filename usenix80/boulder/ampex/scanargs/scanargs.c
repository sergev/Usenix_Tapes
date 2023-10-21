/* 
		Version 7 compatible
	Argument scanner, scans argv style argument list.

	Some stuff is a kludge because sscanf screws up

	Gary Newman - 10/4/1979 - Ampex Corp. 
*/
#include <stdio.h>
#include <ctype.h>

#define YES 1
#define NO 0
#define ERROR(msg)  {fprintf(stderr, "msg\n"); goto error; }

scanargs(argc,argv,format,arglist)
  int argc;
  char **argv;
  char *format;
  int arglist[];
  {
	_scanargs(argc,argv,format,&arglist);
  }

_scanargs(argc, argv, format, arglist)
  int argc;
  char **argv;
  char *format;
  int *arglist[];
  {
	register check; /* check counter to be sure all argvs are processed */
	register char *cp;
	register cnt;
	char tmpflg;	/* temp flag */
	char c;
	char numnum;	/* number of numbers already processed */
	char numstr;	/* count # of strings already processed */
	char tmpcnt;	/* temp count of # things already processed */
	char required;
	char exflag;	/* when set, one of a set of exclusive flags is set */
	char excnt;	/* count of which exclusive flag is being processed */
	char *cntrl ;	/* control string for scanf's */
	char junk[2];		/* junk buffer for scanf's */

	cntrl = "% %1s";	/* control string initialization for scanf's */
	check = numnum = numstr = 0;
	cp = format;
	while(*cp)
	  {
		required = NO;
		switch(*(cp++))
		{
		  default:	/* all other chars */
			break;
		  case '!':	/* required argument */
			required = YES;
		  case '%':	/* not required argument */
			switch(tmpflg = *(cp++))
			{
			  case '-':	/* argument is flag */
					/* go back to label */
				cp -= 3;
				for( excnt = exflag = 0 
					; *cp != ' ' ; (--cp, excnt++) )
				  {
					for(cnt = 1 ; cnt < argc ; cnt++ )
					  {
						/* flags all start with - */
						if(*argv[cnt] == '-')
						    if(*(argv[cnt]+1) == *cp)
							  {
								if(exflag)
								    ERROR(more than one exclusive flag chosen)
								exflag++;
								required = NO;
								check += cnt;
								**arglist |= 
								  (1<<excnt);
								break;
							  }
					  }
				  }
				if(required)
					ERROR(flag argument missing)
				while(*++cp != ' ') ;
				while(*cp == ' ') cp++;
				arglist++;
				break;
			  case 's':	/* char string */
			  case 'd':	/* decimal # */
			  case 'o':	/* octal # */
			  case 'x':	/* hexadecimal # */
			  case 'f':	/* floating # */
			  case 'D':	/* long decimal # */
			  case 'O':	/* long octal # */
			  case 'X':	/* long hexadecimal # */
			  case 'F':	/* double precision floating # */
				tmpcnt = tmpflg == 's' ? numstr : numnum;
				for(cnt = 1 ; cnt < argc ; cnt++ )
				  {
					if(tmpflg == 's') /* string */
					  {
						if((c= *argv[cnt]) == '-')
							continue;
						if(c >= '0' && c <= '9')
							continue;
						if(tmpcnt-- != 0)
							continue;
						**arglist = argv[cnt];
						check += cnt;
						numstr++;
						required = NO;
						break;
					  }
					if( (*argv[cnt] == '-')
						&& !isdigit(*(argv[cnt]+1)) )
						continue;
					if( (*argv[cnt] != '-')
						&& isalpha(*argv[cnt]) )
						continue;
					if(tmpcnt-- != 0) /* skip to new one */
						continue;
						/* kludge for sscanf */
					if((tmpflg == 'o' || tmpflg == 'O')
							&& *argv[cnt] > '7' )
						ERROR(Bad numeric argument)
					cntrl[1] = tmpflg;/* put in conversion*/
					if( sscanf(argv[cnt], cntrl , *arglist
							, junk ) != 1)
						ERROR(Bad numeric argument)
					check += cnt;
					numnum++;
					required = NO;
					break;
				  }
				if(required)
					switch(tmpflg)
					{
			  		  case 'x':
			  		  case 'X':
					     ERROR(missing hexadecimal argument)
					  case 's':
						ERROR(missing string argument)
					  case 'o':
			  		  case 'O':
						ERROR(missing octal argument)
					  case 'd':
			  		  case 'D':
						ERROR(missing decimal argument)
			  		  case 'f':
					  case 'F':
						ERROR(missing floating argument)
					}
				arglist++;
				while(*cp == ' ') cp++;
				break;
			  default:	/* error */
				fprintf(stderr, "error in call to scanargs\n");
				return(0);
			}
		}
	  }
		/* sum from 1 to N = n*(n+1)/2 used to count up checks */
	if(check != (((argc-1) * argc) / 2))
		ERROR(extra arguments not processed)
	return(1);

	error:
		fprintf(stderr, "usage : ");
		if(*(cp = format) != ' ')
			while(putc(*cp++, stderr) != ' ');
		else
			fprintf(stderr, "?? ");
		while(*cp == ' ') cp++;
		format = cp;
		required = NO;
		while(*cp)
		  {
			switch(*cp)
			{
			  default:
				cp++;
				break;
			  case '!':
				required = YES;
			  case '%':
				switch(*++cp)
				{
				  case '-':	/* flags */
					if(!required)
					  {
						putc('[', stderr); putc('-', stderr);
					  }
					else 
					  {
						putc('-', stderr); putc('{', stderr);
					  }
					while(*--cp != ' ') ;
					cp++;
					while(*cp != '%' && *cp != '!')
						putc(*cp++, stderr);
					if(!required)
						putc(']', stderr);
					else 
						putc('}', stderr);
					while(*cp != ' ')
						 cp++;
					break;
			  	case 's':	/* char string */
			  	case 'd':	/* decimal # */
			  	case 'o':	/* octal # */
			  	case 'x':	/* hexadecimal # */
			  	case 'f':	/* floating # */
			  	case 'D':	/* long decimal # */
			  	case 'O':	/* long octal # */
			  	case 'X':	/* long hexadecimal # */
			  	case 'F':	/*double precision floating # */
					if(!required)
						putc('[', stderr);
					for(; format < cp-1 ; format++)
						putc(*format, stderr);
					if(!required)
						putc(']', stderr);
					break;
				  default:
					break;
				}
				required = NO;
				format = ++cp;
				putc(' ', stderr);
			}
		  }
		putc('\n', stderr);
		return(0);
  }
