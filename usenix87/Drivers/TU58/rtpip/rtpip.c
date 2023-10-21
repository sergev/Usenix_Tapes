/*************************************************************/
/*********                                                   */
/**rtpip *                                                   */
/*********                                                   */
/*                                                           */
/* status: 1 dec 75 version 1.                               */
/*         jan 79, Modified by D. Ts'o                       */
/*            to use rk0-9, tap0-9                           */
/*            with a three-letter key                        */
/*	   feb 82, Modified by D. Ts'o			     */
/*	      to accept k,l,p as second letter for rk,rl,rp  */
/*	      or f for file dk?				     */
/*	   jan 85, Modified by J. Kourlas for vax.	     */
/*	      compile with -DVAX flag.			     */
/* author: t. forgacs univ of nijmegen comp. graphics        */
/* compiler: c unix version 6                                */
/* function: this is a program callable by shell in unix     */
/*           resides on direct0ry /usr/bin                   */
/*           it provides file shipping between rt11 and unix */
/* calling sequence:                                         */
/*        rtpip key filenames                                */
/*         where                                             */
/*           key is composed from 2 or 3 letters             */
/*             the first letters specifies the function      */
/*             the second one the rt11 device to be mounted  */
/*             and the third, the unit number                */
/*             the first letter                              */
/*                  r    restore on rt11 device              */
/*                  x    extract from rt11 device            */
/*                  d    delete rt11 file                    */
/*                  l    list directory                      */
/*                     (if no filename specified , all)      */
/*                  e list directory filenames specify       */
/*                           only the ext part of the file   */
/*                  n same as  e but namepart is specified   */
/*                  f   list free spaces available           */
/*            the second letter                              */
/*                   r        rk disk default                */
/*		     k,x,p,h,l rk, rx, rp, hk, rl	     */
/*		     f	      disk file dkx		     */
/*                   t        tap DEC-tape                   */
/*                   and the third,                          */
/*                   0...9    unit0...unit9                  */
/*          filenames can be unix filenames as appr for      */
/*           shell or rt11 filenames as appr for rt11        */
/*           but the latter ones must be preceeded by'+'     */
/* externals: rtun.c subroutines are called see unix sources */
/**************************************************************/

#ifdef VAX
#define r_short short
#else
#define r_short int
#endif

char *device = "/dev/rkx";	/* ## dyt default device */

int rtarg[20],unarg[20];/*tables containing indecies of
                          rt11 and unix filename arguments in argv*/
char rtbuffer[1024];/*rtun internal buffer */

main(argc,argv) int argc; char **argv;
{
  int devindex,rwf,i,j,k,l,m,n;
  if( argc < 2 ) {
    printf("Usage: %s abc [file ..]\n",argv[0]);
    printf("	where\n");
    printf("	a is a command:\n");
    printf("		r restore on rt11 device\n");
    printf("		x extract from rt11 device\n");
    printf("		d delete rt11 file\n");
    printf("		l list directory\n");
    printf("		e list those with specific ext\n");
    printf("		n list those with specific namepart\n");
    printf("		f list free space\n");
    printf("	b is rt11 device:\n");
    printf("		r,k,x,p,h,l rk,rk,rx,rp,hk,rl\n");
    printf("		f disk file dk? (unix file)\n");
    printf("		t DEC-tape\n");
    printf("	c is the unit number (0-9)\n");
    printf("	\n");
    printf("	file is either a unix file or a rt11 file (preceeded by +)\n");
    printf("	or an extension or a namepart\n");
    printf("	\n");
    printf("\t\texample: %s xf1 +junk.sav\t(extract JUNK.SAV from dk1)\n",argv[0]);
    printf("\t\texample: %s lx0 \t\t(list dir on rx0)\n",argv[0]);
    printf("\t\texample: %s ef2 +.com\t(list *.COM from dk2)\n",argv[0]);
    exit(0);
  }
  switch (argv[1][0] ) /*establish rwflag for rtmount */
    {
    case 'x': case 'l': case 'e': 
    case 'n': case 'f': rwf=0; break;
    case 'r':  case 'd': rwf=1;break;
    default: printf("bad usage\n");
             goto end;
    }
  devindex= 7;	/* index to unit number of device */
  switch (argv[1][1]) /* second letter is either the device or unit number */
   {
    case 'r':
	break;		/* /dev/rkx */
    case 'x':
    case 'k':
    case 'l':
    case 'p':
	device[6] = argv[1][1];
	break;
    case 'h':
	device = "/dev/hkx";
	break;
    case 'f':
	device = "dkx";
	devindex = 2;
	break;
    case 't':
	device = "/dev/tapx";
	devindex++;	/* unit index kicked by one */
	break;
    default:
	if(argv[1][1] >= '0' && argv[1][1] <= '9') {
		device[devindex] = argv[1][1];	/* copy unit number */
		break;
	}
	printf("bad usage\n");
	goto end;
    }
  if(device[devindex] == 'x' || argv[1][2] != 0) {
	if(device[devindex] == 'x' && argv[1][2] >= '0' && argv[1][2] <= '9'
		&& argv[1][3] == 0) {
		device[devindex] = argv[1][2];	/* copy unit number */
	}
	else {
	    printf("bad usage \n"); goto end;
	}
  }
  if(rtmount(device,rwf)== -1)
    {printf("%s can not be mounted\n",device); goto end;}
/*argc ==2 is allowed only for function l and f */
  if (argc==2)
    {
    if (argv[1][0]== 'l') {dirlist(0,"0");goto umou;}
    else if(argv[1][0]== 'f') {freespace();goto umou;}
    else {printf("bad usage\n"); goto umou;}
    }
  /*fill out rtarg unarg */
  j=0; k=0;
  for(i=2; i!=argc; i++)
    if(argv[i][0]=='+') rtarg[k++]=i;
    else unarg[j++]=i;
  l=k;
/*fill with zeros the untouched element of the arrays if the 
  corresponding element of the other array was filled out */
  if(k>j) for(l=j; l!= k; l++) unarg[l]=0;
  else if (j>k) for(l=k; l!= j; l++)rtarg[l]=0;
  /*l now contains the number of rt11-unix pairs+1*/
  switch (argv[1][0])
    {
    case 'r': case 'x': 
/*establish indecies to be used in the rucop or urcop call */
    for(i=0; i!=l;i++)
      {
      if(unarg[i]==0)
        {
        j=rtarg[i]; k=1;
        m=rtarg[i]; n=1;
        }
      else if(rtarg[i]==0)
        {
        j=unarg[i];k=0;
        m=unarg[i];n=lastslash(argv[m]);
        }
      else
        {
        j=unarg[i];k=0;
        m=rtarg[i];n=1;
        }
      if(argv[1][0]=='r')
        {
        if(urcop(&argv[j][k],&argv[m][n])== -1)
        printf("copy between %s and %s is not possible\n",argv[j],argv[m]);
        }
      else
        if(rucop(&argv[j][k],&argv[m][n])== -1)
        printf("copy between %s and %s is not possible\n",argv[j],argv[m]);
      }
    break;
    case 'l':
    for(i=2; i!= argc; i++)
      {
      if(dirlist(1,&argv[i][0])== -1)
      printf("%s invalid name\n",argv[i]);
      }
    break;
    case 'e':
    for(i=2; i!= argc; i++)
      {
      if(dirlist(2,&argv[i][0])== -1)
      printf("%s invalide name\n",argv[i]);
      }
    break;
    case 'n': 
    for(i=2; i !=argc; i++)
      {
      if(dirlist(3,&argv[i][0])== -1)
      printf("%s invalide name\n",argv[i]);
      }
    break;
    case 'f':
    freespace(); break;
    case 'd':
    for(i=2; i!=argc;i++)
      {
      if(argv[i][0]!='+'){printf("%s invalid\n",argv[i]);
                           continue;}
      if(rtdelete(&argv[i][1])== -1)
      printf("%s can not be deleted\n",argv[i]);
      }
    }
  umou: rtumount();
  end:;
} /*rtpip end*/

                      /*---convasc---*/


convasc(radname,ascii) char *ascii; r_short radname;
/*interprets radname as 3 rad50 letters converts them and    
  puts them into ascii */

{
  int i, k;
  /* to get the first char needs special treatment because
    radname can be negative and then "/" gives false result */
  if(radname & 01)k=1;
  else k=0;
  radname >>= 1;
  radname &= 077777;
  ascii[2]=((radname % 024)*2)+k;
  radname /= 024;
/*for the rest follow the normal way*/
  ascii[1]=radname % 050;
  radname /= 050;
  ascii[0]= radname %050;
  for(i=0; i!= 3;i++)
    if(ascii[i]==0)ascii[i]=040;
    else if(ascii[i]==033)ascii[i]=044;
    else if(ascii[i]==034)ascii[i]=056;
    else if(ascii[i]<=032 && ascii[i]>=001)ascii[i]=ascii[i]+0100;
    else if(ascii[i]<=047 && ascii[i]>=036)ascii[i]=ascii[i]+022;
  return;
}


                      /*---getradn---*/

getradn(radname,name) r_short *radname; char *name;
/*converts radname radix filename into name ascii filename */

{

  int i,j;
  convasc(radname[0],name);
  convasc(radname[1],name+3);
  name[6]='.';
  convasc(radname[2],name+7);
  name[10]=0;
  return;
}


                      /*---lastslash---*/

lastslash(name) char *name;
/*returns the index of char after the last slash in name
  if there is no slash it returns 0 */

{
  int i,j;
  j=0;
  for(i=0; name[i] != 0; i++)
    if(name[i]=='/') j=i+1;
  return(j);
}



                      /*---rucop---*/

rucop(nam1,nam2) char *nam1, *nam2;
/*copies rt11 file nam2 into unix file nam1 */
{
  char *cbuff; int a,b,c,d;
  cbuff=rtbuffer;
  if((a=rtopen(nam2,0))== -1)return(-1);
  if((b=creat(nam1,0777))== -1)return(-1);
  do
    {
    if((c=rtread(a,cbuff,1024))== -1)return(-1);
    if((d=write(b,cbuff,c))== -1)return(-1);
    }
  while(c && d);
  if(close(b)== -1)return(-1);
  if(rtclose(a)== -1)return(-1);
  return(0);
}


                        /*---urcop---*/

urcop(nam1,nam2) char *nam1, *nam2;
/*copies unix file  nam1 into rt11 file nam2 */

{
  char  *cbuff; int a,b,c,d,i;
  cbuff=rtbuffer;
  if((a=rtopen(nam2,1))== -1)return(-1);
  if((b=open(nam1,0))== -1)return(-1);
  do
    {
    if((c=read(b,cbuff,1024))== -1)return(-1);
    if((d=rtwrite(a,cbuff,c))== -1)return(-1);
    }
  while(c && d);
  if(close(b)== -1)return(-1);
  if(rtclose(a)== -1)return(-1);
  return(0);
}



                      /*---dirlist---*/

dirlist(dflag,name) char *name;
/*dirlist list the rt11 directory on the standard output
  device,partially or totally depending on dflag
  if dflag =
     0      the whole directory is listed
     1      name is taken as a whole filename(filnam.ext)
             and  it is  printed if present
     2      name is taken as .ext and all file with ext are printed
     3        same as 2 but name is taken as filnam
*/

{
  r_short radname[3],radnam1[3];
  int size,mess[2],a;
  char ascii[11];
  int k;
  if(dflag != 0 && name[0]!='+')return(-1);
  if(dflag == 2  && name[1]== '.')k=2;
  else k=1;
  rtinitdir();
  if(dflag != 0)
    if(rtgetname(name+k,radnam1)== -1)return(-1);
  while((a=rtserdir(radname,mess,&size,mess,mess)) != -1)
    {
    if(a==0400 || a==01000)/*entry is empty or tentative*/continue;
    switch(dflag)
      {
      case 0: getradn(radname,ascii);
              printf("%s  %d\n",ascii,size);
              break;
      case 1: if(radname[0]== radnam1[0] &&
                 radname[1]== radnam1[1] &&
                    radname[2]== radnam1[2])
                  {
                   getradn(radname,ascii);
                   printf("%s   %d\n",ascii,size);
                    }
                 break;
      case 2: if(radname[2]== radnam1[0])
                {
                getradn(radname,ascii);
                printf("%s   %d\n",ascii,size);
                }
              break;
      case 3: if(radname[0]==radnam1[0] &&
                 radname[1]==radnam1[1])
                {
                getradn(radname,ascii);
                printf("%s   %d\n",ascii,size);
                }
              break;
    }
  }
  return(0);
}


                       /*---freespace---*/

freespace()
/* list freespaces on rt11 device */
{
  int mess[3],size,a;
  rtinitdir();
  while((a=rtserdir(mess,mess,&size,mess,mess))!= -1)
    if(a==01000)printf("%d\n",size);
  return;
}
