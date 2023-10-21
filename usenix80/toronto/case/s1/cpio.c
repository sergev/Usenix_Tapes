#include "stdio.h"
#include "stat.h"
#define EQ(x,y)	(strcmp(x,y)==0)
#define MAGIC	070707
#define IN	1
#define OUT	2
#define PASS	3
#define HDRSIZE	((sizeof Hdr)-256)
#define INPUT	0
#define OUTPUT	1

struct	inode	Statb, Xstatb;

struct header {
	int	h_magic,
		h_dev,
		h_ino,
		h_mode,
		h_uid,
		h_gid,
		h_nlink,
		h_majmin;
	long	h_mtime;
	int	h_namesize;
	long	h_filesize;
	char	h_name[256];
} Hdr;

int	Buf[256], Wbuf[256];
int	*Wp	Wbuf,
	Wct	256;

union { int	integ; };

int	Option,
	Mod_time,
	Dir,
	Uncond,
	Link,
	Rename,
	Toc,
	Verbose,
	Select;

int	Ifile,
	Ofile;
unsigned	Blocks;

char	Fullname[256],
	Name[128];

FILE	*Rtty,
	*Wtty;

char	*Pattern;
int	Dev;
int	Uid, Gid;
int	A_directory,
	A_special;

main(argc, argv)
char **argv;
{
	register ct;
	long	filesz;
	register char *fullp, *lastarg;
	long itol();

	if(argc < 2 || argc > 4) {
usage:
		fprintf(stderr,
		"Usage: cpio -o[v] <name-list >archive\n%s\n%s\n",
		"       cpio -i[dmrtuv] [pattern] <archive",
		"       cpio -p[dlmruv] [pattern] directory <name-list");
		exit(1);
	}
	lastarg = argv[argc-1];
	if(*argv[1] != '-')
		goto usage;
	Uid = getuid() & 0377;
	Gid = getgid() & 0377;

	while(*++argv[1]) {
		switch(*argv[1]) {
		case 'i':
			Option = IN;
			Pattern = argc == 2? "*": argv[2];
			break;
		case 'o':
			Option = OUT;
			break;
		case 'p':
			if(access(lastarg, 2) == -1) {
accerr:
				fprintf(stderr, "cannot write in <%s>\n",
					lastarg);
				exit(1);
			}
			strcpy(Fullname, lastarg);
			strcat(Fullname, "/");
			stat(Fullname, &Xstatb);
			if((Xstatb.i_mode&IFMT) != IFDIR)
				goto accerr;
			Option = PASS;
			Dev = Xstatb.i_dev;
			Pattern = argc == 3? "*": argv[2];
			break;
		case 'm':
			++Mod_time;
			break;
		case 'd':
			Dir++;
			break;
		case 'r':
			Rename++;
			Rtty = fopen("/dev/tty", "r");
			Wtty = fopen("/dev/tty", "w");
			break;
		case 't':
			Toc++;
			break;
		case 'u':
			Uncond++;
			break;
		case 'l':
			Link++;
			break;
		case 'v':
			Verbose++;
			break;
		default:
			goto usage;
		}
	}
	if(!Option) {
		fprintf(stderr, "Options must include o|i|p\n");
		exit(1);
	}

	switch(Option) {

	case OUT:
		while(getname()) {
			if(Hdr.h_filesize == 0L) {
				bwrite(OUTPUT, &Hdr, HDRSIZE+Hdr.h_namesize);
				continue;
			}
			if((Ifile = open(Hdr.h_name, 0)) < 0) {
				fprintf(stderr, "<%s> ?\n", Hdr.h_name);
				continue;
			}
			bwrite(OUTPUT, &Hdr, HDRSIZE+Hdr.h_namesize);
			for(filesz=Hdr.h_filesize; filesz>0; filesz-= 512){
				ct = filesz>512? 512: filesz;
				read(Ifile, Buf, ct);
				bwrite(OUTPUT, Buf, ct);
			}
			close(Ifile);
			if(Verbose)
				fprintf(stderr, "%s\n", Hdr.h_name);
		}
		strcpy(Hdr.h_name, "TRAILER!!!");
		Hdr.h_filesize = 0L;
		Hdr.h_namesize = strlen("TRAILER!!!") + 1;
		bwrite(OUTPUT, &Hdr, 600);
		break;

	case IN:
		while(gethdr()) {
			Ofile = ckname(Hdr.h_name)? openout(Hdr.h_name): 0;
			for(filesz=Hdr.h_filesize; filesz>0; filesz-= 512){
				ct = filesz>512? 512: filesz;
				bread(ct, Wbuf);
				if(Ofile)
					write(Ofile, &Wbuf, ct);
			}
			if(Ofile) {
				close(Ofile);
/* 				set_time(Hdr.h_name, Hdr.h_mtime, Hdr.h_mtime); */
			}
			if(!Select)
				continue;
			if(Verbose)
				if(Toc)
					pentry(Hdr.h_name);
				else
					puts(Hdr.h_name);
			else if(Toc)
				puts(Hdr.h_name);
		}
		break;

	case PASS:
		fullp = Fullname + strlen(Fullname);

		while(getname()) {
			if(!ckname(Hdr.h_name))
				continue;
			strcpy(fullp, Hdr.h_name);

			if(Link
			&& !A_directory
			&& Dev == Statb.i_dev
			&& (Uid == Statb.i_uid || !Uid)) {
				unlink(Fullname);
				if(link(Hdr.h_name, Fullname) < 0) {
					fprintf(stderr,
					 "Cannot link <%s> & <%s>\n",
					 Hdr.h_name, Fullname);
					continue;
				}
/* 				set_time(Fullname, Hdr.h_mtime, Hdr.h_mtime); */
				goto ckverbose;
			}
			if(!(Ofile = openout(Fullname)))
				continue;
			if((Ifile = open(Hdr.h_name, 0)) < 0) {
				fprintf(stderr, "<%s> ?\n", Hdr.h_name);
				close(Ofile);
				continue;
			}
			filesz = itol(Statb.i_size0 & 0377, Statb.i_size1);
			for(; filesz > 0; filesz -= 512) {
				ct = filesz>512? 512: filesz;
				read(Ifile, Buf, ct);
				if(Ofile)
					write(Ofile, Buf, ct);
				++Blocks;
			}
			close(Ifile);
			if(Ofile) {
				close(Ofile);
/* 				set_time(Fullname, Statb.i_atime, 
					Statb.i_mtime); */
ckverbose:
				if(Verbose)
					puts(Fullname);
			}
		}
	}
	fprintf(stderr, "%l blocks\n", Blocks);
	exit(0);
}

getname()
{
	register char *namep  Name;

	for(;;) {
		if(gets(namep) == NULL)
			return 0;
		if(*namep == '.' && namep[1] == '/')
			strcpy(Hdr.h_name, namep+2);
		else
			strcpy(Hdr.h_name, namep);
		if(stat(Hdr.h_name, &Statb) < 0) {
			fprintf(stderr, "<%s> ?\n", Hdr.h_name);
			continue;
		}
		A_directory = (Statb.i_mode & IFMT) == IFDIR;
		A_special = Statb.i_mode & IFMT;
		Hdr.h_magic = MAGIC;
		Hdr.h_namesize = strlen(Hdr.h_name) + 1;
		Hdr.h_uid = Statb.i_uid;
		Hdr.h_gid = Statb.i_gid;
		Hdr.h_dev = Statb.i_dev;
		Hdr.h_ino = Statb.i_ino;
		Hdr.h_mode = Statb.i_mode;
		Hdr.h_mtime = Statb.i_mtime;
		Hdr.h_nlink = Statb.i_nlink;
		Hdr.h_filesize = Hdr.h_mode&IFMT? 0L:
			itol(Statb.i_size0 & 0377, Statb.i_size1);
		Hdr.h_majmin = Statb.i_addr[0];
		return 1;
	}
}

gethdr()
{

	bread(HDRSIZE, &Hdr);

	if(Hdr.h_magic != MAGIC) {
		fprintf(stderr,
		"Out of phase--get help (or try `_cpio' for old format)\n");
		exit(1);
	}
	bread(Hdr.h_namesize, &Hdr.h_name);
	if(EQ(Hdr.h_name, "TRAILER!!!"))
		return 0;
	A_directory = (Hdr.h_mode & IFMT) == IFDIR;
	A_special = Hdr.h_mode & IFMT;
	return 1;
}

ckname(namep)
register char *namep;
{
	++Select;
	if(!gmatch(namep, Pattern)) {
		Select = 0;
		return 0;
	}
	if(Rename) {
		fprintf(Wtty, "Rename <%s>\n", namep);
		fflush(Wtty);
		fgets(namep, 128, Rtty);
		if(feof(Rtty))
			exit(1);
		namep[strlen(namep) - 1] = '\0';
		if(EQ(namep, "")) {
			printf("Skipped\n");
			return 0;
		}
	}
	return !Toc;
}

openout(namep)
register char *namep;
{
	register f;

	if(A_directory) {
		if(!Dir
		|| Rename
		|| EQ(namep, ".")
		|| EQ(namep, "..")
		|| stat(namep, &Xstatb) == 0)
			return 0;

		makdir(namep);
ret:
		chmod(namep, Hdr.h_mode);
		if(Uid == 0)
			chown(namep, (Hdr.h_uid&0377) | (Hdr.h_gid<<8));
		return 0;
	}
	if(Hdr.h_nlink > 1)
		if(!postml(namep))
			return 0;
	if(A_special) {
		unlink(namep);
		if(mknod(namep, Hdr.h_mode, Hdr.h_majmin) < 0) {
			fprintf(stderr, "Cannot mknod <%s>\n", namep);
			return 0;
		}
		goto ret;
	}
	if(stat(namep, &Xstatb) == 0)
		if(!Uncond && (Hdr.h_mtime < Xstatb.i_mtime)) {
			fprintf(stderr, "current <%s> newer\n",
				namep);
			return 0;
		}
	if(Option == PASS
	&& Hdr.h_ino == Xstatb.i_ino
	&& Hdr.h_dev == Xstatb.i_dev) {
		fprintf(stderr, "Attempt to pass file to self!\n");
		exit(1);
	}
	if((f = creat(namep, 0600)) < 0) {
		if(Dir) {
			register char *np;
			register v;

			for(np = namep; *np; ++np) {
				if(*np == '/') {
					*np = '\0';
					if((v=stat(namep, &Xstatb))== -1) {
						makdir(namep);
						*np = '/';
					} else if((Xstatb.i_mode&IFMT)
						!= IFDIR) {
						*np = '/';
						break;
					}
					*np = '/';
				}
			}
		}
		if((f = creat(namep, 0600)) < 0) {
			fprintf(stderr, "Cannot create <%s>\n", namep);
			return 0;
		}
	}
	if(Uid == 0)
		chown(namep, (Hdr.h_uid&0377) | (Hdr.h_gid<<8));
	chmod(namep, Hdr.h_mode); /* move til after copy */
	return f;
}

bread(fct, b)
register fct, *b;
{
	static nleft;
	static *ip;

	fct = (fct+1)>>1;
	while(fct--) {
		if(!nleft) {
			if(read(INPUT, Buf, 512)!=512) {
				fprintf(stderr, "Read error\n");
				exit(1);
			}
			nleft = 256;
			ip = Buf;
			++Blocks;
		}
		*b++ = *ip++;
		--nleft;
	}
}

bwrite(f, rp, c)
register *rp;
register c;
{
	c = (c+1) >> 1;
	while(c--) {
		if(!Wct) {
			if(write(f, Wbuf, 512)<0) {
				fprintf(stderr, "Cannot write.\n");
				exit(1);
			}
			Wct = 256;
			Wp = Wbuf;
			++Blocks;
		}
		*Wp++ = *rp++;
		--Wct;
	}
}

postml(namep)
register char *namep;
{
	register i;

	static struct ml {
		int	m_dev,
			m_ino;
		char	m_name[2];
	} *ml[250];
	
	static	mlinks 0;

	for(i = 0; i < mlinks; ++i) {
		if(mlinks == 250) break;
		if(ml[i]->m_ino==Hdr.h_ino &&
			ml[i]->m_dev==Hdr.h_dev) {
			if(Verbose)
			  printf("%s linked to %s\n", ml[i]->m_name,
				Hdr.h_name);
			unlink(namep);
			if(link(ml[i]->m_name, namep) < 0) {
				fprintf(stderr, "Cannot link <%s>&<%s>.\n",
					ml[i]->m_name, namep);
			}
/* 			set_time(namep, Hdr.h_mtime, Hdr.h_mtime); */
			return 0;
		}
	}
	if(mlinks == 250
	|| (ml[mlinks] = alloc(strlen(namep) + sizeof(struct ml))) < 0) {
		fprintf(stderr, "Out of core for links\n");
		mlinks = 250;
		return 1;
	}
	ml[mlinks]->m_dev = Hdr.h_dev;
	ml[mlinks]->m_ino = Hdr.h_ino;
	strcpy(ml[mlinks]->m_name, namep);
	++mlinks;
	return 1;
}

pentry(namep)
register char *namep;
{

	register i;
	static lastid -1;
	static char nbuf[100], tbuf[32];

	printf("%-7o", Hdr.h_mode);
	if(lastid == Hdr.h_uid)
		printf("%-6s", nbuf);
	else {
		if(getpw(Hdr.h_uid&0377, nbuf)==0) {
			for(i=0; nbuf[i] != ':'; ++i);
			nbuf[i] = '\0';
			printf("%-6s", nbuf);
			lastid = Hdr.h_uid;
		} else
			printf("%-6d", Hdr.h_uid);
	}
	printf("%7D ", Hdr.h_filesize);
	strcpy(tbuf, ctime(&Hdr.h_mtime));
	tbuf[24] = '\0';
	printf(" %s  %s\n", &tbuf[4], namep);
}

gmatch(s, p)
register char *s, *p;
{
	register int c;
	int cc, ok, lc, scc;

	if(EQ(p, "*"))
		return 1;
	scc = *s;
	lc = 077777;
	switch (c = *p) {

	case '[':
		ok = 0;
		while (cc = *++p) {
			switch (cc) {

			case ']':
				if (ok)
					return(gmatch(++s, ++p));
				else
					return(0);

			case '-':
				ok =| lc <= scc & scc <= (cc=p[1]);
			}
			if (scc==(lc=cc)) ok++;
		}
		return(0);

	case '?':
	caseq:
		if(scc) return(gmatch(++s, ++p));
		return(0);
	case '*':
		return(umatch(s, ++p));
	case 0:
		return(!scc);
	}
	if (c==scc) goto caseq;
	return(0);
}

umatch(s, p)
register char *s, *p;
{
	if(*p==0) return(1);
	while(*s)
		if (gmatch(s++,p)) return(1);
	return(0);
}

makdir(namep)
register char *namep;
{
	if(fork())
		wait(0);
	else {
		execl("/bin/mkdir", "mkdir", namep, 0);
		fprintf(stderr, "mkdir <%s> failed\n", namep);
		exit(1);
	}
}

/* set_time(namep, atime, mtime)
register *namep;
long atime, mtime;
{
	static long timevec[2];

	if(!Mod_time)
		return;
	timevec[0] = atime;
	timevec[1] = mtime;
	utime(namep, timevec);
}
*/
