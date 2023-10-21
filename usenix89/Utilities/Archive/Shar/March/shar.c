/*
 *	$Header$
 */
/********************************************************
 *							*
 *			shar.c				*
 *							*
 ********************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
/*
 *	External functions.
 */
extern char *malloc();
extern int strlen();
extern char *strcpy();
extern int strcmp();
extern int getarg();
extern stat();
extern char strcat();
extern long atol();
/*
 *	External variables.
 */
extern int optind;
extern char *optarg;
/*
 *	File tree struture:
 *
 *	  Defines files or directories, and holds the processing
 *	state flags.
 */
typedef struct Leaf {
  struct Leaf *Parent,*Sybling,*Child;	/* tree linkage */
  char *Name;				/* leaf name */
  char Flags;				/* state flags */
  u_short Mode;				/* original permissions */
  long Size;				/* file size */
} Leaf;
#define	F_FILE		01		/* file type */
#define	F_DIR		02		/* directory type */
#define	F_WALK		04		/* traversed */
#define	F_USED		010		/* written */
#define	F_BAD		020		/* unarchivable object */
/*
 *	Out file linkage.
 */
typedef struct Ref {
  struct Ref *Next;			/* pointer to next leaf reference */
  struct Leaf *Leaf;			/* associated leaf */
} Ref;
typedef struct Out {
  struct Out *Next;			/* pointer to next out file */
  short Number;				/* file number */
  long Size;				/* approximate file size */
  struct Ref *Ref;			/* list of contained leaves */
} Out;
/*
 *	Static variables.
 */
static char *RootName = NULL;		/* output file name prefix */
static long Size = 60 * 1024;		/* target file size */
static char Quiet = 0;			/* generate quiet code */
static char Check = 0;			/* check file sizes */
static char Perm = 0;			/* use original permissions */
static char OverWrite = 0;		/* overwrite existing files */
static char InStream = 0;		/* take names from standard input */
static Leaf Dot = {			/* relative tree */
  NULL,NULL,NULL,".",F_DIR};
static Leaf Abs = {			/* absolute tree */
  NULL,NULL,NULL,"",F_DIR};
static char CharBuf[128];		/* useful string buffer */
static char *SourceName = NULL;		/* original file name string */
static char PathName[1024];		/* pathname return string */
static int PathSp = 0;			/* path stack pointer */
static char *PathStack[64];		/* path name stack */
static int FileCount = 0;		/* output file number */
static Out *FileList = NULL;		/* list of output files */
static Out *CurFile = NULL;		/* current file */
static char *Terminal =			/* file terminator */
  "\\Rogue\\Monster\\";
static char Buffer[4096];		/* file data buffer */
/*
 *	Make path name:
 *
 *	  The passed tree leaf is converted to a path name string.
 *	The result is left in the global variable "PathName".
 */
static MakePathName(tre)
register Leaf *tre;
{
  /*
   *	Climb the tree, pushing file names on to the stack.
   */
  for (PathSp = 0; tre; tre = tre->Parent)
    PathStack[PathSp++] = tre->Name;
  /*
   *	Unwind the stack and form the path name.
   */
  strcpy(PathName,PathStack[--PathSp]);
  while (PathSp--){
    strcat(PathName,"/");
    strcat(PathName,PathStack[PathSp]);
  }
}
/*
 *	Parse file name:
 *
 *	  This creates an entry in the file tree from the passed leaf.
 *	A "." has special meaning in that there is no movement down
 *	the tree. An attempt to move through a name which is not a
 *	directory also causes an error. A ".." is legal if a move outside
 *	the tree top is not attempted. A pointer to the terminal leaf
 *	is returned.
 */
static Leaf *ParseFileName(tre,nam)
register Leaf *tre;
register char *nam;
{
  /*
   *	Locals.
   */
  register int i;
  /*
   *	Extract current file name.
   */
  for (i = 0; *nam && *nam != '/';)
    CharBuf[i++] = *nam++;
  CharBuf[i] = '\0';
  /*
   *	Handle the "." and ".." cases.
   */
  if (!strcmp(CharBuf,".")){
    if (*nam)
      return (ParseFileName(tre,++nam));
    return (tre);
  } else if (!strcmp(CharBuf,"..")){
    if (!tre->Parent){
      fprintf(stderr,"untraversable path name: \"%s\"\n",SourceName);
      return (NULL);
    } else if (*nam)
      return (ParseFileName(tre->Parent,++nam));
    else
      return (tre->Parent);
  } if (!(tre->Flags & F_DIR)){
    /*
     *	The current tree leaf is not a directory, give an error.
     */
    fprintf(stderr,"\"%s\" in \"%s\" is not a directory\n",CharBuf,
      SourceName);
    return (NULL);
  } else {
    register Leaf *wlk,*owk;
    /*
     *	See if the name is already in the tree.
     */
    for (owk = NULL, wlk = tre->Child; wlk; wlk = (owk = wlk)->Sybling)
      if (!strcmp(CharBuf,wlk->Name)){
        if (owk){
          owk->Sybling = wlk->Sybling;
          wlk->Sybling = tre->Child;
          tre->Child = wlk;
        }
        break;
      }
    /*
     *	If the name doesn't already exist in the tree, create it.
     */
    if (!wlk){
      struct stat fs;
      wlk = (Leaf *) malloc(sizeof(Leaf));
      wlk->Parent = tre;
      wlk->Sybling = tre->Child;
      tre->Child = wlk;
      wlk->Child = NULL;
      wlk->Name = strcpy(malloc(strlen(CharBuf) + 1),CharBuf);
      /*
       *	Form the path name and get the file stats.
       */
      MakePathName(wlk);
      stat(PathName,&fs);
      /*
       *	Only directories and files are acceptable; too late
       *	to fix things, but give an error and mark bad.
       */
      wlk->Flags = 0;
      if (fs.st_mode & S_IFDIR)
        wlk->Flags |= F_DIR;
      else if (fs.st_mode & S_IFREG){
        wlk->Flags |= F_FILE;
        wlk->Size = fs.st_size;
      } else {
        fprintf(stderr,"\"%s\" is not a file or directory\n",SourceName);
        wlk->Flags |= F_BAD;
      }
      /*
       *	Save the original permissions.
       */
      wlk->Mode = fs.st_mode;
    }
    /*
     *	If required, continue to parse the name.
     */
    if (*nam)
      return (ParseFileName(wlk,++nam));
    else
      return (wlk);
  }
}
/*
 *	Expand filter:
 *
 *	  This removes the "." and ".." objects from the directory scans.
 */
static int ExpandFilter(dir)
register struct direct *dir;
{
  /*
   *	Do gross string comparisons.
   */
  return (strcmp(dir->d_name,".") && strcmp(dir->d_name,".."));
}
/*
 *	Expand tree:
 *
 *	  The passed leaf must be a directory. This function recursively
 *	expands the directory until its bottoms out.
 */
static ExpandTree(tre)
Leaf *tre;
{
  /*
   *	Locals.
   */
  register int i,Count;
  struct direct **NameList;
  /*
   *	Get the names of all objects in the directory.
   */
  MakePathName(tre);
  SourceName = PathName;
  if ((Count = scandir(PathName,&NameList,ExpandFilter,NULL)) < 0){
    fprintf(stderr,"Cannot access \"%s\"\n",SourceName);
    tre->Flags = (tre->Flags & ~F_DIR) | F_BAD;
    return;
  } else if (!Count)
    return;
  /*
   *	See if a new entry needs to be defined. If so create it.
   *	If the object is a directory, expand its tree too.
   */
  for (i = 0; i < Count; ++i){
    register Leaf *obj;
    struct stat fs;
    for (obj = tre->Child; obj; obj = obj->Sybling)
      if (!strcmp(obj->Name,NameList[i]->d_name))
        break;
    if (!obj){
      /*
       *	Create the new entry.
       */
      obj = (Leaf *) malloc(sizeof(Leaf));
      obj->Parent = tre;
      obj->Sybling = tre->Child;
      tre->Child = obj;
      obj->Child = NULL;
      obj->Name = strcpy(malloc(strlen(NameList[i]->d_name) + 1),
        NameList[i]->d_name);
      /*
       *	Get the file status of the object.
       */
      MakePathName(obj);
      stat(PathName,&fs);
      /*
       *	Determine the file type.
       */
      obj->Flags = 0;
      obj->Mode = fs.st_mode;
      if (fs.st_mode & S_IFDIR)
        obj->Flags |= F_DIR;
      else if (fs.st_mode & S_IFREG){
        obj->Flags |= F_FILE;
        obj->Size = fs.st_size;
      } else {
        fprintf(stderr,"\"%s\" is not a file or directory\n",PathName);
        obj->Flags |= F_BAD;
      }
    }
    /*
     *	See if further expansion is required.
     */
    free(NameList[i]);
    if (obj->Flags & F_DIR)
      ExpandTree(obj); 
  }
  free(NameList);
}
/*
 *	Make out files:
 *
 *	  This runs recursively moves through the passed tree and
 *	adds file leaves to output files.
 */
static MakeOutFiles(tre)
register Leaf *tre;
{
  /*
   *	Locals.
   */
  register Leaf *wlk;
  register Ref *ref;
  /*
   *	Move through the children of the passed tree. Directories
   *	cause immediate descent and bad entries are ignored.
   */
  for (wlk = tre->Child; wlk; wlk = wlk->Sybling)
    if (wlk->Flags & F_DIR)
      MakeOutFiles(wlk);
    else if (wlk->Flags & F_FILE){
      /*
       *	See if the leaf is too big for the current file, flush.
       */
      if (CurFile && (CurFile->Size + wlk->Size) > Size){
      	CurFile->Number = ++FileCount;
      	CurFile->Next = FileList;
      	FileList = CurFile;
      	CurFile = NULL;
      }
      /*
       *	Make sure that a current file exists.
       */
      if (!CurFile){
        CurFile = (Out *) malloc(sizeof(Out));
        CurFile->Size = 0;
        CurFile->Ref = NULL;
      }
      /*
       *	Add the leaf to the current file.
       */
      ref = (Ref *) malloc(sizeof(Ref));
      ref->Next = CurFile->Ref;
      CurFile->Ref = ref;
      ref->Leaf = wlk;
      CurFile->Size += wlk->Size;
    }
}
/*
 *	Clear tree:
 *
 *	  The WALK bits in the passed tree are cleared.
 */
static ClearTree(tre)
register Leaf *tre;
{
  /*
   *	Locals.
   */
  register Leaf *wlk;
  /*
   *	Clear the current tree. Move through the children, only directories
   *	need to be cleared.
   */
  tre->Flags &= ~F_WALK;
  for (wlk = tre->Child; wlk; wlk = wlk->Sybling)
    if (wlk->Flags & F_DIR)
      ClearTree(wlk);
}
/*
 *	Check tree:
 *
 *	  This makes sure that the directory structure is in place
 *	within the current archive file.
 */
static CheckTree(stm,tre)
FILE *stm;
register Leaf *tre;
{
  /*
   *	Make sure this directory exists, and if so, is unmarked.
   */
  if (!tre || !tre->Parent || (tre->Flags & F_WALK))
    return;
  /*
   *	Handle its parent then do it.
   */
  CheckTree(stm,tre->Parent);
  MakePathName(tre);
  fprintf(stm,"if `test ! -d %s`\nthen\n  mkdir %s\n",PathName,PathName);
  if (!Quiet)
    fprintf(stm,"  echo \"mkdir %s\"\n",PathName);
  fprintf(stm,"fi\n");
  tre->Flags |= F_WALK;
}
/*
 *	Output ref:
 *
 *	  The passed reference is written to the stream, using the
 *	user's rules.
 */
static OutputRef(stm,ref)
FILE *stm;
register Ref *ref;
{
  /*
   *	Locals.
   */
  register int count;
  FILE *inp;
  /*
   *	Make sure that the tree to support this file already defined.
   */
  CheckTree(stm,ref->Leaf->Parent);
  /*
   *	Put out the existance check header if required.
   */
  MakePathName(ref->Leaf);
  if (!OverWrite)
    fprintf(stm,"if `test ! -s %s`\nthen\n",PathName);
  /*
   *	Open the input file. Echo file name written.
   */
  if (!(inp = fopen(PathName,"r"))){
    fprintf(stderr,"Unable to read \"%s\"\n",PathName);
    fprintf(stm,"  echo \"read error when SHAR-ed: %s\"\nfi\n",PathName);
    return;
  }
  if (!Quiet)
    fprintf(stm,"echo \"writing %s\"\n",PathName);
  fprintf(stm,"cat > %s << '%s'\n",PathName,Terminal);
  /*
   *	Move the file data into the archive.
   */
  while (count = fread(Buffer,sizeof(char),sizeof(Buffer),inp))
    fwrite(Buffer,sizeof(char),count,stm);
  /*
   *	Take care of the tail end linkage.
   */
  fprintf(stm,"%s\n",Terminal);
  if (!OverWrite && !Quiet)
    fprintf(stm,"else\n  echo \"will not over write %s\"\nfi\n",PathName);
  else if (!OverWrite)
    fprintf(stm,"fi\n");
  /*
   *	Handle the permission stuff.
   */
  if (Perm)
    fprintf(stm,"chmod %o %s\n",ref->Leaf->Mode & 07777,PathName);
  /*
   *	Add the checking code.
   */
  if (Check){
    fprintf(stm,"if [ `wc -c %s | awk '{printf $1}'` -ne %d ]\nthen\n",
      PathName,ref->Leaf->Size);
    fprintf(stm,
      "echo `wc -c %s | awk '{print \"Got \" $1 \", Expected \" %d}'`\nfi\n",
      PathName,ref->Leaf->Size);
  }
  /*
   *	Close the input file.
   */
  fclose(inp);
}
/*
 *	Pack archive:
 *
 *	  This turns the passed file specification into a packed archive.
 */
static PackArchive(fil)
Out *fil;
{
  /*
   *	Locals.
   */
  register Ref *ref;
  FILE *stm;
  /*
   *	Open the output file, failure here causes a return.
   */
  sprintf(CharBuf,"%s.shr%d",RootName,fil->Number);
  if (!(stm = fopen(CharBuf,"w"))){
    fprintf(stderr,"Unable to open archive: %s\n",CharBuf);
    return;
  } else
    fprintf(stderr,"\tForming archive: %s\n",CharBuf);
  /*
   *	Put out the header.
   */
  fprintf(stm,"#!/bin/sh\n");
  fprintf(stm,"# to extract, remove the header and type \"sh filename\"\n");
  /*
   *	Move through all file references and put them out.
   */
  ClearTree(&Abs);
  ClearTree(&Dot);
  for (ref = fil->Ref; ref; ref = ref->Next)
    OutputRef(stm,ref);
  /*
   *	Close the output file before leaving.
   */
  if (!Quiet)
    fprintf(stm,"echo \"Finished archive %d of %d\"\n",fil->Number,FileCount);
  fprintf(stm,"exit\n");
  fclose(stm);
}
/*
 *	Main body:
 *
 *	  This parses the command line and executes its directives.
 */
main(argc,argv)
int argc;
char *argv[];
{
  /*
   *	Locals.
   */
  register int c;
  Leaf *tre;
  /*
   *	Command line switches:
   *
   *	-c			=	add checking code
   *	-f <name>		=	output file name root
   *	-i			=	take names from standard input
   *	-m <number>		=	sets the target segment size
   *	-o			=	overwrite existing files
   *	-p			=	use original permissions
   *	-q			=	generate quite code
   */
  while ((c = getarg(argc,argv,"cf:im:opq")) != EOF)
    switch (c){
      case 'c':
        Check = 1;
        break;
      case 'f':
        RootName = optarg;
        break;
      case 'i':
        InStream = 1;
        break;
      case 'm':
        Size = atol(optarg) * 19;	/* reserve 5% for overhead */
        Size /= 20;
        break;
      case 'o':
        OverWrite = 1;
        break;
      case 'p':
        Perm = 1;
        break;
      case 'q':
        Quiet = 1;
        break;
      case '?':
        fprintf(stderr,"Unknown switch: -%c\n",c);
        break;
      default:
        /*
         *	File names can be absolute or relative, decide which
         *	tree the filename goes in.
         */
        SourceName = optarg;
        if (*optarg == '/')
          tre = ParseFileName(&Abs,++optarg);
        else
          tre = ParseFileName(&Dot,optarg);
        /*
         *	If the name results in a directory, expand the directory.
         */
        if (tre && (tre->Flags & F_DIR))
          ExpandTree(tre);
    }
  /*
   *	Do tests, additional tree building if required.
   */
  if (!RootName){
    fprintf(stderr,"A \"-f <filename>\" must be specified\n");
    exit(1);
  } else if (InStream){
    char buf[1024];
    while (scanf("%s",buf) == 1){	/* read names from stdin */
      SourceName = buf;
      if (buf[0] == '/')
        tre = ParseFileName(&Abs,&buf[1]);
      else
        tre = ParseFileName(&Dot,buf);
      /*
       *	If a directory, expand it.
       */
      if (tre && (tre->Flags & F_DIR))
        ExpandTree(tre);
    }
  }
  /*
   *	Construct the list of output files.
   */
  MakeOutFiles(&Abs);
  MakeOutFiles(&Dot);
  if (CurFile){
    CurFile->Number = ++FileCount;
    CurFile->Next = FileList;
    FileList = CurFile;
  }
  fprintf(stderr,"***  Archive contains %d files  ***\n",FileCount);
  /*
   *	Move through the file list and pack up the data.
   */
  for (CurFile = FileList; CurFile; CurFile = CurFile->Next)
    PackArchive(CurFile);
}
