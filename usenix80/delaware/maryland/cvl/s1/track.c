int a[16],b[16];
int th,dx[4],dy[4],cr[4];
extern cgoof;
main(){
      int row[512];
      register i,j;
      dx[0]=1;dy[0]=0;
      dx[1]=0;dy[1]=1;
      dx[2]= -1;dy[2]=0;
      dx[3]=0;dy[3]= -1;
      gopen(a,0,0,512,512);
      gopen(b,0,0,512,512);
      genter(b,4,0,0,0,0);
  while(1)
      {
      scanf("%d",&th);
      if(th < 0){printf(" -1");cexit();return;}
      grcur(a,cr,0);
      i=cr[0];j=cr[1];
      grrow(a,row,j,0,512,1);
      for(i=i;row[i+1]>th;i++);
      printf(" %d %d",i+1,j);
      trak(i,j,1);
      printf(" -1");
      }
      }
trak(x0,y0,code0) int x0,y0,code0;
{int x,y,code;
 x=x0;
 y=y0;
 code=code0;
 while(1)
     {printf(" %d",code);
      while(grpnt(a,x+dx[code],y+dy[code])>th)
	  {x=x+dx[code];
	   y=y+dy[code];
	   gwpnt(b,1,x,y);
	   if(--code== -1)code=3;
	  }
      if(++code==4)code=0;
      if(x == x0 && y == y0 && code == code0)return;
     }
}
