
#include	"dec.h"  

err(code)
/*
**  Prints error message to stderr based
**  on code and then exits.
*/
int	code;
{
	int i;

	switch (code)
	{
		case VIDERR:
			fprintf(stderr, "Volume ids do not match\n");
			fprintf(stderr, "Volume specified: %s\n", vid);
			fprintf(stderr, "Tape volume: ");
			for (i=4; i<10; i++)
				fputc(*(inptbuf+i), stderr);
			break;

		case SECERR:
			fprintf(stderr, "Volume protected - Unable to read\n");
			break;  

		case REDERR:
			fprintf(stderr, "Error Code %d - Unable to read tape\n",
                                errno);
			break;

		case CMDERR:
			fprintf(stderr, "Command Error - Valid options: r, w, or d\n");
			break;

		case OPNERR:
			fprintf(stderr, "Error Code %d - Unable to open file\n",
				errno);
			break;

		case FLGERR:
			fprintf(stderr, "Required input parameters missing:\n");
			if (!(flags & 1))
				fprintf(stderr, "\tdev=\n");
			if (!(flags & 2))
				fprintf(stderr, "\ttf=ansi or tf=stdl\n");
			if (!(flags & 4))
				fprintf(stderr, "\tvol=\n");
			break;

		case EOVERR:
			fprintf(stderr, "Error - Multiple volume file\n");
			break;

		case FPRERR:
			fprintf(stderr, "File protected - Unable to read\n");
			break;

		case CLOERR:
			fprintf(stderr,"Error Code %d - Unable to close file\n",
                           	errno);
			break;

		case TECERR:
			fprintf(stderr, "Tape recording technique error\n");
			break;

		case FCLERR:
			fprintf(stderr, "Error - Unable to close output file\n");
			break;

		case FOPERR:
			fprintf(stderr,"Error - Output filename cannot be accessed\n");
			break;

		case VOLERR:
			if (ansi)
				fprintf(stderr, "Error - Not ansi label tape\n");
			else if (stdl)
				fprintf(stderr, "Error - Not standard label tape\n");
			break;
                case WRTERR:
                        fprintf(stderr, "Error code %d - Unable to write tape\n", errno);
                        break;
		case FMTERR:
			fprintf(stderr, "Error - Invalid record format specified\n");
			break;

		case RECERR:
			fprintf(stderr, "Error - Variable length records require blklen and reclen\n");
			break;

		case MAXERR:
			fprintf(stderr, "Error - Blklen must be less than %d\n", MAXBUF);
			break;

		case BLKERR:
			switch (recode)
			{
				case F:
				case V:
					fprintf(stderr, "Error - Blklen must equal reclen\n");
					break;

				case FB:
					fprintf(stderr, "Error - Blklen must be a multiple of reclen\n");
					break;

				case VB:
					fprintf(stderr, "Error - Blklen must be four more than a multiple of reclen\n");
					break;
	
			}

		case FNMERR:
			fprintf(stderr, "Error - Option fn must be specified with 'w' option\n");
			break;
  
		case RFMERR:
			fprintf(stderr, "Error - Invalid record format specified for ansi label tape\n");
			break;

	}
	fprintf(stderr, "\n");
	exit(0);
}
