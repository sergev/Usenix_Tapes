#include <stdio.h>

#define DEFSIZE	10000		/* Default size for output files */
#define MINSIZE	 1000		/* minimum size for output files */
#define	LINELEN	  512		/* maximum line length in input file */

#define SHAR_HEADER	": This is a shar archive.\n: Remove everything above this line.\n: Run the file through sh, not csh.\n: (type `sh %s')\n: If you do not see the message\n:\t`%s completed!'\n: then the file was incomplete.\n"
#define SHAR_COMMAND	"echo extracting - %s\nsed 's/^%s//' > %s << '%s'\n"
#define SHAR_BEGIN	"X"
#define SHAR_EOF	"FRIDAY_NIGHT"
#define SHAR_TRAILER	"echo %s completed!\n: That's all folks!\n"

main(argc, argv)
int	argc;
char  **argv;
{
int	maxsize = DEFSIZE;
int	cursize = 0;
int	level   = 1;
char	outname[40], line[LINELEN], outline[LINELEN];
char	c, *cp, *suffix;
FILE	*fin = NULL, *fout = stdout;

	outname[0] = NULL;

	while(--argc){
		
		cp = *++argv;

		if(*cp == '-'){	/* Process option flags */
			while(*++cp){
				switch(*cp){
				    case 'm':	/* reset maxsize */
					maxsize = atoi(*++argv);
					if(maxsize < MINSIZE){
						fprintf(stderr,"maxsize too small!  using default!\n");
						maxsize = DEFSIZE;
						}
					break;

				    case 'o':	/* Start a new output file */
					strcpy(outname, *++argv);
					for(suffix=outname; *suffix; ++suffix);
					*suffix++ = '.';
					if(fout && fout != stdout){
						fprintf(fout, SHAR_TRAILER, outname);
						fclose(fout);
						}
					fout = NULL;
					break;

				    default:
					fprintf(stderr, "usage: shar [-o outfile] [-m maxsize] [file1...]\n");
					exit(1);
				    }
				}

			continue;
			}

		/* Assume it's a file to be shar'ed */

		/* Check to see if we are over our limit */

		if(fout == stdout){
			if(cursize == 0)
				fprintf(fout, SHAR_HEADER, "this_file", "this_file");
			}
		else if(cursize >= maxsize || fout == NULL){
			if(fout){
				fprintf(fout, SHAR_TRAILER, outname);
				fclose(fout);
				}

			sprintf(suffix, "%d", level++);
Retry_file:
			if((fout=fopen(outname,"r")) != NULL){
				fprintf(stderr,"WARNING:  File `%s' already exists!\nShould I overwrite it? [n]  ", outname);
				fgets(line, sizeof(line), stdin);
				if(*line == 'y' || *line == 'Y')
					goto Open_file;

				sprintf(suffix, "%d", level++);
				fprintf(stderr,"Should I try `%s'? [n]  ", outname);
				fgets(line, sizeof(line), stdin);
				if(*line == 'y' || *line == 'Y')
					goto Retry_file;
				exit(2);
				}
Open_file:
			if((fout=fopen(outname, "w")) == NULL){
				fprintf(stderr, "Can't create output file `%s'!\n", outname);
				sprintf(suffix, "%d", level++);
				fprintf(stderr,"Should I try `%s'? [n]  ", outname);
				if(*line == 'y' || *line == 'Y')
					goto Retry_file;
				exit(3);
				}

			fprintf(stderr, "starting new output file `%s'.\n", outname);
			fprintf(fout, SHAR_HEADER, outname, outname);

			cursize = 0;
			}

		if((fin=fopen(*argv, "r")) == NULL){
			fprintf(stderr,"WARNING:  Can't read input file `%s'!\n", *argv);
			continue;
			}

		fprintf(fout, SHAR_COMMAND, *argv, SHAR_BEGIN, *argv, SHAR_EOF);

		while(fgets(line, sizeof(line), fin) != NULL){
			sprintf(outline, "%s%s", SHAR_BEGIN, line);
			for(cp=outline; *cp; ++cp,++cursize);
			fprintf(fout, "%s", outline);
			}

		fprintf(fout, "%s\n", SHAR_EOF);

		fclose(fin);
		}
}
