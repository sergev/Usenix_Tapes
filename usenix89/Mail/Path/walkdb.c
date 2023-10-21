#include <stdio.h>

extern char *gets();
extern char *malloc();
extern char *index();
extern char *rindex();

char *zalloc();

#define	node	struct _node

node {
    node *nnext;
    char *myname;
    char *rname;
    char *lname;
    };

char msg_string[] = "string";
char msg_node[] = "node";

node *leaf[128];

char iline[1024];

#define	newstr(s)	zalloc(msg_string,strlen(s)+1)
#define	newnode()	(node *)zalloc(msg_node,sizeof(node))

main(){
    int i;

    for(i=0; i<128;) leaf[i++] = (node *)0;

    while(gets(iline)) addnode();
    writenode();
}

addnode(){
    int i;
    node *n;
    char *s, *s1, *s2;
    char *dstnode, *leafnode, *rootnode;
    static char zap[] = "!!";
    static char zip[] = "!!";

    dstnode = leafnode = rootnode = 0;

    s = index(iline,'\t');
    if(!s){
	fprintf(stderr,"warning: no tab in \"%s\".\n",iline);
	return;
	}
    dstnode = iline;
    *s = 0; s1 = s+1;
    s = rindex(s1,'!');
    if(!s){
	fprintf(stderr,"warning: better be host name \"%s\".\n",iline);
	return;
	}
    *s = 0;
    s2 = rindex(s1,'!');
    if(!s2){
	leafnode = s1;
	fprintf(stderr,"warning: better be direct link \"%s\".\n",iline);
	s2 = &zip[1];
	strcpy(zip,zap);
	}
    else leafnode = s2+1;
    *s2 = 0;
    s2 = rindex(s1,'!');
    if(!s2) s2 = s1;
    else ++s2;
    rootnode = s2;

    if(strcmp(dstnode,leafnode)) rootnode = 0;
    else leafnode = 0;    
#if 0
    if(rootnode)
	fprintf(stderr,"parent of %s is %s\n",dstnode,rootnode);
    else
	fprintf(stderr,"alias  of %s is %s\n",dstnode,leafnode);
#endif
    n = newnode();
    n->myname = newstr(dstnode);
    strcpy(n->myname,dstnode);
    if(rootnode){
	n->rname = newstr(rootnode);
	strcpy(n->rname,rootnode);
	}
    else n->rname = 0;
    if(leafnode){
	n->lname = newstr(leafnode);
	strcpy(n->lname,leafnode);
	}
    else n->lname = 0;
    i = *dstnode;
    n->nnext = leaf[i];
    leaf[i] = n;
}

char *zalloc(reason,size)
char *reason;
unsigned size;
{
    char *s;

    s = malloc(size);
    if(!s){
	fprintf(stderr,"Couldn't allocate %d bytes for a %s.\n",size,reason);
	exit(1);
	}
    return(s);
}

writenode(){
    int i, j;
    register node *n;
    FILE *ochan;
    char zap[50];

fprintf(stderr,"writing nodes now.\n");

    for(i=0; i<128; ++i)
	if(n=leaf[i]){
	    sprintf(zap,"br%d.map.c",i);
	    ochan = fopen(zap,"w");
	    if(!ochan){
		fprintf(stderr,"Couldn't open %s for writing.\n",zap);
		exit(2);
		}
	    fprintf(ochan,"#include \"walkdb.h\"\n\n");
	    for(j=0; n; ++j, n=n->nnext);
	    ++j;
	    fprintf(ochan,"WALK br%d[%d] = {\n",i,j);
	    for(n=leaf[i]; n; n=n->nnext){
		fprintf(ochan,"    { \"%s\", 0x", n->myname);
		if(n->rname){
		    fprintf(ochan,"%02x, ",*(n->rname));
		    j = lookfor(n->rname);
		    }
		else{
		    fprintf(ochan,"%02x, ",*(n->lname) + 0x80);
		    j = lookfor(n->lname);
		    }
		fprintf(ochan,"%d },\n",j);
		}
	    fprintf(ochan,"    { \"\", 0, 0 }\n};\n\n");
	    fclose(ochan);
	    }
    ochan = fopen("master.map.c","w");
    if(!ochan){
	fprintf(stderr,"Couldn't open master.map.c for writing.\n");
	exit(3);
	}
    fprintf(ochan,"#include \"walkdb.h\"\n\n");
    for(i=0; i<128; ++i)
	if(leaf[i])
	    fprintf(ochan,"extern WALK br%d[];\n",i);
    fprintf(ochan,"\n\nWALK *dbase[129] = {\n");
    for(i=0; i<128; ++i){
	if(leaf[i]) fprintf(ochan,"    br%d,\n",i);
	else fprintf(ochan,"    0,\n");
	}
    fprintf(ochan,"    0\n};\n");
    fclose(ochan);
}

lookfor(s)
char *s;
{
    register int i, j;
    register node *n;
    char *e;

    i = *s;
    n = leaf[i];
    if(!n){ e = "bad initial character"; goto error; }
    for(j=0; n; ++j, n=n->nnext){
	if(!strcmp(s,n->myname))
	    return(j);
	}
    e = "node not in leaf";
error:
fprintf(stderr,"lookfor(%s) - %s.\n",s,e);
    return(-1);
}
