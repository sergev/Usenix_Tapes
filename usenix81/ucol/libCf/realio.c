/*
      Copyright (C) Duncan Carr Agnew 1980
*/
#define MAXOPN 16	/*maximum number of channels open*/
#include <stdio.h>
long lbt[MAXOPN];	/*array holding position last reached, for each channel*/

realio_(list,rlx,rhx,name,nrecx,dum1)
/* fortran callable routine for reading or writing diskfiles, from */
/* data in array list to positions rlx through rhx in file name */
/* for nrec 0 read from file; positive, create file and write to it; */
/* negative, write on file already in existence. (After first call in */
/* a program, nrec>0 is equivalent to nrec<0, unless a call has been */
/* made to return). Files may be integer or real; integer files are */
/* read-only for realio; they may be written or read with intio */
/* (below) */
/*   calls routines getfd, namtyp, ctype */
char name[];		/*filename*/
float *rlx,*rhx;	/*positions in file to read or write*/
float list[];		/*array of numbers to read or write*/
int *nrecx;		/*variable giving course of action*/
long dum1;		/*UNIX dummy variable*/
{
int ity,iloc,lun,nb,nbr,nrec,i,j,nbuf,nbl;
int ibuf[256];		/*buffer to hold integers when reading an integer file*/
long rl,rh,sp,offset,lep;

rl = *rlx;
rh = *rhx;
nrec = *nrecx;
ity = namtyp(name);	/*check type of filename (-1 real, 1 integer, 0 illegal)*/
if(ity==0){
   fprintf(stderr,"Filename %s is illegal. Realio aborts.\n",name);
   exit(15);
}
if(ity>0 && nrec!=0){	/*cannot write to integer file*/
   fprintf(stderr,"File %s is integer, and realio cannot write to it. Program aborts.\n",name);
   exit(15);
}
lun = getfd(name,nrec,&iloc);   /* get file descriptor; iloc is position of name in array lbt*/
lep = lbt[iloc];		/*last byte read or written*/
if(ity>0){
   sp = 2*(rl-1);		/*start byte in integer file*/
   nb = 2*(rh-rl+1);		/*number of bytes to read*/
   nbuf = (nb-1)/512;		/*number of buffer loads*/
   nbl = nb - 512*nbuf;		/*size of last load*/
}
else{
   sp = 4*(rl-1);		/*start byte in real file*/
   nb = 4*(rh-rl+1);		/*number of bytes to read*/
}
offset = sp-lep;		/*number of bytes to move from last place in file*/
if(offset< -lep/2)
   lseek(lun,sp,0);		/*seek from start of file, */
else
   lseek(lun,offset,1);		/*or last position, whichever is closer */
if(nrec!=0){
   nbr = write(lun,list,nb);		/*write reals out*/
   if(nb!=nbr){
      fprintf(stderr,"Error in writing to real file %s\n",name);
      fprintf(stderr,"Terms to be written were %ld through %ld, or %d bytes.\n",rl,rh,nb);
      fprintf(stderr,"%d bytes actually written. Program aborts.\n",nbr);
      exit(15);
   }
}
else if(ity<0){
   nbr = read(lun,list,nb);		/*read reals in */
   if(nb!=nbr){
      fprintf(stderr,"Error in reading from real file %s\n",name);
      fprintf(stderr,"Terms to be read were %ld through %ld, or %d bytes.\n",rl,rh,nb);
      fprintf(stderr,"%d bytes actually read. Program aborts.\n",nbr);
      exit(15);
   }
}
else{
   for(i=0; i<=nbuf; i++){		/*read integers in*/
      if(i!=nbuf){
         nbr = read(lun,ibuf,512);	/*read full buffer*/
         if(nbr!=512){
            fprintf(stderr,"Error in realio in reading from integer file %s\n",name);
            fprintf(stderr,"Terms to be read were %ld through %ld.\n",rl,rh);
            fprintf(stderr,"Buffer load %d should have contained 512 bytes\n",i+1);
            fprintf(stderr,"%d bytes actually read. Program aborts.\n",nbr);
            exit(15);
         }
         for(j=0;j<=255;j++)		/*transfer data*/
            list[256*i+j] = ibuf[j];
      }
      else{
         nbr = read(lun,ibuf,nbl);	/*read last (partial) buffer*/
         if(nbr!=nbl){
            fprintf(stderr,"Error in realio in reading from integer file %s\n",name);
            fprintf(stderr,"Terms to be read were %ld through %ld.\n",rl,rh);
            fprintf(stderr,"Final buffer load (no. %d)",nbuf+1);
            fprintf(stderr," should have contained %d bytes\n",nbl);
            fprintf(stderr,"%d bytes actually read. Program aborts.\n",nbr);
            exit(15);
         }
         for(j=0; j<nbl/2; j++)
            list[256*nbuf+j] = ibuf[j];		/*transfer data*/
      }
   }
}
lbt[iloc] = sp+nb;		/*store place stopped at for later use*/
return;
}

intio_(list,rlx,rhx,name,nrecx,dum1)
/* fortran callable routine for reading and writing integers on disk */
/* arguments as for realio (above), except that list is integer*2 rather */
/* than real, and the filename must be integer */
char name[];
int list[];
float *rlx,*rhx;
int *nrecx;
long dum1;
{
int ity,iloc,lun,nb,nbr,nrec;
long rl,rh,sp,offset,lep;

rl = *rlx;
rh = *rhx;
nrec = *nrecx;
ity = namtyp(name);
if(ity==0){
   fprintf(stderr,"Filename %s is illegal. Intio aborts.\n",name);
   exit(15);
}
if(ity<0){	/*Cannot read or write to real file*/
   fprintf(stderr,"File %s is real, and intio cannot access it. Program aborts.\n",name);
   exit(15);
}
lun = getfd(name,nrec,&iloc);
/* find place to start, and seek to there, either from start of file */
/* or last place touched within it */
lep = lbt[iloc];
sp = 2*(rl-1);
nb = 2*(rh-rl+1);
offset = sp-lep;
if(offset< -lep/2)
   lseek(lun,sp,0);
else
   lseek(lun,offset,1);
if(nrec!=0){
   nbr = write(lun,list,nb);		/*write integers*/
   if(nb!=nbr){
      fprintf(stderr,"Error in writing to integer file %s\n",name);
      fprintf(stderr,"Terms to be written were %ld through %ld, or %d bytes.\n",rl,rh,nb);
      fprintf(stderr,"%d bytes actually written. Program aborts.\n",nbr);
      exit(15);
   }
}
else{
   nbr = read(lun,list,nb);		/*read integers*/
   if(nb!=nbr){
      fprintf(stderr,"Error in intio in reading from integer file %s\n",name);
      fprintf(stderr,"Terms to be read were %ld through %ld, or %d bytes.\n",rl,rh,nb);
      fprintf(stderr,"%d bytes actually read. Program aborts.\n",nbr);
      exit(15);
   }
}
if(nbr==nb)
   lbt[iloc] = sp+nb;		/*store place last stopped at*/
return;
}
