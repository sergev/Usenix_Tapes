#
/**********/
/* dlm's filemagic.h */
/*
 *      These defines contain a list of a major magic
 *      numbers for data files.  Used by lpr.c
 *      and perhaps file.c
 */
#define ARMAGIC  0177555 /* archive (old format) */
#define NARMAGIC 0177554 /* source archive (narc) */
#define NETMAGIC 0177553 /* netarc archve (nethelp) */
#define ARCMAGIC 0177545 /* new archiver format */
#define VMAGIC   0405    /* for overlays etc. */
#define FMAGIC   0407    /* standard executable */
#define NMAGIC   0410    /* pure, shareable executable */
#define IMAGIC   0411    /* separate I/D executable */
/**************/
/*
Name:
        compare

Synopsis:
        compare [-a] [-ad] [-r] [-v] [-d] [-e] [-l] sources destination

History:
        written by Keith Davis Feb 13, 1976
        mod by RBalocca UoI to exec diff 1978March28

installation:
        cc -O -n -s compare.c -lj -o compare;chmod 555 compare;
        cp compare /usr/bin/.

*/
char id[] "~|^`compare.c:\t2.0\n";


int parent;     /* the parent process id for the mkdir forks */

struct
{       
        int     dev;    /* the device type */
        int     inumber;/* the inode number */
        int     flags;  /* the flags */
        char    nlinks; /* number of links to the file */
        char    uid;    /* the uid of the file */
        char    gid;    /* the group id of the file */
        char    size0;  /* high byte of the 24 bit size */
        int     size1;  /* low word of the 24 bit size */
        int     addr;   /* first address */
        int     jnk[11];/* other info */
}       
stat1, stat2;

int     type1, type2;

char name1[256],name2[272]; /* buffers for source and destination names */
char *name1l, *name2l;  /* points to the last char '\0' in name1 and name2 */

int     direntry[9];    /* buffer for directory entries */

int     askflag;        /* ask before doing anything */
int     askdirflag;     /* ask before doing anything to directories */
int     dupflag;        /* if on then duplicates will be removed */
int     xdifflag;       /* exec diff if there are differences */
int     linkflag;       /* if on then duplicates will be linked */
int     recursiveflag;  /* if on then will branch down directory structure */
int     verboseflag;    /* prints lots of information */

main(argc, argv)
int argc;
char **argv;
{       
        register int i;
        register char *s, *t;
        char *name2ptr; /* remembers loc of last arg */
        int k;  
        k = 0;
        for(i=2;i<16;i++) close(i); /* close unneeded files */
        if ( equal("differ",argv[0])) xdifflag = 1;  /* differ => -e */
        for(i=1;i<argc;i++)
        {
                if(argv[i][0] == '-') switch(argv[i][1])
                {       
                case 'a': 
                        if (argv[i][2] == 'd') askdirflag = 1;
                        else askflag = askdirflag = 1;
                        break;
                case 'd': 
                        dupflag = 1;  
                        break;
                case 'e': 
                        xdifflag = 1; 
                        break;
                case 'l': 
                        linkflag = 1;  
                        break;
                case 'r': 
                        recursiveflag = 1;  
                        break;
                case 'v': 
                        verboseflag = 1;  
                        break;
                }
                else name2ptr = argv[k++] = argv[i]; /* copy the arg */
        }
        if(verboseflag)
        {
                printf("compare ");
                for(i=0;i<argc;i++)
                        printf("%s ",argv[i]);
                printf("\n");
        }
        if(--k < 1)  /* k now is the number of sources */
        {       
                if (k)
                {       
                        printf("Error: arg count\n");
                        exit(0);
                }
                k++; /* if only one source, use working directory */
                name2ptr = ".";
        }
        for(i=0;i<k;i++)   /* this code solves the multiple sources problem */
        {
                /* get a source */
                s = name1;  
                t = argv[i];
                while(*s++ = *t++);
                name1l = --s; /* point to the '\0' in char string */
                if (*--s == '/') { 
                        *s = 0; 
                        name1l = s; 
                } /* extra / */

                /* get the destination */
                s = name2;  
                t = name2ptr; /* always the same file */
                while(*s++ = *t++);
                name2l = --s;
                if (*--s == '/') { 
                        *s = 0; 
                        name2l = s; 
                } /* extra / */

                getstat();
                if (type1 == 2 && type2 != 0) cd(); /* do directory trans */
                else    /* do the cp command */
                {       /* make entry in the direntry for the cp command */
                        s = &direntry[1];  
                        t = name1;
                        while (*s = *t++) if(*s++ == '/') s = &direntry[1];
                        cp(); /* do the cp command */
                }
        }
}

getstat() /* gets the status on the two files */
{       
        if (stat(name1,&stat1) == -1) type1 = 4;
        else    type1 = ((stat1.flags)>>13)&3;
        if (stat(name2,&stat2) == -1) type2 = 4;
        else    type2 = ((stat2.flags)>>13)&3;
}

cd() /* copy a directory */
{       
        register int dirfd; /* source-directory file descriptor */
        int pid,status,mode,modeflags,name1s,name2s;
        mode = type2;  
        modeflags = stat1.flags; /* save for later use */
        if (type2 == 4) /* does not exist so create the directory */
        {       
                printf("could not find directory %s\n",name2);
                return;
        }
        if (type2 != 2) /* if two then directory otherwise error */
        {       
                printf("%s not a directory\n",name2);
                return;
        }
        if ((dirfd = open(name1,0)) == -1)
        {       
                printf("Error: could not open directory %s\n",name1);
                return;
        }
        name1s = name1l;  
        name2s = name2l; /* save pointers */
        getentry(dirfd); /* skip first two entries */
        getentry(dirfd); /* because they are the "." & ".." entries */
        while(getentry(dirfd))  /* get an entry */
        {       
                push_name();    /* and push it onto the name1 & name2 */
                getstat();
                switch (type1)
                {       
                case 0: /* just a file */
                        if (ask(askflag,"compare file",name1)) cp();
                        break;
                case 1: /* character-type special file */
                case 3: /* block-type special file */
                        if (ask(askdirflag,"compare special file",name1)) sf();
                        break;
                case 2: /* directory */
                        if (recursiveflag) /* ignore if not */
                                if (ask(askdirflag,"examine directory",name1))
                                        cd(); /* compare the directory */
                        break;
                default:/* error */
                        printf("Error: %s does not exist",name1);
                        return;
                }
                name1l = name1s;  
                *name1l = 0; /* pop name1 */
                name2l = name2s;  
                *name2l = 0; /* pop name2 */
        }
        close(dirfd); /* close the file source-directory */
        return;
}


sf() /* copy a special file */
{       
        getstat();
        if(type1        != type2        ||
            stat1.flags  != stat2.flags  ||
            stat1.addr   != stat2.addr   )
        {       
                printf("special file %s does not compare\n",name2);
        }
}

getentry(df)
int df; /* the directory file descriptor */
{       
        do
            {       
                if(read(df,direntry,16) != 16) return(0);
        } 
        while (direntry[0] == 0);
        return(1);
}

push_name()
{       
        register char *s, *t;

        /* add entry onto name1 */
        if (name1l+17 > &name1[256])
        {       
                printf("Error file name too long %s\n",name1);
                exit(1);
        }
        s = name1l;  
        t = &direntry[1];  
        *s++ = '/';
        while (*s++ = *t++);
        name1l = --s;

        /* add entry onto name2 */
        if (name2l+17 > &name2[256])
        {       
                printf("Error file name too long %s\n",name2);
                exit(1);
        }
        s = name2l;  
        t = &direntry[1];  
        *s++ = '/';
        while (*s++ = *t++);
        name2l = --s;
}


cp()    /* works just like the cp command */
{
        int bufold[256],bufnew[256];
        int fold, fnew;
        register int n;
        register char *s, *t;
        struct {
                int integer;
        };
        int dummy;
        int magic;

        fold = fnew = -1;
        /* is target a directory? */
        if (type2 == 2) /* if so then pad name2 with direntry */
        {       
                s = name2l;        /* set to last char in destination name */
                *s++ = '/';        /* push a '/' */
                t = &direntry[1];  /* points to begin of source string */
                while(*s++ = *t++);/* add last part of name2 */
                getstat();
        }

        if (type2 == 4)
        {       
                printf("Error: %s does not exist\n",name2);
                goto out;
        }

        magic = 0;

        /* process quickly if they are the same device and inumber */
        if (stat1.dev == stat2.dev && stat1.inumber == stat2.inumber)
        {       
                if (dupflag) unlink(name2);
                return;
        }

        /* process quickly if the sizes differ */
        if (stat1.size0 != stat2.size0) goto nout;
        if (stat1.size1 != stat2.size1) goto nout;
        if((fold = open(name1,0)) < 0)
        {       
                printf("Error: cannot open %s\n",name1);
                goto out;
        }
        if((fnew = open(name2,0)) < 0)
        {       
                printf("Error: cannot open %s\n",name2);
                goto out;
        }

        for(;;)
        {
                s = bufold;  
                t = bufnew;
                if ((n = read(fold,s,sizeof bufold)) < 0)
                {       
                        printf("Error: read error on %s\n",name1);
                        goto out;
                }
                if (read(fnew,t,sizeof bufnew) != n) goto nout;
                if (n==0) /* the files matched */
                {       
                        if (dupflag) unlink(name2);
                        else if (stat1.dev == stat2.dev)
                        {       
                                if (linkflag && stat1.dev == stat2.dev)
                                {       
                                        unlink(name2);
                                        link(name1,name2);
                                }
                                else printf("%s compared but were not linked\n",name1);
                        }
                        goto out;
                }
                if(!magic)  /* check first block for magic numbers */
                {
                        magic = mgc1(s->integer,t->integer);
                }
                do if(*s++ != *t++) goto nout; 
                while(--n);
        }

nout:   
        printf("%s does not compare\n",name1);
        if(xdifflag)
        {
                if(!magic)      /* file is not a magic number (object) */
                {
                        if(fnew<0)
                                fnew = open(name2,0);
                        s = bufnew;
                        if(read(fnew,s,sizeof s->integer)<sizeof s->integer)
                                s->integer = 0;
                        if(fold<0)
                                fold = open(name1,0);
                        t = bufold;
                        if(read(fold,t,sizeof t->integer)<sizeof t->integer)
                                t->integer = 0;
                        magic = mgc1(s->integer,t->integer);
                }

                if(magic!=1)
                        for(;;)
                        {
                                switch(fork())
                                {
                                case 0:
                                        printf("_____\n");
                                        execl("/usr/bin/diff", "difference", name1, name2, 0);
                                        execl("/bin/diff", "difference", name1, name2, 0);
                                        printf("Can't exec diff\n");
                                        exit(-1);

                                case -1:
                                        sleep(100);
                                        continue;

                                default:
                                        wait(&dummy);
                                        printf("_____\n");
                                        break;
                                }
                                break;
                        }

        }
out:    
        if (fnew > 0) close(fnew);
        if (fold > 0) close(fold);
}

mgc1(a,b)
char * a, * b;
{
        if(mgc(a)||mgc(b))
                return 1;
        else
            return 2;
}

mgc(a)
char * a;
{
        switch(a)
        {
        case ARMAGIC:   /* archive (old format) */
        case NARMAGIC:  /* source archive (narc) */
        case NETMAGIC:  /* netarc archve (nethelp) */
        case ARCMAGIC:  /* new archiver format */
        case VMAGIC:    /* for overlays etc. */
        case FMAGIC:    /* standard executable */
        case NMAGIC:    /* pure, shareable executable */
        case IMAGIC:    /* separate I/D executable */
                return 1;
        default:
                return 0;
        }
}

ask(f,s,t) int f;
{       
        char ch;
        register flg;
        if (f)  /* if flag true then ask question */
        {       
                printf("%s %s? ",s,t);
                if (read(0,&ch,1) != 1) return(0);
                flg = ch == 'y';
                while (ch != '\n' && read(0,&ch,1) == 1) ;
                return(flg);
        }
        else if (verboseflag) printf("%s %s\n",s,t);
        return(1);
}

equal(s, t)
register char *s, *t;
{
	while (*s)
		if (*s++ != *t++) return(0);
	if (*t == 0) return(1);
	return(0);
}
