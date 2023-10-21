#include <stdio.h>

extern char *gets();
extern	char *malloc();
extern char *index(), *rindex();

char iline[256];

#define	PATH	struct _zap
PATH{
    PATH *next;
    char *site;
    char *path;
};

#define	newpath()	(PATH *)malloc(sizeof(PATH))

PATH *paths = 0;

main(){
    while(gets(iline)){
	addpath();
	}
    fixpaths();
    writepaths();
}

addpath(){
    char *s;
    char *s1;
    char *s2, *s3;
    PATH *p;

    s = index(iline,'\t');
    if(!s){
	fprintf(stderr,"No tab in path - \"%s\".\n",iline);
	return;
	}
    *s = 0;
    s1 = s+1;
    s = rindex(s1,'!');
    if(!s){
	fprintf(stderr,"No bang in path - \"%s\t%s\".\n",iline,s1);
	s = s1;
	}
    *s = 0;

    p = newpath();
    if(!p) outofmem();
    s2 = malloc(strlen(iline)+1);
    if(!s2) outofmem();
    s3 = malloc(strlen(s1)+1);
    if(!s3) outofmem();
    p->next = paths;
    paths = p;
    strcpy(s2,iline);
    strcpy(s3,s1);
    p->site = s2;
    p->path = s3;
}

outofmem(){
    fprintf(stderr,"Out of memory.\n");
    exit(1);
}

fixpaths(){
    register PATH *p1, *p2;
    register char *s;

    for(p1=paths; p1; p1=p1->next){
	s = rindex(p1->path,'!');
	if(!s) continue;
	++s;
	if(!strcmp(s,p1->site)) continue;
	for(p2=paths; p2; p2=p2->next){
	    if(!strcmp(s,p2->site)) goto cmain;
	    }
	sprintf(iline,"%s\t%s!%%s",s,p1->path);
	fprintf(stderr,"Adding line: \"%s\".\n",iline);
	addpath();
cmain: ;
	}
}

writepaths(){
    register PATH *p;

    for(p=paths; p; p=p->next){
	if(*(p->path))
	    printf("%s\t%s!%%s\n",p->site,p->path);
	else
	    printf("%s\t%%s\n",p->site);
	}
}
