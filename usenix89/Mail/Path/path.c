#include <stdio.h>
#include "walkdb.h"

#undef	DEBUG

extern WALK *dbase[];
extern int strcmp();
extern char *rindex();
extern char *index();

int strccmp();

WALK *lastsite = 0;

char prgname[128];
char site[128];
int numsites = 2;
char xpath[512];
int fullpath = 0;

int main(acnt,avar)
int acnt;
char *avar[];
{
    char *s;
    int argc, i;
    extern int optind;
    extern char *optarg;

    if(s = rindex(avar[0],'/')) ++s;
    else s = avar[0];
    strcpy(prgname,s);

    if(acnt < 2){
	fprintf(stderr,"%s: usage: %s <site> {<site>...}\n",prgname,prgname);
	exit(3);
	}

    while((i=getopt(acnt,avar,"s:rh")) != EOF){
	switch(i){
	    case 's':				/* Search for map */
		ssearch(optarg);
		break;
	    case 'r':
		fullpath = 1;
		break;
	    case 'h':
		header();
		return(0);
	    }
	}

   numsites = acnt - optind;
   for(; optind < acnt; ++optind){
	if(fullpath) upath(1,avar[optind]);
	else{
	    strcpy(site,avar[optind]);
	    findsite(1,xpath);
	    }
	}
}

header(){
    char iline[256];
    extern char *gets();

    fullpath = 1;
    numsites = 1;
    while(gets(iline)){
	if(!strncmp(iline,"To: ",4)){
	    upath(0,&iline[4]);
	    strcpy(&iline[4],xpath);
	    }
	printf("%s\n",iline);
	}
}

upath(prnt,fpath)
int prnt;
char *fpath;
{
    char uname[256];
    char itsite[256];
    int numbang, i;
    char *s;

    for(s=fpath, i=0; *s; ++s) i += (*s == '!');
    if(i < 2){
	strcpy(xpath,fpath);
	if(prnt) printf("%s\n",fpath);
	return;
	}

    for(numbang=1;; ++numbang){
	strcpy(itsite,fpath);
	for(s = &itsite[strlen(itsite)], i=0; s != itsite &&
							i < numbang ; --s)
	    if(*s == '!') ++i;
	if(i != numbang){
	    fprintf(stderr,"%s: couldn't find a known site.\n",prgname);
	    break;
	    }
	++s;
	strcpy(uname,s+1);
	*s = 0;
	s = rindex(itsite,'!');
	if(!s) s = itsite; else ++s;
	strcpy(site,s);
	if(findsite(0,xpath)){
	    strcat(xpath,"!");
	    strcat(xpath,uname);
	    if(prnt) printf("%s\n",xpath);
	    break;
	    }
	}
}

findsite(prnt,pname)
int prnt;
char *pname;
{
#ifdef DEBUG
fprintf(stderr,"findsite(0x%02x,pname) - site = %s\n",prnt,site);
#endif

    if(!(prnt & 0x80) && index(site,'.')) return(dodomain(prnt,pname));
    else{
	*pname = 0;
	if(!search(site[0],site,strcmp,pname) &&
           !search(chlower(site[0]),site,strccmp,pname) &&
           !search(chupper(site[0]),site,strccmp,pname)){
	    if(!(prnt & 0x40) && !fullpath)
		fprintf(stderr,"%s: Can't find a path to %s\n",prgname,site);
	    return(0);
	    }
	else{
	    if(prnt & 0x1){
		printf("%s\n",pname);
		}
	    return(1);
	    }
	}
}

dodomain(prnt,pname)
int prnt;
char *pname;
{
    char *s;
    char tsite[256];
    char tpath[256];

#ifdef DEBUG
fprintf(stderr,"dodomain(0x%02x,pname)\n",prnt);
#endif

    lastsite = 0;
    s = index(site,'.');
    strcpy(tsite,site);
    strcpy(site,s);
    if(!strccmp(site,".uucp")){
	strcpy(site,tsite);
	*s = 0;
	if(findsite(0xC0|prnt,pname)) return(1);
	strcpy(site,".uucp");
	}
    if(findsite(0xC0,tpath)){
	if(lastsite) strcpy(index(tsite,'.'),lastsite->wname);
	if((numsites > 1) && (s = index(tpath,':'))){
	    strcpy(pname,s);
	    strcpy(tpath,tsite);
	    strcat(tpath,pname);
	    }
	sprintf(pname,"%s!%s",tpath,tsite);
	if(prnt & 1) printf("%s\n",pname);
	return(1);
	}
    else{
	strcpy(site,tsite);
	return(findsite(prnt|0x80,pname));
	}
}

int chlower(ch)
int ch;
{
    return((ch >= 'A' && ch <= 'Z') ? ch-'A'+'a' : ch);
}

int chupper(ch)
int ch;
{
    return((ch >= 'a' && ch <= 'z') ? ch-'a'+'A' : ch);
}

int strccmp(s1,s2)
char *s1, *s2;
{
    while(*s1 && *s2 && chlower(*s1) == chlower(*s2)) ++s1, ++s2;
    return(chlower(*s1) - chlower(*s2));
}

ssearch(s)
char *s;
{
    register WALK *wr;
    register int j, i;
    int numfound = 0;
    char *e;
    extern char *re_comp();
    extern int re_exec();

    e = re_comp(s);
    if(e){
	fprintf(stderr,"%s: %s\n",prgname,e);
	return;
	}

    for(i=0; i<128; ++i){
	if(!(wr = dbase[i])) continue;
	for(j=0; wr; ++j, ++wr){
	    if(!(*(wr->wname))) break;
	    if(re_exec(wr->wname) == 1){
		strcpy(site,wr->wname);
		++numfound;
		findsite(1,xpath);
		}
	    }
	}
    if(!numfound)
	fprintf(stderr,"%s: Couldn't find any sites matching \"%s\".\n",
	    prgname,s);
}

int search(i,s,fn,pname)
int i;
char *s, *pname;
int (*fn)();
{
    register WALK *wr;
    register int j;

#ifdef DEBUG
fprintf(stderr,"search(0x%02x,\"%s\",fn,pname)\n",i,s);
#endif

    if(!(wr = dbase[i])) return(0);
    for(j=0; wr; ++j, ++wr){
	if(!(*(wr->wname))) break;
	if(!((*fn)(wr->wname,s))){
	    follow(i,j,pname);
	    return(1);
	    }
	}
    return(0);
}

follow(oi,oj,pname)
int oi, oj;
char *pname;
{
    int i, j;
    WALK *w;

    w = dbase[oi & 0x7F];
    w += oj;

    i = w->wflag;
    j = w->windex;

#ifdef DEBUG
fprintf(stderr,"%s: follow(0x%02x,0x%02x);\n",prgname,oi,oj);
fprintf(stderr,"\twname=\"%s\", wflag=0x%02x, windex=0x%02x.\n",
	w->wname,i,j);
#endif

    if((i & 0x7F) == oi && j == oj){
	if(numsites > 1) sprintf(pname+strlen(pname),"%s: ",site);
	strcat(pname,w->wname);
	return(1);
	}
    follow(i & 0x7F,j,pname);
    if(!(i & 0x80))
	sprintf(pname+strlen(pname),"!%s",w->wname);
    lastsite = w;
    return(0);
}
