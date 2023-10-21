#
#define BOOTSZ 512
#define ROLLSZ 10240
#define REELSZ 512
char *argv0;
int infile;
int outtap;
int records;
int bootfd ;
char not1st 0;  /* Flag set when not first file on tape */
int tvec[2];
int *tarray;
int label[7] {
0102610,        /* UNX in RAD50 */
0000000,        /* RAD50 device name is filled in here */
0071344,        /* extention ROL in RAD50 */
0000401,        /* UIC 1,1 */
0000233,        /* Protection code */
8364,           /* Julian date from 1970 */
0000000,
};
char reel[REELSZ];
char boot[BOOTSZ];
char rollin[ROLLSZ];
char buff[8192];

main(argc, argv)
int argc;
char *argv[];
{
	register int count;
	register char **args;
	int *localtime();

	args = argv;
	argv0 = *args++;
	if(--argc <= 0) {
		printf("%s writes ROLLIN format files on /dev/nrw_rmt0\n",
			argv0);
		printf("Use - if other files are already on tape\n");
		printf("and position the tape to the end before writing.\n");
		printf("%s [ - ] special_file . . .\n", argv0);
		exit(0);
	}
	if((bootfd = open("/usr/mdec/rollboot", 0)) < 0
		|| seek(bootfd, 020, 0) == -1) {
		printf("%s boot open error\n", argv0);
		exit(1);
	}
	if(read(bootfd, boot, BOOTSZ) != BOOTSZ) {
		printf("%s boot read error\n", argv0);
		exit(1);
	}
	if(read(bootfd, rollin, ROLLSZ) != ROLLSZ) {
		printf("%s rollin read error\n", argv0);
		exit(1);
	} else close(bootfd);
	reel[0] = 1;
	time(tvec);
	tarray = localtime(tvec);
	/* Convert to DOS format time */
	label[5] = tarray[7] + ((tarray[5] - 70) * 1000);

	while(argc-- > 0) if(*args[0] == '-') {
		not1st++;
		args++;
	} else {
		if((infile = open(*args, 0)) < 0) {
			printf("%s cannot open %s\n", argv0, *args);
			exit(1);
		}
		makename(*args++);
		if((outtap = open("/dev/nrw_rmt0", 2)) < 0) {
			printf("%s tape open error\n", argv0);
			exit(1);
		}
		if(write(outtap, label, 14) != 14) {
			printf("%s error writing %s tape label\n",
				argv0, *--args);
			exit(1);
		}
		if(not1st == 0) {
			not1st++;
			if(write(outtap, boot, BOOTSZ) != BOOTSZ) {
				printf("%s boot write error\n", argv0);
				exit(1);
			}
			if(write(outtap, rollin, ROLLSZ) != ROLLSZ) {
				printf("%s rollin write error\n", argv0);
				exit(1);
			}
		}
		if(write(outtap, reel, REELSZ) != REELSZ) {
			printf("%s reel label write error\n", argv0);
			exit(1);
		}
		count = sizeof buff;
		records = 304;
		while(count = read(infile, buff, count)) {
			if(count != sizeof buff) {
				if(records > 0 || count != sizeof buff / 2) {
					printf("%s error reading %s\n",
						argv0, *--args);
					exit(1);
				}
			}
			if(write(outtap, buff, count) != count) {
				printf("%s error writing on tape\n", argv0);
				exit(1);
			}
			if(--records == 0) count = sizeof buff / 2;
			else if(records < 0) break;
		}
		close(outtap);
		printf("\n");
	}
	flush();
}

int ator50(numb)
int numb;
{
	register int num;

	num = numb;
	if('0' <= num && num <= '9') return(num - '0' + 036);
	if('a' <= num && num <= 'z') return(num - 'a' + 1);
	if('A' <= num && num <= 'Z') return(num - 'A' + 1);
	if(num == '$') return(033);
	return(0);
}

makename(name)
char *name;
{
	register char *nam, *indx;
	static char nbuff[6];

	nam = name;
	indx = nbuff;
	while(indx < &nbuff[3]) *indx++ = 0;
	while(*nam++);
	while(nam-- != name) {
		if(('0' <= *nam && *nam <= '9')
		   || ('A' <= *nam && *nam <= 'Z')) *--indx = *nam;
		else
		if('a' <= *nam && *nam <= 'z') *--indx = *nam - 'a' + 'A';
		if(indx == nbuff) break;
	}
	printf("%s = UNX%s.ROL: ", name, indx);
	/* fill in second word of file name in tape label */
	label[1] = (((ator50(indx[0]) * 050)
		    + ator50(indx[1])) * 050) + ator50(indx[2]);
}
