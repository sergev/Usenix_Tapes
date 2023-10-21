/*
**   Cgrep - a program to extract context from text files.
**	by David J. Iannucci @ Saint Joseph's University, Philadelphia
**						  iannucci@sjuvax.UUCP
**
**			Copyright (c) 1986
**
**	You may do anything you like with this program except:
**		1.  Use it to direct commercial advantage.
**		2.  Take credit for any code I have written.
**
**	Any changes made to this program must be documented so that I do not
**	receive credit or blame for having made them.
**
**
*/

#include <stdio.h>
#include <ctype.h>
#define	FALSE	0
#define	TRUE	1
#define	LINES	1
#define	PARA	2
#define	PATN	3
#define LOOKBACK	100	/* Number of lines of look-back */
#define MAXLINE		256 	/* Maximum length of a line */

char *strsave(), *progname;
int ignorecase=FALSE;
int patgotten=FALSE;
int filesread=0;
FILE *file;

main(argc, argv)
int argc;
char *argv[];
{

	char *buf[LOOKBACK], aline[MAXLINE+1];
	char pat[MAXLINE], patn_f[MAXLINE], patn_b[MAXLINE];
	int lines_f=0, lines_b=0, schback=0, schforw=0;
	int i=0, j=0, readsofar=0, lastone=(-1), mmode=FALSE;

	progname = *argv;
	file = stdin;

if ( argc < 2 )
{
    fprintf(stderr, 

"Usage: %s [-mi] [-n -p -/pat] [+n +p +/pat] pattern [ files... ]\n",

						progname);
						exit(1);
}

	while ( --argc > 0 )	{	/* Parse the command line */
		switch(argv[1][0])	{
			case '+': if ( strlen(argv[1]) == 1 )	{
					lines_f=1;
					schforw = LINES;
				  }
				  else switch (argv[1][1])   {
					  case 'p': schforw = PARA;
						    break;
					  case '/':
						strcpy(patn_f, argv[1]+2);
						schforw = PATN;
						break;
					  default:
						sscanf(argv[1],"+%d",&lines_f);
						schforw = LINES;
						break;
					}
				   break;

			case '-':  if ( strlen(argv[1]) == 1 )	{
					lines_b=1;
					schback = LINES;
				   }
				   else switch (argv[1][1])    {
					   case 'm': mmode=TRUE; break;
					   case 'p': schback = PARA; break;
					   case 'i': ignorecase=TRUE;break;
					   case '/': 
						strcpy(patn_b, argv[1]+2);
						schback = PATN;
						break;
					   default: 
					      sscanf(argv[1], "-%d", &lines_b);
					      schback = LINES;
					      lines_b=((lines_b > LOOKBACK) ?
							LOOKBACK : lines_b);
					      break;
					}
				        break;

			default:  if ( !strcmp(pat, "") )	{
					strcpy(pat, argv[1]);
					patgotten=TRUE;
					--argc;
				  }
				  break;
		}
	++argv;
	if ( patgotten )
		break;
	}

	/* end of command line parsing */

      do
      {
	if ( argc-- != 0 )
	{
		i=0;
		readsofar=0;
		++filesread;
		if ((file=fopen(argv[filesread], "r")) == NULL )
		{
		fprintf(stderr, "%s: cannot open %s\n", progname,
							     argv[filesread]);
		exit(1);
		}
	}

           while  ( fgets(aline, MAXLINE, file) != NULL )
	   {
		buf[i++%LOOKBACK] = strsave(aline);
		readsofar++;
		if ( schback == PARA )
			   if ( emptyline(aline) )
				   lastone = i-1;
		if ( schback == PATN )
			   if ( found(aline, patn_b) )
				   lastone = i-2;
		if ( found(aline, pat) )		{
			if ( filesread > 1 )
				printf("<%s>\n", argv[filesread]);
			if ( schback == PARA || schback == PATN )
				lines_b=((lastone == -1) ? 0 : i-2-lastone);
			lines_b=(( lines_b >= readsofar) ? i-1 : lines_b);
			for (j=(--i-lines_b); j <= i; j++)
				output(mmode, buf[nmod(j, LOOKBACK)]);
			j=0;
			if ( schforw )
			   while ( fgets(aline, MAXLINE, file) != NULL )  {
				if ( ( (schforw==LINES) && ++j > lines_f ) || 
				     ( (schforw==PARA) && emptyline(aline) ) )
					exit(0);
				output(mmode, aline);
				if ( schforw==PATN )
					if ( found(aline, patn_f) )
						exit(0);
			   }
			   exit(0);
		}
           }
	   if ( file != stdin )
	   	fclose(file);

      } while ( argc > 0 );

      fprintf(stderr, "%s: %s not found.\n", progname, pat);
      exit(1);

}

found(s,  t)	/* Almost the same as K&R's "index" function */
char *s, *t;
{
	int i, j ,k;

	for (i=0; s[i] != '\0'; i++)	{
		for (j=i, k=0; t[k] != '\0' && match(s[j], t[k]); j++, k++)
			;
		if ( t[k] == '\0' )
			return(TRUE);
	}
	return(FALSE);
}

output(mailmode, line)
int mailmode;
char *line;
{
	if ( mailmode )		{
		while ( *line == ' ' || *line == '\t' )
			line++;
		printf("> %s", line);
	}
	else printf("%s", line);
}

emptyline(s)
char *s;
{
	while ( *s )	{
		if ( *s != ' ' && *s != '\t' && *s != '\n' )
			return(FALSE);
		++s;
	}
	return(TRUE);

}

nmod(a, m)
int a, m;
{
	if ( a < 0 )
		return(m + (a % m));
	else
		return(a % m);
}

match(c, d)
int c, d;
{
	if ( ignorecase )	{
		if ( isupper(c) && islower(d) )
			return(c==toupper(d));
		else if ( islower(c) && isupper(d) )
			return(c==tolower(d));
		else return(c==d);
	}
	else
		return(c==d);
}

char *strsave(s)
char *s;
{
	char *p;

	if ((p = (char *) malloc((strlen(s)+1)*sizeof(char))) == (char *)0 )
	{
		fprintf(stderr,
		"%s: Malloc failed in %s:%d\n", progname,__FILE__, __LINE__);
		exit(2);
	}
	strcpy(p, s);
	return p;
}
