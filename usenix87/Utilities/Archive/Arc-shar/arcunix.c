#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <time.h>
#include "arc.h"

char *index(), *rindex(), *xalloc(), *malloc(), *realloc();
extern char *Progname;
extern int nerrs;

#if m68000
typedef long int dos_long_t;
typedef short int dos_int_t;
#define doshdrsize (FNLEN + sizeof(dos_long_t) + sizeof(dos_int_t) + \
		sizeof(dos_int_t) + sizeof(dos_int_t) + sizeof(dos_long_t))
#define odoshdrsize (doshdrsize - sizeof(dos_long_t))
#define dos2long(x) ((unsigned char)(x)[0] + ((unsigned char)(x)[1]<<8) + \
		((unsigned char)(x)[2]<<16) + ((unsigned char)(x)[3]<<24))
#define dos2int(x) (0xffff & ((unsigned char)(x)[0]+((unsigned char)(x)[1]<<8)))
#define long2dos(x,l) ((x)[0]=(char)((l)&0xff), (x)[1]=(char)((l)>>8&0xff),\
		(x)[2]=(char)((l)>>16&0xff), (x)[3]=(char)((l)>>24&0xff))
#define int2dos(x,i) ((x)[0]=(char)((i)&0xff), (x)[1]=(char)((i)>>8&0xff))

/* urdhdr -- read a new-style dos header; munge to unix header */
urdhdr(hdr,f)
struct heads *hdr;
FILE *f;
{
	_urdhdr(hdr,f,doshdrsize);
}
/* urdohdr -- read an old-style dos header; munge to unix header */
urdohdr(hdr,f)
struct heads *hdr;
FILE *f;
{
	_urdhdr(hdr,f,odoshdrsize);
}
static _urdhdr(hdr,f,size)
struct heads *hdr;
FILE *f;
int size;
{
	char doshdr[doshdrsize];
	char *s = doshdr;

	fread(doshdr,size,1,f);
	memcpy(hdr->name,s,sizeof(hdr->name)); s += FNLEN;
/*printf("\nurdhdr: name=0x%x,'%.12s'0x%x",doshdr,hdr->name,s);*/
	hdr->size = dos2long(s);	s += sizeof(dos_long_t);
/*printf(" size=%ld,0x%x",hdr->size,s);*/
	hdr->date = dos2int(s);		s += sizeof(dos_int_t);
/*printf(" date=%d,0x%x",hdr->date,s);*/
	hdr->time = dos2int(s);		s += sizeof(dos_int_t);
/*printf(" time=%d,0x%x",hdr->time,s);*/
	hdr->crc = dos2int(s);		s += sizeof(dos_int_t);
/*printf(" crc=%d,0x%x",hdr->crc,s);*/
	if(size == doshdrsize)		hdr->length = dos2long(s);
/*if(size == doshdrsize) printf(" length=%d\n",hdr->length);*/
}
/* uwrhdr -- write a dos header; munge from unix header */
uwrhdr(hdr,f)
struct heads *hdr;
FILE *f;
{
	char doshdr[doshdrsize];
	char *s = doshdr;

	memcpy(s,hdr->name,FNLEN);	s += FNLEN;
	long2dos(s, hdr->size);		s += sizeof(dos_long_t);
	int2dos(s, hdr->date);		s += sizeof(dos_int_t);
	int2dos(s, hdr->time);		s += sizeof(dos_int_t);
	int2dos(s, hdr->crc);		s += sizeof(dos_int_t);
	long2dos(s, hdr->length);	s += sizeof(dos_long_t);
	fwrite(doshdr,sizeof(doshdr),1,f);
}
#endif

/*
 * makefnam -- replace any extension in base with extension in ext, save in buf
 */
char *makefnam(base,ext,buf)
char *base,*ext,*buf;
{
	char *s;
	strcpy(buf, base);
	if((base = rindex(buf, '/')) == NULL)
		base = buf;
	if((s = index(base,'.')) != NULL) *s = '\0';
	if((s = rindex(ext,'.')) != NULL) strcat(buf,s);
	else {
		strcat(buf,".");strcat(buf,ext);
	}
	return(buf);
}

char *lower(str)
char *str;
{
	char *s;
	for(s = str; *s; ++s)
		if(isupper(*s)) *s = tolower(*s);
	return(str);
}

char *upper(str)
char *str;
{
	char *s;
	for(s = str; *s; ++s)
		if(islower(*s)) *s = toupper(*s);
	return(str);
}

/*
 * kludge; assume we wont run out of memory
 */
unsigned coreleft()
{
	return(5120);	/* always enough */
}

/*
 * dir -- glob pattern, if speced, and repeatedly return matches
 * NOTE: return nulstr, NOT NULL, on no match
 */
char *dir(pattern,mode)               /* get files, one by one */
char *pattern;                        /* template, or NULL */
int mode;                              /* search mode bits */
{
	static FILE *fp = NULL;
	static char nulstr[] = "";
	static char buf[1025];
	char *s, *fgets();
	extern FILE *popen();

	if(fp == NULL && pattern == NULL)
		return(nulstr);
	if (pattern) {
		if(fp) pclose(fp);
		sprintf(buf,"echo %s | tr ' ' '\012'",pattern);
		fp = popen(buf,"r");
		if (fp == NULL)
			abort("dir(): cant glob %s",pattern);
	}
	if(fgets(buf,sizeof(buf),fp) == NULL) {
		pclose(fp);
		fp = NULL;
		return(nulstr);
	}
	if((s = index(buf,'\n')) != NULL) *s = '\0';
	if((s = rindex(buf,'/')) == NULL)
		s = buf;
	else
		++s;
	if(legalize(s))
		return(nulstr);
	pattern = xalloc(strlen(s)+1);
	strcpy(pattern, s);
	return(pattern);
}

static legalize(name)
char *name;
{
	char *dot = index(name,'.');
	if(dot && ((dot-name > 8) || index(dot+1,'.'))) {
		fprintf(stderr,"%s: unix name '%s' not legal as dos name; skipping this pattern\n",Progname,name);
		++nerrs;
		return(1);
	}
	if((dot && strlen(dot+1) > 3) ||
	  (!dot && strlen(name) > 12)) {
truncate:
		fprintf(stderr,"%s: Warning: truncating name '%s' to ",Progname,name);
		if(dot) dot[4] = '\0';
		else name[12] = '\0';
		fprintf(stderr,"'%s'\n",name);
	}
	return(0);
}

setmem(buf,len,c)
char *buf,c;
int len;
{
	while(len--)
		*buf++ = c;
}

rename(from, to)
char *from, *to;
{
	if(link(from, to))
		return(-1);
	return(unlink(from));
}

/*
 * setstamp -- convert dos time to unix tm and update access, modify times
 */
setstamp(path,date,time)                  /* set a file's date/time stamp */
char *path;                               /* file to set stamp on */
unsigned int date, time;               /* desired date, time */
{
	extern time_t tm_to_time ();
	struct tm t;
	struct {
	time_t a,m;
	} ut;

	int yr = (date >> 9) & 0x7f;      /* dissect the date */
	int mo = (date >> 5) & 0x0f;
	int dy = date & 0x1f;

	int hr = (time >> 11) & 0x1f;     /* dissect the time */
	int mm = (time >> 5) & 0x3f;
	int ss = (time & 0x1f) * 2;

	t.tm_year = (yr + 80) % 100;
	t.tm_mon = mo - 1;
	t.tm_mday = dy;
	t.tm_hour = hr;
	t.tm_min = mm;
	t.tm_sec = ss;
	ut.a = ut.m = tm_to_time(&t);
	if(utime(path, &ut)) {
		fprintf(stderr, "%s: cant set utime ",Progname);
		perror(path);
	}
}

/*
 * setstamp -- get modify time and convert dos time
 */
getstamp(path,date,time)                  /* get a file's date/time stamp */
char *path;                               /* file to get stamp from */
unsigned int *date, *time;                /* storage for the stamp */
{
	struct stat sbuf;
	if(stat(path, &sbuf)) {
		fprintf(stderr, "%s: cant stat ",Progname);
		perror(path);
		*date = *time = 0;
	}
	else {
		struct tm *localtime();
		struct tm *t = localtime(&sbuf.st_mtime);
		int yr = t->tm_year - 80;
		int mo = t->tm_mon + 1;
		int dy = t->tm_mday;
		int hr = t->tm_hour;
		int mm = t->tm_min;
		int ss = t->tm_sec;
		*date = (yr<<9)+(mo<<5)+dy;
		*time = (hr<<11)+(mm<<5)+ss;
	}
}

/*
 * xalloc -- malloc with error checking
 */
char *xalloc(n)
unsigned n;
{
	char *s = malloc(n);
	if(s == NULL)
		outofmem();
	return(s);
}

/*
 * xrealloc -- realloc with error checking
 */
char *xrealloc(s,n)
char *s;
unsigned n;
{
	s = realloc(s,n);
	if(s == NULL)
		outofmem();
	return(s);
}
static outofmem()
{
	Fatal("out of memory",ENOMEM);
}
static Fatal(s,code)
char *s;
int code;
{
	fprintf(stderr,"\n%s: fatal error\n%s\n",Progname,s);
	exit(code);
}
abort(a,b,c,d,e)
{
	char buf[100];
	sprintf(buf,a,b,c,d,e);
	Fatal(buf,1);
}
