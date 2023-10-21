#include	<stdio.h>
#include        "vmstape.h"

/* NOTE -- FILES MUST BE TEXTUAL (NOT CONTAINING A NULL BYTE) */

/* arguments to open system call
 */
#define	READ	0
#define	WRITE	1
#define RDWT	2

#define	CREATE	1
#define	LIST	2
#define	EXTRACT 3
#define APPEND  4

int open_file();

main(argc, argv)
	int     argc;
	char    **argv;
{
	register        char    *p;
			int     function;
			int     func_count;
			int     mode;
			char	device[100];

	if(argc < 2) {
		fprintf(stderr, "Usage (H = HELP): vmstape [crtxvfdbFRVH] [ filename ]\n");
		exit(FAILURE);
		}

	strcpy(device, MAGTAPE);
	strcpy(vol_label, VOL_LABEL);
	blocksz = BLOCKSZ ; /* default */
	fixreclen = FIXRECLEN; /* default */
	func_count = 0;
	for(p = argv[1]; *p != '\0' ; p++)
		switch(*p) {
			case 'H':
				fprintf(stdout,"Help option specified. All others ignored.\n");
				fprintf(stdout,"OPTIONS: c = create tape\n");
				fprintf(stdout,"         r = append to tape\n");
				fprintf(stdout,"         t = list tape\n");
				fprintf(stdout,"         x = extract from tape specified file name\n");
				fprintf(stdout,"         v = verbose messages\n");
				fprintf(stdout,"         f = specify new device\n");
				fprintf(stdout,"         d = if appending, make new section\n");
				fprintf(stdout,"         b = specify new blocksize.(default:2048b/block.)\n");
				fprintf(stdout,"         F = write fixed length records (non text files).\n");
				fprintf(stdout,"         R = specify new fixed record length.(default:128b/rec.)\n");
				fprintf(stdout,"         V = specify new volume label\n");
				fprintf(stdout,"         H = see this help screen\n");
				exit(FAILURE);
			case 'R':
		    		fixreclen = atoi(argv[2]);
				if (fixreclen > MAXFIXRECLEN) 
				{
				  fprintf(stderr,"ERROR:Fixed record length cannot be greater than %d\n",MAXFIXRECLEN);
				  exit(FAILURE);
				}
			        argv++;argc--;
				/* Fall through to writing fixed length recs */;
		    	case 'F':
				fixed_length_flag = 1;
				break;
			case 'b':
		    		blocksz = atoi(argv[2]);
				if (blocksz > MAXBLOCKSZ) 
				{
				  fprintf(stderr,"ERROR:Blocksz cannot be greater than %d\n",MAXBLOCKSZ);
				  exit(FAILURE);
				}
			        argv++;argc--;
				break;
			case 'c':
				function = CREATE;
				func_count++;
				mode = WRITE;
				break;

			case 't':
				function = LIST;
				func_count++;
				mode = READ;
				break;

			case 'x':
				function = EXTRACT;
				func_count++;
				mode = READ;
				break;

			case 'r':
				function = APPEND;
				func_count++;
				mode = RDWT;
				break;

		    	case 'd':
			/* appending , but starting the file offset count
			all over again so that a VMS dir command will split
			the tape directory into sections. */
				section_flag = 1;
				break;
			case 'f':
				strcpy(device, argv[2]);
				argv++ ; argc-- ; 
				break;

#ifdef OLD
/* could use these for writing tapes.
 */
			case 'B':
				binflag = TRUE;
				break;

			case 'R':
				rawflag = TRUE;
				break;
#endif

			case 'V':
				strcpy(vol_label, argv[2]);
				argv++ ; argc--;
				break;

			case 'v':
				verbose = TRUE;
				break;

			
			default:
				printf("Unknown key: %c\n", *p);
				exit(FAILURE);
		}

	if (blocksz % fixreclen ) {
		fprintf(stderr,"ERROR:The blocksize must be a multiple of the fixed record length.\n");
		exit (FAILURE);
		}
	if(func_count == 0) {
		fprintf(stderr,"No function specified\n");
		exit(FAILURE);
		}
	if(func_count > 1) {
		fprintf(stderr,"Too many functions specified\n");
		exit(FAILURE);
		}
#ifdef OLD
	magtape = open(device, mode);
	if(magtape < 0) {
		fprintf(stderr, "Cannot open %s\n", device);
		exit(FAILURE);
		}
#endif
	switch(function) {
		case CREATE:
			vt_create(&argv[2], device, mode);
			break;

		case LIST:
			vt_list(device, mode);
			break;

		case EXTRACT:
			vt_extract(&argv[2], device, mode);
			break;

		case APPEND:
			vt_append(&argv[2], device, mode);
			 break;

		default:
			printf("This can never happen!\n");
			break;
		}

	close(magtape);
	exit(SUCCESS);
}

make_pad_record()
{
int i;
for (i=0;i<MAXFIXRECLEN;i++) pad_record[i] = '^';
}

int open_file (device, mode)

char device[];
int mode;

{
	int descrip;

	descrip = open(device, mode);
	if(descrip < 0) {
		fprintf(stderr, "Cannot open %s\n", device);
		exit(FAILURE);
	}
	return (descrip);
}
