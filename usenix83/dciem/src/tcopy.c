/*
 * tcopy - absolute tape copy
 *
 * Usage: tcopy [-adtc] [infile [outfile]]
 *
 * Written by M.M.Taylor, DCIEM, Toronto, 2/May/79
 *
 * Modified for V7 by Alexis Kwan (HCR for DCIEM), 17/Feb/81
 *
 * KD1-Mar 1 82  Kim Davidson (HCR) for DCIEM
 *	added c option to allow byte by byte comparing of
 *	two tapes or a tape and disk files using -dc option
 */
/*% cc -O tcopy.c -o tcopy
 */

#include <stdio.h>
#include <signal.h>

/*
 * Copies a tape to disk, file by file. Consecutive files will have
 * names "firstfile.0", "firstfile.1" .... "firstfile.n"
 */

#define BIG 24*1024
#define READ 0
#define WRITE 1
#define ERROR -1

int ibytes, obytes;
int ifd;
int ofd;
FILE *outf;
char buffer[BIG];	/* This must be aligned */
char cbuffer[BIG];	/* KD1 */
int aflag;
int tflag;
int dflag;
int cflag;		/* KD1 */

int i;			/* KD1 */
char *bp, *cbp;		/* KD1 */

int endflag, nfiles;
int lastrec, nrecs;

char stape[80] = "/dev/nrmt0";
char dtape[80] = "/dev/nrmt1";
int dnumpos;

quit ()
{
	printf("%d records counted: there may be more\n",nrecs);
	restore();
	exit(0);
}

main (argc, argv)
	int argc;
	char *argv[];
{
	register char *ap;

	signal(SIGINT,quit);
	--argc;
	argv++;
	if (argc && **argv=='-') {
		for (ap = *argv; *++ap; ) {
			switch (*ap) {
			case 'a':
				aflag++;
				break;
			case 't':
				tflag++;
				break;
			case 'd':
				dflag++;
				break;
/*KD1 begin*/
			case 'c':
				cflag++;
				break;
/*KD1 end*/
			default:
				fprintf(stderr,"Unknown flag %c\n",*ap);
				exit(1);
			}
		}
		--argc;
		argv++;
	}
	if ((dflag && aflag) || (dflag && argc!=2) || argc>2) {
		fprintf(stderr,"Usage: tcopy -t [infile]\n");
		fprintf(stderr," or    tcopy -d infile outfile\n");
		fprintf(stderr," or    tcopy [-a] [infile [outfile]]\n");
		exit(1);
	}
	if (argc) {
		stape[strlen(stape)-1] = argv[0][strlen(argv[0])-1];
		argv++;
		argc--;
		if (!tflag)
			if (dflag) {
				strcpy(dtape,*argv);
				dnumpos = strlen(dtape)+1;
				strcat(dtape,".0");
			}
			else
				if (argc)
					dtape[strlen(dtape)-1] = argv[0][strlen(*argv)-1];
	}
	if (!tflag)
		if (cflag)
			fprintf(stderr,"Comparing %s to %s\n",stape,dtape);
		else
			fprintf(stderr,"Copying %s to %s\n",stape,dtape);
	if ((ifd = open(stape,READ))==ERROR) {
		fprintf(stderr,"Cannot open %s\n",stape);
		exit(1);
	}
	if (aflag) {
		if ((ofd = open(dtape,READ))==ERROR) {
			fprintf(stderr,"Cannot open %s\n",dtape);
			restore();
			exit(1);
		}
		nrecs = nfiles = 0;
		do {
			while (read(ofd,buffer,BIG) > 0)
				nrecs++;
			nfiles++;
		} while (read(ofd,buffer,BIG)>0);
		fprintf(stderr,"Skipped %d records in %d files\n",nrecs,nfiles);
/*KD1 begin*/
		if (cflag)
			read(ofd,buffer,BIG);	/*One more read needed to get to start of appended files */
/*KD end*/
		close(ofd);
	}
	if (!tflag) {
		if (dflag) {
/*KD1 begin*/
			if (cflag) {
				if ((outf = fopen(dtape,"r"))==NULL) {
					fprintf(stderr,"Cannot open %s for reading\n",dtape);
					restore();
					exit(1);
				}
			}
/*KD1 end*/
			else if ((outf = fopen(dtape,"w"))==NULL) {
				fprintf(stderr,"Cannot open %s for writing\n",dtape);
				restore();
				exit(1);
			}
		}
		else {
/*KD1 begin*/
			if (cflag) {
				if ((ofd = open(dtape,READ))==ERROR) {
					fprintf(stderr,"Cannot open %s for reading\n",dtape);
					restore();
					exit(1);
				}
			}
/*KD1 end*/
			else if ((ofd = open(dtape,WRITE))==ERROR) {
				fprintf(stderr,"Cannot open %s for writing\n",dtape);
				restore();
				exit(1);
			}
		}
	}

/*
 * End of tape is two consecutive end-of-files. Since tape driver cannot
 * distinguish errors from end-of-files, two consecutive errors will also
 * end the transfer.
 *
 * Record sizes and file lengths sent to standard output
 */

	nfiles = 0;
	for (;;) {
		lastrec = 0;
		nrecs = 1;
		endflag = 1;
		while ((ibytes = read(ifd,buffer,BIG)) > 0) {
			endflag = 0;
			/* Make ibytes even and pad buffer */
			if (ibytes & 1)
				buffer[ibytes++] = '\0';
			if (ibytes==lastrec)
				nrecs++;
			else {
				if (lastrec!=0)
					printf("%d records\n",nrecs);
				lastrec = ibytes;
				nrecs = 1;
				if (lastrec!=0)
					printf("Record size %d bytes -- ",lastrec);
			}
			if (!tflag) {
				if (dflag) {
/*KD1 begin*/
					if (cflag) {
						if (fread(cbuffer,1,ibytes,outf)==NULL) {
							fprintf(stderr,"error on reading %s\n",dtape);
							restore();
							exit(1);
						}
						for (bp=buffer,cbp=cbuffer,i=0;i<ibytes;i++,bp++,cbp++) {
							if (*bp != *cbp) {
								fprintf(stderr,"error on comparing %d byte in file %s, record %d\n",i,dtape,nrecs);
								restore();
								exit(1);
							}
						}
					}
/*KD1 end*/
					else if (fwrite(buffer,1,ibytes,outf)==NULL) {
						printf("error on writing %s\n",dtape);
						restore();
						exit(1);
					}
				}
				else {
/*KD1 begin*/
					if (cflag) {
						if ((obytes=read(ofd,cbuffer,ibytes))!=ibytes) {
							fprintf(stderr,"Read error on %s\n",dtape);
							fprintf(stderr,"wanted %d, got %d\n",ibytes,obytes);
							restore();
							exit(1);
						}
						for(bp=buffer,cbp=cbuffer,i=0;i<ibytes;bp++,cbp++,i++) {
							if (*bp != *cbp) {
								fprintf(stderr,"error on comparing %d byte in file %d, record %d\n",i,nfiles,nrecs);
								restore();
								exit(1);
							}
						}
/*KD1 end*/
					}
					else if ( write(ofd,buffer,ibytes)!=ibytes) {
						fprintf(stderr,"Write error on %s\n",dtape);
						quit();
					}
				}
			}
		}
		if (endflag) {
			printf("****  End of tape  ****\n");
			if (dflag && !tflag) {
				fclose(outf);
				unlink(dtape);
			}
			restore();
			exit(0);
		}
		printf("%d records\n",nrecs);
		nfiles++;
		printf("End of file ");
		if (dflag) {
			printf("%s\n",dtape);
			if (!tflag) {
				sprintf(&dtape[dnumpos],"%d",nfiles);
				fclose(outf);
				if ((outf = fopen(dtape,"w"))==NULL) {
					fprintf(stderr,"open error on file %s\n",dtape);
					restore();
					exit(1);
				}
			}
		}
		else {
			printf("%d\n",nfiles);
/*KD1*/
			if(!tflag){
				if(cflag){
					obytes= read(ofd,cbuffer,BIG);
					if(obytes != ibytes){
						fprintf(stderr,"at EOF on infile, read returned different value on outfile; ret vals: %d, %d\n",ibytes,obytes);
						restore();
						exit(1);
					}
				}
				else{
					close(ofd);
					ofd = open(dtape,WRITE);
				}
			}
		}
	}
}

/*
 * Rewind any tape drives used.
 */

char tape[80] = "/dev/rmtX";

restore ()
{
	register rfd, tnumpos;

	if (ifd != 0)
		close(ifd);
	tnumpos = strlen(tape)-1;
	tape[tnumpos] = stape[strlen(stape)-1];
	if ((rfd = open(tape,READ))!=ERROR)
		close(rfd);
	if (!dflag && !tflag) {
		if (ofd != 0)
			close(ofd);
		tape[tnumpos] = dtape[strlen(dtape)-1];
		if ((rfd = open(tape,READ))!=ERROR)
			close(rfd);
	}
}
