#include <stdio.h>

extern long time();
extern short ran_vec[];

#define	MAXCOL	78

int col = 0;
int space = 1;

struct gstruct {
         int items;
         char *item[128];
         char data;
       } *group[64];

int groups;
char line[2048];
int startdata[20000];
FILE *f1;

rnd(i)
  int i;
{
  return((rand()&0xffff) % i);
}

search(word)
  char word[];
{
  int i;
  for(i=0;i<groups;++i)
  if(comps(word,group[i]->item[0]))return(i);
  return(-1);
}

comps(word1,word2)
  char word1[],word2[];
{
  int i;
  for(i=0;(word1[i]==word2[i]&&word1[i]!=0&&word2[i]!=0);++i);
  if(word1[i]!=word2[i])return(0);
  return(1);
}

alpha(a)
  char a;
{
  if((a>='a'&&a<='z')||(a>='A'&&a<='Z')||(a>='0'&&a<='9')||(a=='_'))return(1);
  return(0);
}

init(filename)
  char filename[];
{
  int i;
  char *pnt;
  f1=fopen(filename,"r");
  if(f1==NULL){printf("Bad open\n");exit();}
  groups=0;
  group[groups] = (struct gstruct *)(&startdata[0]);
  group[groups]->items=0;
  group[groups]->item[0]= &(group[groups]->data);
  for(;;)
    {
      getline(f1,line);
      if(line[0]=='%')
        {
          group[groups+1]=(struct gstruct *)((int)(group[groups]->item[group[groups]->items]+1) & -2);
          ++groups;
          group[groups]->items=0;
          group[groups]->item[0]= &(group[groups]->data);
          getline(f1,line);
          if(line[0]=='%')return(1);
        }
      pnt=group[groups]->item[group[groups]->items];
      for(i=0;(*(pnt++)=line[i++])!=0;);
      group[groups]->items += 1;
      group[groups]->item[group[groups]->items] = pnt;
    }
}

exec(word)
  char word[];
{
  char *ppnt;
  char word1[64];
  int g;
  g=search(word);
  if(g == -1)
    {
      g = strlen(word);
      if((col+g) > MAXCOL){
	outstr("\n");
	}
      outstr(word);
      return(1);
    }
  ppnt=group[g]->item[1+rnd(group[g]->items - 1)];
  for(;;)
    {
      getword(&ppnt,word1);
      if(word1[0]==0)return(1);
      exec(word1);
    }
}

getword(ppnt,word)
  char word[];
  char **ppnt;
{
  int i;
  static char tstr[] = "x";

  word[0]=0;
  while((!alpha(**ppnt))&&(**ppnt!=0))
    {
      if(**ppnt == '\\'){ outstr("\n"); }
      else if(**ppnt != '|'){
	tstr[0] = **ppnt;
	outstr(tstr);
	}
      (*ppnt)++;
    }
  if(**ppnt==0)return(1);
  for(i=0;alpha(**ppnt);(*ppnt)++)word[i++]= **ppnt;
  word[i]=0;
}

getline(f1,line)
  char line[];
  FILE *f1;
{
  int i;
  for(i=0;(line[i]=getc(f1))!='\n';++i);
  line[i]=0;
}

argerr()
{ 
  printf("Correct usage is    code (-nn) protofile\n");
  exit();
}

main(argc,argv)
  int argc;
  char *argv[];
{
  int i,st,ncount;
  char filename[32];
  ncount=1;
  if(argc==1) argerr();
  if(argc>3) argerr();
  if(argc==3)
    {
      ncount=0;
      if(argv[1][0]!='-') argerr();
      for(i=1;argv[1][i]!=0;++i)ncount=ncount*10+argv[1][i]-'0';
      for(i=0;argv[2][i]!=0;++i)filename[i]=argv[2][i];
    }
  if(argc==2)
    for(i=0;argv[1][i]!=0;++i)filename[i]=argv[1][i];
  st = time(0);
  srand(st);
  init(filename);
  for(i=0;i<ncount;++i)exec("CODE");
}

#define	isalpha(ch)	((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z'))
#define	toupper(ch)	((ch>='a'&&ch<='z')?(ch-'a'+'A'):ch)
#define	tolower(ch)	((ch>='A'&&ch<='Z')?(ch-'A'+'a'):ch)
#define	isspace(ch)	((ch==' '||ch=='\n'||ch=='\t'))

outstr(s)
char *s;
{
    for(; *s; ++s){
	if(*s == '@'){ space = 1; continue; }
	if(isspace(*s) || *s == '.') space = 1;
	else if(isalpha(*s)){
	    if(!space) *s = tolower(*s);
	    else{
		*s = toupper(*s);
		space = 0;
		}
	    }
	putchar(*s);
	if(*s == '\n') col = 0; else ++col;
	}
}
