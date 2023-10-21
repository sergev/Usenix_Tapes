

/******************************************************************/
/*     Directory repack                                           */
/*     written by Rich Karas for the public domain                */
/*     rev 1.0 1/23/86                                            */
/*                                                                */
/*                                                                */
/*                                                                */
/******************************************************************/

#include "stdio.h"
#include "customiz.h"
#define MAXSIZE 16383
#define MINSIZE 8192
#define MAXDIRS 512
char fat[MINSIZE];
     /*  these are global variables */

int drive,num_sect,begin,status,error,directory[MAXSIZE];
unsigned int fat_strt,dir_strt,data_area,dir_len;
unsigned int byte_sect,sect_unit,reserv_strt,no_fat,fat_sect,no_root;
unsigned int sect_image,media,sect_track,no_heads,no_hidden;
int *bufptr;
#define UNDERL "\033\133\064\155\0"
#define NORMAL "\033\133\155\0"
#define REVERSE "\033\133\067\155\0"
#define BOLD "\033\133\061\155\0"
#define BLINK "\033\133\065\155\0"
#define CLRSCR "\033\133\062\112\0"
#define ERASELN "\033\133\113\0"
#define XTIME 30
#define CPOS1 "\033\133\065\073\062068\110\0"
/******************************************************************/
/*                                                                */
/*    Introduction is a procedure that prints using ansii         */
/*    ESCAPE codes to implement blinking and cusor position.      */
/*                                                                */
/*                                                                */
/*                                                                */
/******************************************************************/

intro()             /* introduces who we are */
{
 printf("%s",CLRSCR);
 printf("%s",CPOS1);
 printf("%s%s    DIRECTORY REPACK %s\n",BOLD,BLINK,NORMAL);
 printf("\n\n\n\n\n\n");
 printf("                      Rich Karas\n");
 printf("                      14 Wells Road\n");
 printf("                      Flemington,NJ 08822\n");
 printf("                      Voice 201-782-0639\n");
 printf("                      Data  201-782-7640\n");
 printf("\n\n\n\n\n");
 printf("            Public Domain Software 1986, Richard Karas \n");
 printf("                        REV 1.0 1/23/86");
 sleep(XTIME);
 printf("%s",CLRSCR);
}
/******************************************************************/
/*                                                                */
/*     Select drive is an assembly language program which         */
/*     calls DOS system functions to find out how many drives     */
/*     are present.                                              */
/*                                                                */
/*                                                                */
/******************************************************************/

select_drive()
{
  register int x;
#asm
     push ds
     push ss
     push ax
     push dx
     mov  ah,19h
     int 21h
     mov dl,al
     mov ah,0eh
     int 21h
     mov di,ax
     pop dx
     pop ax
     pop ss
     pop ds
#endasm
    return( x & 0177);
}
/******************************************************************/
/*                                                                */
/*      Reset dsk does just what it says,using                    */
/*      another DOS system call.                                  */
/*                                                                */
/*                                                                */
/*                                                                */
/******************************************************************/


reset_dsk()
{
#asm
     push ds
     push ss
     push ax
     push dx
     mov  ah,0dh
     int 21h
     pop dx
     pop ax
     pop ss
     pop ds
#endasm
}
/******************************************************************/
/*                                                                */
/*      Disk read performs absolute disk reads through            */
/*      the system calls. IT reads X number of sectors begining   */
/*       at sector specified.                                      */
/*                                                                */
/*                                                                */
/******************************************************************/


disk_read()  /* routine to perform absolute disk read */
{
#asm
     push ds
     push ax
     push bx
     push cx
     push dx
     mov  ax,DS:WORD PTR drive
     mov  bx,offset bufptr
     mov  dx,[bx]
     mov  bx,dx
     mov  cx,DS:WORD PTR num_sect
     mov  dx,DS:WORD PTR begin
     int  25h
     pushf
     mov  DS:WORD PTR error,ax
     pop  ax
     mov  DS:WORD PTR status,ax
     popf
     pop  dx
     pop  cx
     pop  bx
     pop  ax
     pop  ds
#endasm
}
/******************************************************************/
/*                                                                */
/*            Same as read, but writes to disk.                   */
/*                                                                */
/*                                                                */
/*                                                                */
/*                                                                */
/******************************************************************/

disk_write()  /* routine to perform absolute disk write */
{
#asm
     push ds
     push ax
     push bx
     push cx
     push dx
     mov  ax,DS:WORD PTR drive
     mov  bx,offset bufptr
     mov  dx,[bx]
     mov  bx,dx
     mov  cx,DS:WORD PTR num_sect
     mov  dx,DS:WORD PTR begin
     int  26h
     pushf
     mov  DS:WORD PTR error,ax
     pop  ax
     mov  DS:WORD PTR status,ax
     popf
     pop  dx
     pop  cx
     pop  bx
     pop  ax
     pop  ds
#endasm
}
/******************************************************************/
/*                                                                */
/*        Operator enters the disk dirve to repack.               */
/*                                                                */
/*                                                                */
/*                                                                */
/*                                                                */
/******************************************************************/

prompt(k)  /* prompts for operator entry of drive to dir repack */
int k;
{
  char j,num;
  int i;
  j=65;
  drive=0;
  printf("Please select disk drive to repack directories \n");
  printf("          ");
  for(i=0;i<k;i++)
     {  printf("  %c",j);
        j=j+1;
        }
  printf("\n Drive = ");
  scanf("%c",&num);      /* gets operator value and converts to upper */
  num=toupper(num);
  printf("\n\n");
  i=num-65;
  if(i>k) {
      printf(" drive selected is not configured in system");
      exit(0);
      }
  else
     drive=i;
}
/******************************************************************/
/*         This procedure analysis the media discription and      */
/*         performs some math. It also and displays these         */
/*         attributes.                                            */
/*         It reads the boot sector byte offset 11-29.            */
/*                                                                */
/*                                                                */
/******************************************************************/


attrib()
{
  double val1[1],val2[1],val3[1],val4[1],val5[1];
  char l1[100];
  int temp,i;
  fat_sect=directory[11];      /* ref dos manual for explanation */
  sect_track=directory[12];
  no_heads=directory[13];
  no_hidden=directory[14];
  reserv_strt=directory[7];
  media=(directory[10] & 0xff00)>>8;
  no_fat=directory[8] & 0x00ff;
  sect_unit=(directory[6] & 0xff00)>>8;
  temp=(directory[6] & 0x00ff)<<8;
  byte_sect=temp;
  temp=(directory[9] & 0x00ff)<<8;
  no_root=temp + ((directory[8] & 0xff00)>>8);
  temp=(directory[10] & 0x00ff)<<8;
  sect_image=temp + ((directory[9] & 0xff00) >>8);
  int2bcd(val1,sect_image);
  int2bcd(val4,byte_sect);
  bmul(val5,val4,val1);
  format(l1,val5);


     /* do some math to find attributes */
  fat_strt=reserv_strt;
  dir_strt=fat_strt+(fat_sect*no_fat);
  if (no_root >= 1024) {
     dir_len=no_root/byte_sect;
     dir_len=dir_len*32;
  } else {
     dir_len=((no_root*32)/byte_sect);
  }
  data_area=dir_strt+dir_len;
     /* display media attributes */
  printf("%s",CLRSCR);
  printf("\n  Media description and capacity");
  printf("\n\n");
  printf("Boot area starts at sector   0\n");
  printf("FAT tables begin at sector %d\n",fat_strt);
  printf("Directory begins at sector %d\n",dir_strt);
  printf("Data area begins at sector %d\n",data_area);
  printf("Total sectors on the disk is %d\n",sect_image);
  printf("Number of sectors per track is %d\n",sect_track);
  printf("Number of heads used to read/write is %d\n",no_heads);
  printf("Media byte descriptor is %x\n",media);
  printf("Sectors per allocation unit is %d\n",sect_unit);
  puts("Total bytes of storage are ");
  puts(l1);
}
/******************************************************************/
/*                                                                */
/*      Determines if absolute disk read/writes were valid,       */
/*        aborts if found in error.                               */
/*                                                                */
/*                                                                */
/*                                                                */
/******************************************************************/

valid()  /* check status for bit 0=1 or 0 than print error if 1 */
{

   status=status & 1;
   if(status==1) {
      printf("\n Error detected during disk read/write \n");
      switch (error) {
         case 0:
              printf("\n\n Attempt to write to protected disk");
              exit(1);
         case 1:
              printf("\n\n Unknown unit ");
              exit(1);
         case 2:
              printf("\n\n Drive not ready ");
              exit(1);
         default :
              printf("\n\n Error type %d check DOS manual \n\n",error);
              exit(1);
              }
       }
}
/******************************************************************/
/*                                                                */
/*    This routine repacks the root directory only.               */
/*         It calls the proceedure subrepack if it                */
/*         finds a subdirectory. Since the root directory is      */
/*        a fixed number of entries, I coded this as standalone.  */
/*                                                                */
/******************************************************************/

repack()
{
    int firstbyte,attribute,i,n,j,true,more;
    char name[12];
    unsigned int cln,count;
    more=0;
    count=1;
    true=1;
    n=0;
    printf("\n\nAanlyzing & packing ROOT directory \n");
    firstbyte=directory[n]&0x00ff;
    attribute=(directory[n+5]&0xff00)>>8;
    while(true) {
       if(attribute ==0x0010 && firstbyte != 0x00e5) {
            disk_write();                        /* found a subdir */
            for(i=0,j=0;i<=8;i=i+2,j++) {
               name[i]=directory[n+j]&0x00ff;
               name[i+1]=(directory[n+j]&0xff00)>>8;
               }
            name[10]=directory[n+5]&0x00ff;
            name[11]='\0';
            i=directory[n+13]&0x00ff <<8;
            j=directory[n+13]&0xff00 >>8;
            cln=i+j;
            n=n+16;
            firstbyte=directory[n]&0x00ff;
            attribute=(directory[n+5]&0xff00)>>8;
            subrepack(&name,cln,5);
            bufptr=&directory;
            num_sect=dir_len;
            begin=dir_strt;
            disk_read();
            valid();
       }else{
            if(firstbyte==0x00e5) {
              more=look(no_root,count,n);
              if(more==1){            /* move up if more valid entries */
                moveup(n,no_root,count);
                firstbyte=directory[n]&0x00ff;
                attribute=(directory[n+5]&0xff00)>>8;
              }else
                true=0;
            }else {
                 n=n+16;      /* keep looking for deletes until done */
                 firstbyte=directory[n]&0x00ff;
                 attribute=(directory[n+5]&0xff00)>>8;
               }
       }
       count=count+1;
       if(firstbyte ==0 || count > no_root)
          true=0;
    }
    disk_write();
    valid();
    printf("Root directory repack complete\n\n");
}
/******************************************************************/
/*                                                                */
/*     This is a recursive routine which I've tested to a depth   */
/*     of five subdiredctories. Basically it performes the same   */
/*     as the root directory repack routine with the exception    */
/*     of unlimites number of directory entries.                  */
/*                                                                */
/******************************************************************/

subrepack(name,cluster,tab)
char name[];
int tab;
unsigned int cluster;
{
    int firstbyte,attribute,i,j,n,l,true,more;
    unsigned int ctr,logical[32],next,offset,newclu,count,no_of_dir;
    double val1[1],val2[1],val3[1],val4[1];
    char newname[12];
    more=0;
    n=0;
    true=1;
    for(i=1;i<tab;i++)
       printf(" ");
    printf("Working on subdirectory %s\n",name);
    for(i=0;i<32;i++)
       logical[i]=0;
    i=0;
cont: logical[i]=((cluster-2)*sect_unit)+data_area;
    s2bcd(val2,"1.5");             /* this is where we convert cluster */
    int2bcd(val1,cluster);        /* to logical sector number and */
    bmul(val3,val1,val2);         /* figure out the offset into the FAT */
    s2bcd(val4,".4");
    bsub(val1,val3,val4);
    offset=bcd2int(val1);
    if(cluster % 2 ==0)  {
       next=(fat[offset+1] & 0x0f) <<8;
       next=next +fat[offset];
    } else {
           next=(fat[offset] &0xf0) >>4;
           next=next + (fat[offset+1] << 4);
           }
    cluster=next;
    i=i+1;
    if(cluster < 0xff7)
      goto cont;    /* now can read new subdir and proces */
      num_sect=sect_unit;
      ctr=i;
      n=0;
      no_of_dir=(16*sect_unit)*(ctr);
      count=1;
      for(j=0;j<ctr;j++) {       /* get the subdirectory data */
         begin=logical[j];
         bufptr=&directory[n];
         disk_read();
         valid();
         n=n+((sect_unit*byte_sect)/2);
         if(n > MAXSIZE) {
             printf("\n\n To many files for anyone to control !! ");
             exit(1);
             }
       }
       n=32;           /* same as root directory routine */
       firstbyte=directory[n]&0x00ff;
       attribute=(directory[n+5]&0xff00)>>8;
       while(true) {
            if(attribute == 0x0010 && firstbyte != 0x00e5 ) {
                for(j=0,i=0;j<ctr;j++) {
                   begin=logical[j];
                   bufptr=&directory[i];
                   disk_write();
                   valid();
                   i=i+((sect_unit*byte_sect)/2);
                   }
                for(i=0,j=0;i<=8;i=i+2,j++) {
                   newname[i]=directory[n+j]&0x00ff;
                   newname[i+1]=(directory[n+j]&0xff00)>>8;
                   }
                newname[10]=directory[n+5]&0x00ff;
                newname[11]='\0';
                i=directory[n+13]&0x00ff<<8;
                j=directory[n+13]&0xff00>>8;
                newclu=i+j;
                n=n+16;
                firstbyte=directory[n]&0x00ff;
                attribute=(directory[n+5]&0xff00)>>8;
                subrepack(&newname,newclu,tab+5);
                for(j=0,i=0;j<ctr;j++) {
                   begin=logical[j];
                   bufptr=&directory[i];
                   disk_read();
                   valid();
                   i=i+((sect_unit*byte_sect)/2);
                   }
            } else {
                   if(firstbyte==0x00e5) {
                       more=look(no_of_dir,count,n);
                       if(more==1){
                          moveup(n,no_of_dir,count);
                          firstbyte=directory[n]&0x00ff;
                          attribute=(directory[n+5]&0xff00)>>8;
                          }else
                             true=0;
                       } else {
                              n=n+16;
                              firstbyte=directory[n]&0x00ff;
                              attribute=(directory[n+5]&0xff00)>>8;
                              }
                    }
                    count=count+1;
                    if(firstbyte ==0 || count > no_of_dir)
                       true=0;
       }
       for(j=0,i=0;j<ctr;j++) {
           begin=logical[j];
           bufptr=&directory[i];
           disk_write();
           valid();
           i=i+((sect_unit*byte_sect)/2);
           }
       for(i=0;i<tab;i++)
          printf(" ");
       printf("Repacking complete on %s\n",name);
}
/******************************************************************/
/*                                                                */
/*      Routine that deletes unused directory entries             */
/*      located in the middle of valid entries, By moving         */
/*      valid entries up.                                         */
/*                                                                */
/*                                                                */
/******************************************************************/


moveup(start,no_of_dir,prescnt)
int start;
unsigned int no_of_dir,prescnt;
{
    int i,true;
    true=1;         /* get rid of deleted by moving valid entries up */
    do{
       for(i=0;i<=15;i++)
           directory[start+i]=directory[start+i+16];
       start=start+16;
       prescnt=prescnt+1;
       if((directory[start]&0x00ff) == 0 || prescnt > no_of_dir)
         true=0;
    }while(true);
}

/******************************************************************/
/*                                                                */
/*       Routine to look ahead  and determine if additional       */
/*            valid directory entries exist                       */
/*                                                                */
/*                                                                */
/*                                                                */
/******************************************************************/

int look(maxentries,start,ent1)
unsigned int maxentries,ent1;
int start;
{
  unsigned int i,q;
  q=0;
  for(i=start;i<=maxentries;i++) {
   if((directory[ent1]&0x00ff)== 0x00e5 ||(directory[ent1]&0x00ff)==0x0000)
    ;
   else                 /* if no more valid entries at end of dir */
        q=1;            /* than don't do anymore (lookahead) */
   ent1=ent1+16;
   }
   return(q);
}
/******************************************************************/
/*     Main code which calls the above proceedures                */
/*        This also analyzes the input parameters to              */
/*            determine if operator prompt is required.           */
/*                It will also analyze the media data and         */
/*                    abort the program if it is invalid.         */
/*                                                                */
/******************************************************************/


main(argc,argv)
int argc;
char *argv[];
{
    int num,point;
    char media1;
    intro();
    reset_dsk();              /* introduce the writer */
    num=select_drive();   /* how many drives in system */
    num_sect=1;
    begin=0;
    bufptr=&directory;    /* read sector 0 to get device attributes */
    drive=0;
    if(argc ==1)      /* did the operator pass parameters */
       prompt(num);
    else {
         point=argv[1] [0];     /* if yess don't prompt */
         point=toupper(point);
         point=point-65;
         if(point>=0 && point <=num)
            drive=point;
         else {
              printf("Usage : Dskrpk [drive],or drive not there");
              exit(1);
              }
         }
    disk_read();
    valid();
    attrib();
               /* display info about drive selected */
    if(no_root > 1024) {
        printf("\nThis program supports IBM floppy,10mb,20mb drives only");
        exit(1);
        }
    bufptr=&fat;
    num_sect=fat_sect;
    begin=fat_strt;
    disk_read();           /* get fat information */
    valid();
    media1=fat[0];
    if(media != media1) {
      printf(" \n\n Boot sector and FAT sector media disagree");
      printf(" \n     I cannot repack this drive ");
      exit(1);
      }
    bufptr=&directory;
    begin=dir_strt;
    num_sect=dir_len;
    disk_read();       /* get root directory information */
    valid();
    repack();        /* call to repack directories */
    exit(0);
}
