
/*                   Tigertail C Extensions                       */
/*                                                                */
/*   Copyright (c) 1987 Tigertail Associates. All Rights Reserved */
/*                      320 North Bundy Drive                     */
/*                      Los Angeles, California 90049             */
/*                      USA                                       */
/*                                                                */
/*   e-mail: (uucp) ucla-cs!buz   or (arpa) buz@cs.ucla.edu       */

#ifndef PROC
#define     PROC
#define     then
#define     fi
#define     Bool	 int
#ifndef TRUE
#define     TRUE	 1
#define     FALSE	 0
#endif

#define     NULLC	 '\0'
#define     NULLF	 ((FILE *) NULL)
#define     NULLS	 ((char*) NULL)

#define     abs(x)   (((x)>0)?(x):(-(x)))
#define     sign(x)  (((x)>0)?(1):(((x)<0)?(-1):(0)))
#define     max(x,y) (((x)>(y))?(x):((y)))
#define     min(x,y) (((x)>(y))?(y):((x)))

/*  Define string functions and INDEX for compliers with index sans strchr. */

#if defined(INDEX) || defined(BSD)
#define     strchr   index
#define     strrchr  rindex
char *strcat();
char *strncat();
int   strcmp();
int   strncmp();
char *strcpy();
char *strncpy();
int   strlen();
char *strchr();
char *index();
char *rindex();
int   strspn();
int   strcspn();
char *strtok();
char *strdup();
#else
/*  Define string functions. */
#if defined(lint) || defined(SYSV)
char *strcat();
char *strncat();
int   strcmp();
int   strncmp();
char *strcpy();
char *strncpy();
int   strlen();
char *strchr();
char *strrchr();
char *strpbrk();
int   strspn();
int   strcspn();
char *strtok();
char *strdup();
#else /* Xenix is presumed */
char *strcat(char*,char*);
char *strncat(char*,char*,int);
int   strcmp(char*,char*);
int   strncmp(char*,char*,int);
char *strcpy(char*,char*);
char *strncpy(char*,char*,int);
int   strlen(char*);
char *strchr(char*,int);
char *strrchr(char*,int);
char *strpbrk(char*,char*);
int   strspn(char*,char*);
int   strcspn(char*,char*);
char *strtok(char*,char*);
char *strdup(char*);
#endif
#endif

/*   Debugging macros   */

#ifdef EBUG
#define debug1(w)			printf(w)
#define debug2(w,x)			printf(w,x)
#define debug3(w,x,y)		printf(w,x,y)
#define debug4(w,x,y,z)		printf(w,x,y,z)
#define debug5(w,x,y,z,a)	printf(w,x,y,z,a)
#define debug6(w,x,y,z,a,b)	printf(w,x,y,z,a,b)
#define debug7(w,x,y,z,a,b,c)	printf(w,x,y,z,a,b,c)
#define debug8(w,x,y,z,a,b,c,d)	printf(w,x,y,z,a,b,c,d)
#else
#define debug1(w)	
#define debug2(w,x)			
#define debug3(w,x,y)		
#define debug4(w,x,y,z)		
#define debug5(w,x,y,z,a)	
#define debug6(w,x,y,z,a,b)	
#define debug7(w,x,y,z,a,b,c)
#define debug8(w,x,y,z,a,b,c,d)
#endif

#endif /* No defines ... taxc.h already included */
