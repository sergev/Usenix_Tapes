#
/*************************************************************/
/*  ********                                                 */
/*  * rtun *                                                 */
/*  ********                                                 */
/*                                                           */
/* status: 28 oct 75 version 1.                              */
/* modified: jan 85 J. Kourlas; for VAX.                     */
/*          compile with -DVAX flag                          */
/* author: t.forgacs univ of nijmegen comp. graphics         */
/* compiler: c unix version 6.                               */
/* entrypoints: rtmount rtumount rtopen rtclose              */
/*              rtread  rtwrite  rtseek rtmksp rtdelete      */
/* function: this subr.package makes possible to handle      */
/*          rt-11 files under unix                           */
/*          rtmount: establishes a device on which the rt-11 */
/*                   filestructure resides                   */
/*          rtumount:deestablishes the device                */
/*          rtopen: open rt-11 file for read,write,update    */
/*          rtclose: closes an open rt-11 file               */
/*          rtread: sequential read                          */
/*          rtwrite: sequential write                        */
/*          rtseek: locates read-write pointer               */
/*          rtmksp: rt-11 garbage collection                 */
/*          rtdelete: delete rt-11 file by name              */
/* calling sequences,parameters,returns:                     */
/*        rtmount(name,rwflag)                               */
/*          char *name; special-file name                    */
/*          int  rwflag; 0:mount for read, 1:write, 2:update */
/*          returns: unix-filedescriptor for the spec file   */
/*                   or -1 if rtmount fails                  */
/*        rtumount();                                        */
/*        rtopen(name,mode)                                  */
/*          char *name; rt-11 filename                       */
/*          int  mode;0:read, 1:write, 2:update              */
/*          returns: rt-filedescripror used in rtread etc    */
/*                   or -1 if open fails                     */
/*        rtclose(fildes)                                    */
/*          int  fildes; filedescriptor                      */
/*        rtread(fildes,buffer,nbytes)                       */
/*          int fildes; filedescriptor                       */
/*          char *buffer;                                    */
/*          int  nbytes; number of bytes to be read          */
/*          returns  number of bytes actually  read          */
/*                   0 if end of file was reached            */
/*                   0r -1 if read fails                     */
/*        rtseek=== to be implemented later                  */
/*        rtmksp=== to be implemented later                  */
/*        rtdelete(name)                                     */
/*         char  *name; rt-11 file to be deleted             */
/*         returns: -1 if fails, 0 otherwise                 */
/* externals: rtun calls the following sysroutines(emt):     */
/*              open close read write seek                   */
/* notes: as a default convevtion: at most 4 files can be    */
/*         open and only one of them for write(update).      */
/*         to change that replace #define numop 3 (right     */
/*         after this documentation) with #define numop n-1  */
/*         wher n is the wanted number of max open files     */
/*************************************************************/

#define numop 3
#define readfl 0
#define writfl 1
#define updatfl 2
#define phread 0
#define phwrite 1

#ifdef VAX
#define r_short short
#else
#define r_short int
#endif

/************************************************************/
/* definition of external(global) data and functions
used by the routines*/
/*************************************************************/

                      /*---rtinode(rttabl)---*/

struct rtinode /* data of open files */
{  r_short pblkn; /* phis start blocknumb */
   r_short filsiz; /*filesize in blocks */
   r_short seekp[2]; /* seekpointer-offset: block,byte resp */
   r_short curend[2];/*current endpointer for file to be written*/
   r_short dira[2];/*dir adr block byte resp.*/
   r_short opflg; /* openflag 0,1,2 for read,write,upd and -1 for closed*/
} rttabl[numop+2];
/*rttabl[numop+1] is for old file to be deleted after closing
the one created instead of */

                      /*---dev(rtdev)---*/

struct dev /*data for the mounted device */
{  int fdes; /* unix file descriptor for mounted spec file */
   int rwflag; /* see rtmount */
} rtdev = {-1,0};

                      /*---cseek(rtcurseek)---*/

struct cseek /* current physical seekpointer */
{  int sblkn; /*blocknumb */
   int sbyte /*bytenumb */;
} rtcurseek;

                      /*---dirs(rtdirtab)---*/

struct dirs /*for directory searche */
/* all data are in physical discadress*/
{ r_short dirad[2];/*current searchadr block byte resp. */
  r_short filst; /*startblock of current file */
  r_short senumb[2]; /*number of opened segm; max, current resp. */

  r_short rtextb; /*number of extrabytes per entry */
  r_short curs; /*current dir segment */
  r_short nexts; /*next dir segment */
}rtdirtab;

                      /*---workarea---*/

char rtbuffer[1024]; /*internal buffer */
int  rtwrop  = -1; /* fildes. for write file or -1 if none */
int rtoldfl = 0;/*set if there is old fil to be deleted at close*/
int rtsegfull[31]; /*see rtserdir() func */
r_short rtextb;/*extrabytes per entry in dirsegm, containing
           the file to be written */



                    /*---rtconvrad---*/

rtconvrad(ascii,radix) char *ascii;  r_short *radix;
/* converts at most 3 ascii char to rad50. conversion stops
after the third char or after the first not in radix-set one
in the latter case the missing ones are taken to be space
returns the number of really conv chars */

{int i,retur; char temp[3];
retur=3;
for (i=0;i!=3;i++)
{if (retur != 3) {temp[i]=0;continue;}
  switch (ascii[i])
  { case 040: temp[i]=0;break;
    case 044: temp[i]=033;break;
    case 056: temp[i]=034;break;
    default: if (ascii[i]<=0132 && ascii[i]>=0101)
               temp[i]= ascii[i]-0100;
             else if (ascii[i]<=071 && ascii[i]>= 060)
               temp[i]= ascii[i]-022;
             else if(ascii[i]<=0172 &&ascii[i]>=0141)
               temp[i]=ascii[i]-0140;
             else {retur=i; temp[i]=0;}
  }/*switch*/
}/*for*/

*radix=temp[0]*050*050+temp[1]*050+temp[2];
return(retur);
}/*rtconvrad*/

                      /*---rtgetname---*/

rtgetname(name,radname)  char *name; r_short *radname;
/*converts name (ascii rt11-filename) into radname(radix50)
suitable for rt11 dictionary, i.e. 2 words for filnam 1 word
for ext,padded with spaces */

{ char  tempname[9];/*for name extended to 9 bytes */
   int i,j,retur;/*returned value: 0 o.k. -1 invalid name */
   for (i=9; i--;) tempname[i]=' ';/*fill with spaces */
   j=0;
   for (i=0; name[i] != 0;i++)
     {if(i==10)return(-1);
       if (name[i]=='.')
         if(i>6)return(-1);
         else {j=6; continue;}
       tempname[j]=name[i];j++;
     }
   j=0;
   for (i=0; i!=3;i++)
     {if(rtconvrad(tempname+j,radname+i) !=3) return(-1);
       j += 3;
     }
   return(0);
}
                      /*---rtphio---*/

rtphio(iofl,buffer,nbytes,endblk)
char *buffer;
/* this routine does phisical i0
  reads(iofl=0) or writes(iofl=1) nbytes bytes to (from) buffer
to(from) rt11 device from current dev seekpointer(rtcurseek)
io stops after read(written) nbytes bytes or reaching the endblk
physical blocknumber. after completion modifies rtcurseek.
  returns : number of read(written) bytes or
0 if end of file (endblk reached) or
-1 if io fails */

{int maxb;/*max numb of bytes could be read in */
 int actb;/*numb of bytes actually read in */
 int compb;/*number of free bytes on current block*/
if (endblk<= rtcurseek.sblkn) return(0);
compb=512-rtcurseek.sbyte; 
maxb= (endblk-rtcurseek.sblkn)*512-rtcurseek.sbyte;
if (nbytes==0) return(0);
if (maxb>0 && nbytes >maxb)nbytes=maxb;
if (iofl==0) /*test read or write*/
  {if ((actb=read(rtdev.fdes,buffer,nbytes))== -1)return(-1);}
else
  if((actb=write(rtdev.fdes,buffer,nbytes))== -1)return(-1);
/* here we compute new rtcurseek */
if (actb < compb) {rtcurseek.sbyte += actb;return(actb);}
actb -= compb;
rtcurseek.sblkn += actb/512+1;
rtcurseek.sbyte= actb % 512;
return(actb+compb);
} /*rtphio end*/


                      /*---rtpseek---*/

rtpseek(bloknu,bytnu)
/*if rtcurseek differs from bloknu,bytnu then makes a seek
after that modifies rtcurseek */
{ if (rtcurseek.sblkn != bloknu || rtcurseek.sbyte != bytnu)
        {
#ifdef VAX
        lseek(rtdev.fdes,bloknu*512 + bytnu,0);
#else
        seek(rtdev.fdes,bloknu,3);seek(rtdev.fdes,bytnu,1);
#endif
        rtcurseek.sblkn=bloknu;rtcurseek.sbyte=bytnu;
        }
return;
}


                    /*---rtheader---*/

rtheader()/* processes rtheader of dir segments returns filestart-
           adress referred by this segment */
{int d;  char *cbuff;/*to use rtbuffer as char-array*/
  cbuff=rtbuffer;
  rtpseek(rtdirtab.nexts,0);
  rtphio(0,cbuff,10,rtdirtab.nexts+2);/*read rtheader*/
  rtdirtab.curs=rtdirtab.nexts;
  rtdirtab.dirad[0]=rtdirtab.nexts;
  rtdirtab.dirad[1]=10;
  d=(rtdirtab.filst=((r_short *)rtbuffer)[4]);
  if (rtdirtab.nexts==6) /*first segment*/
    {rtdirtab.senumb[0]=((r_short *)rtbuffer)[0];
      rtdirtab.senumb[1]=((r_short *)rtbuffer)[2];
    }
  rtdirtab.nexts=6+2*(((r_short *)rtbuffer)[1]-1);
  rtdirtab.rtextb=((r_short *)rtbuffer)[3];
  return(d);
}

                      /*---rtinitdir---*/

rtinitdir() /*init directory search */
{ rtdirtab.curs=4;/*not real blocknumber since befor inited search
                  ther is no current segment*/
  rtdirtab.nexts=6;
  rtheader();
  return;
}


                     /*---rtserdir---*/

rtserdir(radname,filst,size,date,dirad)
/*provides the attributes of the next directory entry */
/*we implemented a side function here wich  is usefull
for the rtadjust() function,i.e: this function fills out
the rtsegfull table, for rtsegfull[i]=o if the i-th segment
is not full and 1 if it is */
r_short *radname;
int *filst,/*filestart*/ *size,*date,*dirad;/*current diradr*/
/*func returns the status of file (permanent etc) and -1 if 
  no more file could be found */
{ int d;
  char *cbuff;
  cbuff=rtbuffer;
  *filst=rtdirtab.filst;
  dirad[0]=rtdirtab.dirad[0]; dirad[1]=rtdirtab.dirad[1];
  rtpseek(rtdirtab.dirad[0],rtdirtab.dirad[1]);
  rtphio(0,cbuff,14+rtdirtab.rtextb,rtdirtab.curs+2);
  if(((r_short *)rtbuffer)[0]==04000) /*end of dirseg */
    /*fill rtsegfull here*/
    {if ((d=rtdirtab.dirad[0])%2==1
           && rtdirtab.dirad[1]>=01000-14-rtdirtab.rtextb)
       rtsegfull[(d-6)/2]=1;
     else rtsegfull[(d-6)/2]=0;
     if(rtdirtab.nexts==4)/*last segm*/ return(-1);
     *filst=rtheader();
     dirad[0]=rtdirtab.dirad[0];
     dirad[1]=rtdirtab.dirad[1];
     rtphio(0,cbuff,14+rtdirtab.rtextb,rtdirtab.curs+2);
    }
  /*modify rtdirtable*/
  rtdirtab.filst += ((r_short *)rtbuffer)[4];
  rtdirtab.dirad[0]=rtcurseek.sblkn;
  rtdirtab.dirad[1]=rtcurseek.sbyte;
  *size=((r_short *)rtbuffer)[4];
  *date=((r_short *)rtbuffer)[6];
  radname[0]=((r_short *)rtbuffer)[1];
  radname[1]=((r_short *)rtbuffer)[2];
  radname[2]=((r_short *)rtbuffer)[3];
  return(((r_short *)rtbuffer)[0]);
}


                      /*---rtcondens,rtucondens ---*/

/*rtcondens creates one dirsegm-adress offset from blockn byten
  low bound 0, upper bound 2*0777; rtucondens does the invers conv*/


rtcondens(blkn,byten)
{ 
  if(blkn & 1)return(0777+byten);
  else return(byten);
}

rtucondens(blkn,offs) r_short *blkn/*addr of  loc contains  orig blkn*/;
{int d,e;
  d= *blkn &0177776;
  if (offs/01000)
    {d++;  e=offs % 01000; }
  else e=offs;
  *blkn=d;
  *(blkn+1)=e;
  return;
}

                      /*---rtadjust---*/

rtadjust(fildes,radname) r_short  *radname;
/*it makes the opened file referenced by fildes to  be tentative
  and creates an empty entry right after it.
  if the segment is full, opens a new one and copies the half
  of the old one into the new one. updates the directiry ad-
  resses in rtttabl.dira-s if necessary. and now stats with the
  normal rtadjust proc described above and wich assumes that the
  directory is not full. rtadjust returns -1 if new segment
  can not be opened though required,
  and 0 in any other cases*/
{int nextsegm,sbuf,d,e,filst,i,f,g,acl,c;
  char *cbuff;
  d=(rttabl[fildes].dira[0]-6)/2;/*make segmnumb from discadr*/
  cbuff=rtbuffer;
  if(rtsegfull[d]) /*sgm is full*/
    {
     if(rtdirtab.senumb[0]==rtdirtab.senumb[1])return(-1);

    /*read the rtheader of dirsegm*/
    rtpseek(d=d *2+6,0);
    rtphio(0,cbuff,10,d+1);
    rtextb=((r_short *)rtbuffer)[3];
    filst=((r_short *)rtbuffer)[4];
    nextsegm=((r_short *)rtbuffer)[1];
    /*update the nextsegm pointer*/
    sbuf = ++rtdirtab.senumb[1];
    rtpseek(d,2);
    rtphio(1,(char *)&sbuf,2,d+1);
    rtpseek(6,4);
    rtphio(1,(char *)&sbuf,2,7);
    /*compute where the first entry of the second half starts, 
      because we will copy from here. result is in e*/
    e=14+rtextb-(502%(14+rtextb));
    /*compute the accumulated length of the first half to
      establish filestart-address for the second half*/
    rtpseek(d,0);
    rtphio(0,cbuff,512+e,d+2);
    acl=0;
    for(i=9;i<(512+e)/2;i += (14+rtextb+1)/2)
      acl += ((r_short *)rtbuffer)[i];
    /*now read the second half to the rtbuffer but leave space for the rtheader*/
    rtpseek(d+1,e);
    c=rtphio(0,cbuff+10,512,d+2);
    /*fill out rtheader */
    ((r_short *)rtbuffer)[0]=rtdirtab.senumb[0];
    ((r_short *)rtbuffer)[1]=nextsegm;
    ((r_short *)rtbuffer)[2]=rtdirtab.senumb[1];
    ((r_short *)rtbuffer)[3]=rtextb;
    /*compute new filst */
    ((r_short *)rtbuffer)[4]=filst +acl;
    /*now we write the rtbuffer to the new segment */
    rtpseek(g=(rtdirtab.senumb[1]-1)*2+6,0);
    rtphio(1,cbuff,c+10,g+2);
    /*close the old segment*/
    rtpseek(d+1,e);
    sbuf=04000;
    rtphio(1,(char *)&sbuf,2,d+2);
    /*now modify the rttabl.dira-s*/
    for(i=0;i!=numop+2;i++)
      if(rttabl[i].dira[0]==d+1 && rttabl[i].dira[1]>=e)
        {
        rttabl[i].dira[0]=g;
        rttabl[i].dira[1] -= e-10;
        }
    }

  /*here we do the rtadjust in case of not full segment*/
  /*read from the empty entry but leave space for the tentative
    entry in the rtbuffer*/
  rtpseek(rttabl[fildes].dira[0],rttabl[fildes].dira[1]);
  rtphio(0,cbuff+14+rtextb,1024-14-rtextb,rttabl[fildes].dira[0]+2);
  /*fill out the tentative entry in the rtbuffer */
  ((r_short *)rtbuffer)[0]=0400;
  ((r_short *)rtbuffer)[1]=radname[0];
  ((r_short *)rtbuffer)[2]=radname[1];
  ((r_short *)rtbuffer)[3]=radname[2];
  ((r_short *)rtbuffer)[6]=0; /*date */
  /*write back */
  rtpseek(rttabl[fildes].dira[0],rttabl[fildes].dira[1]);
  rtphio(1,cbuff,1024,rttabl[fildes].dira[0]+2);


  /*modify rttabl.dira-s*/
  d=rtcondens(rttabl[fildes].dira[0],rttabl[fildes].dira[1]);
  e=rttabl[fildes].dira[0] &0177776;
  for(i=0;i!= numop+2;i++)
    {
    if(i==fildes)continue;
    if(rttabl[i].dira[0]== e||rttabl[i].dira[0]==e+1)
     if((f=rtcondens(rttabl[i].dira[0],rttabl[i].dira[1]))>d)
        {
        f=f+14+rtextb;
        rtucondens((rttabl[i].dira[0]),f);
        }
    }
  return(0);
}


                      /*---rtcopy---*/

rtcopy(old,new,size)/*phis copy size in blocks */
{
  char *cbuff; int i;
  cbuff=rtbuffer;
  for(i=0;i!=size-1;i++)
    {
    rtpseek(old+1,0);
    rtphio(0,cbuff,512,old+i+1);
    rtpseek(new+i,0);
    rtphio(1,cbuff,512,new+i+1);
    }
  return;
}
/************************************************************/
/* now we start coding  rtun routines                        */

                      /**********/
                      /*rtmount */
                      /**********/

rtmount(name,rwflag) char *name;
{struct rtinode *i;
int srwf;
if(rtdev.fdes != -1) return(-1);
srwf=(rwflag==1)?2:rwflag;
if((rtdev.fdes=open(name,srwf)) == -1)return(-1);/*open spec file*/
/*seek to the start of rt-11 directory*/
#ifdef VAX
lseek(rtdev.fdes,6*512,0);
#else
seek(rtdev.fdes,6,3);
#endif
/*initialise some external values */
for (i=rttabl;i!= &rttabl[numop+2];i++)
  {i->opflg= -1;i->seekp[0]= 0;i->seekp[1]=0;}
rtdev.rwflag= rwflag;
rtcurseek.sblkn=06;
rtcurseek.sbyte =0;
rtwrop= -1;
return(rtdev.fdes);
}


                      /**********/
                      /*rtumount*/
                      /**********/

rtumount()
{if(rtdev.fdes != -1)
close(rtdev.fdes);
rtdev.fdes= -1;
return;
}



                      /**********/
                      /*rtopen  */
                      /**********/


rtopen(name,mode) char *name;
{ int i,j,emptfl,permfl,retur/*temporary for the radname */;
  int size,filst,date,dirad[2],/*for rtserdir call*/d;
  r_short temprad[3],radname[3];
  if (rtgetname(name,temprad)== -1)return(-1);/*invalid name */
  switch(mode)
    {
    case writfl:;
    case updatfl:
      if(rtwrop!= -1)/*there is file opened for write or update*/
        return(-1);
      if(rtdev.rwflag==readfl)return(-1);/*device mounted for read*/
    case readfl:
      /*search for free rtinode */
      for (i=0; i!=numop+1; i++)
        {if (rttabl[i].opflg== -1) goto opencan;/*free rtinode found*/
        }
     /*no free rtinode */
      default: return(-1);/* invalide mode*/
    }
  opencan: /*start open-procedure*/
  switch(mode)
    {
    case writfl:;
    case updatfl:rtwrop = i /*set fildescr to be written */;
                rttabl[numop+1].filsiz=0;
    case readfl:
      retur=i;/*set fildescr to be returned*/
     rttabl[i].opflg=mode;
     rttabl[i].filsiz=0;
    }
/*look for the specified name and for empty space(in case of write)*/
  rtinitdir();
  emptfl=0;/*will be set if empty entry found */
  permfl=0;/*will be set if permanent entry found*/
/*
 * Feb 82 D. Tso
 *	Mask out 0100000 bit in directory entry
 */
  while((d=rtserdir(radname,&filst,&size,&date,dirad))!= -1)
    { switch (d&(~0100000))
      {case 01000:/*empty entry*/
          if(mode==readfl)goto endwhile;
         emptfl=1;i=retur;break;
        case 02000:/*permanent entry*/
          for(i=3;i--;)/*compare filenames*/
            if(radname[i]!=temprad[i]) goto endwhile;
          if(mode==readfl)
            {i=retur;permfl=1;}
          else
            {i=numop+1;permfl=1;rtoldfl=1;}
          break;
        default: goto endwhile;
      }
    if(rttabl[i].filsiz<size)/*incase of raed always true*/
      {
        rttabl[i].pblkn=filst;
        rttabl[i].filsiz=size;
        rttabl[i].seekp[0]=filst;
        rttabl[i].seekp[1]=0;
        rttabl[i].dira[0]=dirad[0];
        rttabl[i].dira[1]=dirad[1];
        if(mode==readfl)break;
      }
      endwhile:;
    }
switch   (mode)
    {case readfl:
       if(permfl==0)return(-1);
     rttabl[retur].curend[0]=rttabl[retur].pblkn+rttabl[retur].filsiz;
       rttabl[retur].curend[1]=0;
       break;
     case writfl:
       if(emptfl==0)
       return(-1);
       rttabl[retur].curend[0]=rttabl[retur].pblkn;
       rttabl[retur].curend[1]=0;
       if(rtadjust(retur,temprad)== -1)/*no dir-space aval*/
       return(-1);
       break;
     case updatfl:
       if(emptfl==0 || permfl==0)return(-1);
     rttabl[retur].curend[0]=rttabl[retur].pblkn+rttabl[numop+1].filsiz;
       rttabl[retur].curend[1]=0;
       if(rtadjust(retur,temprad)== -1)return(-1);
       rtcopy(rttabl[numop+1].pblkn,rttabl[retur].pblkn,
              rttabl[numop+1].filsiz);
     }
return(retur);
}


                      /**********/
                      /*rtclose */
                     /**********/

rtclose(fildes)
{int mode,sbuf,s,p,i;
  int tentsize;/*size of tentative becoming permanent*/
  int empsize;/*size of the following empty space*/
  mode=rttabl[fildes].opflg;
  if(mode== -1)return(-1);
  switch (mode)
    {case writfl:;
      case updatfl:
       rtwrop= -1;
       /*delete old file if there is any*/
      if(rtoldfl)
       {rtpseek(rttabl[numop+1].dira[0],rttabl[numop+1].dira[1]);
         sbuf=01000;
         rtphio(1,(char *)&sbuf,2,rtcurseek.sblkn+1);
         rtoldfl=0;
       }
/*make tentative to be permanent*/

     rtpseek(rttabl[fildes].dira[0],rttabl[fildes].dira[1]);
     sbuf=02000;
     rtphio(1,(char *)&sbuf,2,rtcurseek.sblkn+1);
/*compute size of tentative and that of the following
  empty one and put them to the directory*/
     empsize=rttabl[fildes].filsiz;
     tentsize=rttabl[fildes].curend[0]-rttabl[fildes].pblkn;
     if(s=rttabl[fildes].curend[1])/*clear resrt of block*/
       {
       tentsize++;
       rtpseek(p=rttabl[fildes].curend[0],s);
       sbuf=0;
       while(rtphio(1,(char *)&sbuf,2,p+1));
       }
     empsize -= tentsize;
     rtpseek(rttabl[fildes].dira[0],rttabl[fildes].dira[1]+8);
     rtphio(1,(char *)&tentsize,2,rtcurseek.sblkn+1);
     rtpseek(rttabl[fildes].dira[0],rttabl[fildes].dira[1]+22+rtextb);
     rtphio(1,(char *)&empsize,2,rtcurseek.sblkn+1);
    case readfl:
     rttabl[fildes].opflg= -1;
   }
  return(0);
}


                      /**********/
                      /*rtread   */
                      /**********/

rtread(fildes,buffer,nbytes) char *buffer;
{int retur;
  if(rttabl[fildes].opflg==writfl)return(-1);
  rtpseek(rttabl[fildes].seekp[0],rttabl[fildes].seekp[1]);

  retur=rtphio(0,buffer,nbytes,rttabl[fildes].curend[0]);
  rttabl[fildes].seekp[0]=rtcurseek.sblkn;
  rttabl[fildes].seekp[1]=rtcurseek.sbyte;
  return(retur);
}


                      /**********/
                      /*rtwrite  */
                      /**********/

rtwrite(fildes,buffer,nbytes)char *buffer;
{int retur;
   if(rttabl[fildes].opflg==readfl)return(-1);
   rtpseek(rttabl[fildes].seekp[0],rttabl[fildes].seekp[1]);
 retur=rtphio(1,buffer,nbytes,rttabl[fildes].pblkn+rttabl[fildes].filsiz);
   rttabl[fildes].seekp[0]=rtcurseek.sblkn;
   rttabl[fildes].seekp[1]=rtcurseek.sbyte;
   if(rtcurseek.sblkn>rttabl[fildes].curend[0])
     {rttabl[fildes].curend[0]=rtcurseek.sblkn;
       rttabl[fildes].curend[1]=rtcurseek.sbyte;
    }
   else if (rtcurseek.sblkn ==rttabl[fildes].curend[0])
          if (rtcurseek.sbyte > rttabl[fildes].curend[1])
             rttabl[fildes].curend[1]=rtcurseek.sbyte;
   return(retur);
}



                      /**********/
                      /*rtdelete*/
                      /**********/


rtdelete(name) char *name;
{int mess,/*dummy*/dirad[2];/*for rtserdir calls*/
  r_short tempname[3],radname[3];
  int sbuf,i,d;
  if(rtgetname(name,radname)== -1)return(-1);
  rtinitdir();
  while((d=rtserdir(tempname,&mess,&mess,&mess,dirad))!= -1)
    {
     for(i=0; i!= 3; i++)
        if(tempname[i]!=radname[i])goto endwhile;
     if(d!= 02000)goto endwhile;/*file is not permanent*/
     rtpseek(dirad[0],dirad[1]);
     sbuf=01000;
     rtphio(1,(char *)&sbuf,2,dirad[0]+1);
     return(0);
     endwhile:;
    }
  return(-1);/*given filename not found*/
}

