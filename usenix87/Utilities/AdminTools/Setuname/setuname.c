#include <a.out.h>
#include <stdio.h>
#include <errno.h>

#define EQUALS(a,b)	(strncmp(a,b,9)==0)

struct utsname {
	char	sysname[9];
	char	nodename[9];
	char	release[9];
	char	version[9];
	char	machine[9];
} utsname[3];

char *namelist = "/unix";
char *kmem = "/dev/kmem";

struct nlist nl[] = {{ "utsname" , 0, 0, 0, 0, 0 },
		     { "", 0, 0, 0, 0, 0}};

extern int optind;
extern char *optarg;
extern int opterr;

int Sflag, Nflag, Rflag, Vflag, Mflag, Uflag, Kflag, kflag, Wflag;

main(argc,argv)
char **argv;
{
	int c;
	int i, j, k, l;
	struct filehdr fh;
	struct scnhdr sh;
	int fdn, fdk;

	++opterr; 		/* I wanna do it my self */
	/*
	 * Parse the args
	 */
	Wflag = 0;
	while ((c=getopt(argc, argv, "s:n:r:v:m:u:kKf")) != EOF) 
		switch(c) {
		case 's':
			strncpy(utsname[0].sysname, optarg, 8);
			Sflag++;
			Wflag++;
			break;
		case 'n':
			strncpy(utsname[0].nodename, optarg, 8);
			Nflag++;
			Wflag++;
			break;
		case 'r':
			strncpy(utsname[0].release, optarg, 8);
			Rflag++;
			Wflag++;
			break;
		case 'v':
			strncpy(utsname[0].version, optarg, 8);
			Vflag++;
			Wflag++;
			break;
		case 'm':
			strncpy(utsname[0].machine, optarg, 8);
			Mflag++;
			Wflag++;
			break;
		case 'u':
			namelist = optarg;
			Uflag++;
			break;
		case 'K':
			Kflag++;
			kflag++;
			break;
		case 'k':
			kflag++;
			break;
		case '?':
			usage();
		}
	/*
	 * Any extra args?
	 +/
	if (optind < argc) {
		fprintf(stderr, "setuname: Too many arguments\n");
		usage();
	}
	/*
	 * Read symbol table from /unix
	 */
	if (nlist(namelist, nl) == -1)
		error("setuname: trouble reading symboltable in \"%s\"\n", namelist);
	/*
	 * Must be a non-zero value
	 */
	if (nl[0].n_value == 0) {
		fprintf(stderr, "setuname: can't find utsname structure in %s's symbol table\n",namelist);
		exit(1);
	}
	if (Kflag || !Wflag)		/* Don't write of nothing to write */
		fdn = open(namelist, 0);	/* Read only */
	else
		fdn = open(namelist, 2);	/* read/write */
	if (fdn < 0)
		error("setuname: trouble opening \"%s\"\n", namelist);
	if (Kflag || kflag) {
		if(Wflag)
			fdk = open(kmem, 2);
		else
			fdk = open(kmem, 0);
		if (fdk < 0)
			error("setuname: trouble opening \"%s\"\n", kmem);
	}
	/*
	 * Seek to spot in /unix where will find it
	 */
	if ((read(fdn, &fh, sizeof(struct filehdr)) != sizeof(struct filehdr)) ||
		(lseek(fdn, (long)(fh.f_opthdr + (nl[0].n_scnum-1)*sizeof(struct scnhdr)), 1) == -1) ||
		(read(fdn, &sh, sizeof(struct scnhdr)) != sizeof(struct scnhdr)) ||
		(lseek(fdn, (long)(sh.s_scnptr + nl[0].n_value - sh.s_vaddr), 0) == -1) ||
		(read(fdn, &utsname[1], sizeof(struct utsname)) != sizeof(struct utsname)))
			error("setuname: trouble getting utsname struct from \"%s\"\n", namelist);
	printf("%s:\n\t%.8s %.8s %.8s %.8s %.8s\n", namelist, utsname[1].sysname, utsname[1].nodename,
			utsname[1].release, utsname[1].version, utsname[1].machine);
	if(!Kflag && Wflag) {
		if(Nflag) {
			strncpy(utsname[1].nodename, utsname[0].nodename, 8);
			utsname[1].nodename[8] = '\0';
		}
		if(Vflag) {
			strncpy(utsname[1].version, utsname[0].version, 8);
			utsname[1].version[8] = '\0';
		}
		if(Mflag) {
			strncpy(utsname[1].machine, utsname[0].machine, 8);
			utsname[1].machine[8] = '\0';
		}
		if(Rflag) {
			strncpy(utsname[1].release, utsname[0].release, 8);
			utsname[1].release[8] = '\0';
		}
		if(Sflag) {
			strncpy(utsname[1].sysname, utsname[0].sysname, 8);
			utsname[1].sysname[8] = '\0';
		}
		/*
		 * Backup so we can write it
		 */
		lseek(fdn, (long)(-sizeof(struct utsname)), 1);
		if(write(fdn, &utsname[1], sizeof(struct utsname)) != sizeof(struct utsname))
			error("setuname: Warning trouble writing \"%s\"\n", namelist);
		printf("new %s:\n\t%.8s %.8s %.8s %.8s %.8s\n", namelist, utsname[1].sysname, utsname[1].nodename,
			utsname[1].release, utsname[1].version, utsname[1].machine);
	}
	/*
	 * Are we gonna try to write /dev/kmem ?
	 */
	if (!kflag) exit(0);
	if ((lseek(fdk, (long)(nl[0].n_value), 0) == -1) ||
	    (read(fdk, &utsname[2], sizeof(struct utsname)) != sizeof(struct utsname)))
		error("setuname: trouble getting utsname struct from \"%s\"\n", kmem);
	/*
	 * Backup so we can write it
	 */
	lseek(fdk, (long)(-sizeof(struct utsname)), 1);
	if(uname(&utsname[1]) < 0)
		error("setuname: can't do uname() system call\n",0);
	if(!(EQUALS(utsname[1].sysname, utsname[2].sysname) &&
	     EQUALS(utsname[1].machine, utsname[2].machine) &&
	     EQUALS(utsname[1].release, utsname[2].release) &&
	     EQUALS(utsname[1].version, utsname[2].version) &&
	     EQUALS(utsname[1].nodename, utsname[2].nodename)))
		error("setuname: %s is not the currently running kernel\n", namelist);
	printf("%s:\n\t%.8s %.8s %.8s %.8s %.8s\n", kmem, utsname[2].sysname, utsname[2].nodename,
			utsname[2].release, utsname[2].version, utsname[2].machine);
	if(!Wflag) exit(0);
	if(Nflag) {
		strncpy(utsname[2].nodename, utsname[0].nodename, 8);
		utsname[2].nodename[8] = '\0';
	}
	if(Vflag) {
		strncpy(utsname[2].version, utsname[0].version, 8);
		utsname[2].version[8] = '\0';
	}
	if(Mflag) {
		strncpy(utsname[2].machine, utsname[0].machine, 8);
		utsname[2].machine[8] = '\0';
	}
	if(Rflag) {
		strncpy(utsname[2].release, utsname[0].release, 8);
		utsname[2].release[8] = '\0';
	}
	if(Sflag) {
		strncpy(utsname[2].sysname, utsname[0].sysname, 8);
		utsname[2].sysname[8] = '\0';
	}
	if(write(fdk, &utsname[2], sizeof(struct utsname)) != sizeof(struct utsname))
		error("setuname: Warning trouble writing \"%s\"\n", kmem);
	printf("new %s:\n\t%.8s %.8s %.8s %.8s %.8s\n", kmem, utsname[2].sysname, utsname[2].nodename,
		utsname[2].release, utsname[2].version, utsname[2].machine);
}

usage()
{
	fprintf(stderr, "usage: setuname [-s sysname] [-n nodename] [-r release]\n\t%s",
		"[-v version] [-m machine] [-u unixfile] [-f] [-k | -K]\n");
	exit(1);
}

error(s1,s2)
char *s1, *s2;
{
	fprintf(stderr, s1, s2);
	if(errno)
		perror("setuname");
	exit(1);
}
