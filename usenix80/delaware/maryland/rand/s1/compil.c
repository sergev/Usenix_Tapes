/* compil - execute cc on argument files - compiling those which need it.
	for each arg "file" - if file.c was modified since file.o
	was created, the arg passed is file.c, else file.o.  Args
	preceded by - are left alone */
int status;     /* save wait status */

main(nargs,args)
int nargs;
char *args[];
{
char svfile[21];
char ch, pwf[100];      /* password buffer: nice and big */
char **a;
int i,j,oc;
int new0,new1;
long buf[9],obuf[9];
strcopy("/tmp/compil.xxxxxxxx",svfile); /* x's will be user name */
getpw(getuid(),pwf);			/* find relevant line in passwd file*/
for (i=12; pwf[i-12] != ':'; i++) svfile[i]=pwf[i-12];	/* copy up to colon */
svfile[i]=0;		/* terminate with a zero */
if (nargs == 1) 	/* then read from a /tmp file */
{	 new0=dup(0);
	 close(0);
	 if (open(svfile,0) < 0)
	 {	 printf("compil: no args given\n");
		 exit(0);
	 }
	 i=j=0; 		     /* character and line counters */
	 args[0]=alloc(20);		/* bug: max of 20 chars per name */
	 while ((ch=putchar(getchar())) != '\n')
	 {	 if (ch==' ')			 /* end of argument */
			{args[i++][j]=0;   /* end str with null */
			 args[i]=alloc(20);
			 args[i][j=0]=0;	/* to test emptiness */
			}
		 else
			args[i][j++]=ch;
	 }
	 if (j != 0) args[i++][j]=0;		/* get the last one */
	 close(0); dup(new0); close(new0);
	 nargs=i;
}

a=alloc(2*(nargs+1));
a[nargs] = 0;
for (i = 1; i < nargs  ; i++)
   if (*args[i] == '-') a[i]=args[i];
   else
   {	if (stat(a[i]=cat(args[i],".c"),buf) != -1) /* there's a .c file */
	{	if (stat(cat(args[i],".o"),obuf) == -1);  /* no .o file */
		else if (buf[8]<obuf[8]) a[i]=cat(args[i],".o"); /* chk date*/
	}
	else if (stat(cat(args[i],".o"),buf) != -1) a[i]=cat(args[i],".o");
		/* will settle for .o file */
	else if (stat(args[i]) != -1) a[i] = args[i];
		/* take named file if any */
	else printf("compil: file %s not found.\n",args[i]);
   }
a[0] = "cc";
new1=dup(1);			/* preserve standard output */
close(1);			/* so next dup will be to 1 */
creat(svfile,0777);	   /* make a file */
for (i=0; i<nargs; i++) printf("%s ",args[i]);
printf("\n");
close(1); dup(new1); close(new1);

for (i = 0; i<nargs  ; i++) printf("%s ",a[i]); printf("\n");
if (stat("a.out",buf) != -1)	/* a.out exists, so delete it */
{	if (fork()==0)
	{	execl("/bin/rm","rm","a.out",0);
		printf("a.out in some strange state\n");
	}
	wait(&status);
}
if (fork()==0)
{	execv("cc",a);		/* get cc from own directory if there */
	execv("/bin/cc",a);
	printf("Couldn't find /bin/cc.\n");
}
wait(&status);  /* wait for the compilation to be completed */
if (strcomp(args[0],"ex"))
	if (stat("a.out",buf) != -1)	/* then a.out exists */
		{printf("ex: executing a.out\n");
		 execl("a.out","a.out",0);
		}
exit(0);		/* for some reason omitting this gives bus error */
}

char *cat(a,b)			/* assumes 2-char 2nd arg */
char *a,*b;
{
char *bb;
char *aa;
aa = a; 			/* point at 1st source string */
while (*aa++);			/* get to end of it */
aa = bb = alloc(3 + aa - a);	/* make a place 2 longer than orig string */
while (*a) *aa++ = *a++;	/* copy 1st source string into new place */
while (*aa++ = *b++);		/* concatenate 2nd source string on in */
return bb;			/* return address of result */
}

strcomp(s,t)
   char *s, *t;
   {	while (*s == *t++) if (!*s++) return(1); return(0);}

strcopy(s,t)
char *s,*t;
{while (*t++ = *s++);}
