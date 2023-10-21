PROGRAM MICROMUF (INPUT,OUTPUT);
{Micromuf - A program to computer the minimum and maximum usable frequencies
of a shortwave path between two specified coordinates. }

CONST PI = 3.14159265;
       I = 'INVALID';

VAR a,n,yt,xt,yr,xr,r,x,h,y,u,q,la,ls,hp,sf,fe,se,re,cp,k,l,xz,mf,ff :REAL;
    xh,z,fo,yf,ty,tl,yg,zo,yz,yn,yi,xn,sx,xs,wx,lh,lm,ab,rd,d,ex,man :REAL;
    transmitter,receiver : string[20];
    m,mh,ve,ho,t :integer;
    correct :boolean;
    key : char;

Function power(man,ex:real):real;
   Begin
      power:=EXP(ex*LN(man));
   End;

Procedure interlat; { Intermediate Latitude & Longitude calculations }
   Begin
      q:=cos(u*rd)*cos(xt*rd)*sin(k*lm*rd);
      x:=q+sin(xt*rd)*cos(k*lm*rd);
      xn:=(arctan(x/sqrt(-x*x+1+1E-12)))*d;
      q:=(cos(k*lm*rd)-sin(xt*rd)*sin(xn*rd));
      yi:=(-arctan(x/sqrt(-x*x+1+1E-12))+(PI/2))*d;
      if u < 180.0 then yi:=-yi;
      yn:=yt+yi;
      if yn > 180.0 then yn:=yn-360.0;
      if yn <-180.0 then yn:=yn+360.0;
   End;

Procedure mini_f2;
   Begin
      yz:=yn;
      If yn<-160.0 then yz:=yn+360.0;
      yg:=(20.0-yz)/50.0;
      zo:=20.0*yg/(1+yg+sqr(yg))+5.0*sqr(1-yg/7.0);
      z:=xn-zo;
      tl:=t-yn/15.0;
      if tl > 24.0 then tl:=tl-24.0;
      if tl < 0.0 then tl:=tl+24.0;
      mh:=m;
      If z <= 0.0 then
         Begin
            z:=-z;
            mh:=mh+6;
         End;
      xh:=cos(30.0*(mh-6.5)*rd);         { 1 week delay on equinoxes }
      sx:=(abs(xh)+xh)/2.0;              { F-layer local summer variance}
      wx:=(abs(xh)-xh)/2.0;              { F-layer local winter variance}
      If z > 77.5 then z:= 77.5;
      ty:=tl;
      If ty < 5.0 then ty:=tl+24.0;
      yf:=(ty-14.0-sx*2.0+wx*2.0-r/175.0)*(7.0-sx*3.0+wx*4.0-r/(150.0-wx*75.0));
      If abs(yf) > 60.0 then yf:=60.0;
      x:=(1+r/(175.0+sx*175.0));
      fo:=6.5*x*cos(yf*rd)*sqrt(cos((z-sx*5.0+wx*5.0)*rd));
      ex:=-0.5;
      sf:=power((1.0-sqr(cos(a*rd)*6367.0/(6367.0+h))),ex);
      ff:=fo*sf;
   End;

Procedure e_layer;
   Begin
      q:=sin(xn*rd)*sin(xs*rd);
      x:=q+cos(xn*rd)*cos(xs*rd)*cos((yn-15.0*(t-12.0))*rd);
      xz:=(-arctan(x/sqrt(-x*x+1+1E-12))+PI/2)*d;
      If xz <= 85.0 then
         Begin
            ex:=(1.0/3.0);
            fe:=3.4*(1.0+0.0016*r)*power(cos(xz*rd),ex);
         End
         Else
            Begin
               ex:=-0.5;
               fe:=3.4*(1.0+0.0016*r)*power((xz-80.0),ex);
            End;
      se:=power(1.0-0.965*sqr(cos(a*rd)),ex);
      ls:=0.028*sqr(fe)*se;
   End;

Begin { Main Program }
   rd:=PI/180;
   d:=180/PI;
   correct:=FALSE;
   ClrScr;
   Writeln ('                              *** MICROMUF ***     ');
   Writeln;
   Writeln (' This program calculate the :');
   Writeln (' * M. U. F. (Maximum Usable Frequency)');
   Writeln;
   Writeln (' * L. U. F. (Lowest Usable Frequency)');
   Writeln;
   Writeln (' of any shortwave sky-wave path.');
   Writeln;
   Writeln (' Calculations can be done for any month and sunspot number.');
   Writeln;
   Writeln;
   Writeln ('Name transmitter location');
   Readln (transmitter);
   Writeln;
   Repeat;
      Writeln ('Transmitter longitude in degrees. (W=+, E=-)');
      Readln (yt);
      If (yt >=-180.0) and (yt <= 180.0) Then correct:=TRUE
      Else Writeln(I);
   Until correct = TRUE;
   correct:=FALSE;
   Writeln;
   Repeat;
      Writeln ('Transmitter lattitude in degrees. (N=+, S=-)');
      Readln(xt);
      If (xt >= -90.0) and (xt <= 90.0) Then correct:=TRUE
      Else Writeln(I);
   Until correct = TRUE;
   correct:=FALSE;
   Writeln;
   Writeln ('Name receiver location.');
   Readln(receiver);
   Writeln;
   Repeat;
      Writeln ('Receiver longitude in degrees. (W=+, E=-)');
      Readln(yr);
      If (yr >= -180.0) and (yr <=180.0) Then correct:=TRUE
      Else Writeln(I);
   Until correct = TRUE;
   correct:=FALSE;
   Writeln;
   Repeat;
      Writeln ('Receiver lattitude in degrees. (N=+, S=-)');
      readln(xr);
      If (xr >=-90.0) and (xr<=90.0) Then correct:=TRUE
      Else Writeln(I);
   Until correct = TRUE;
   correct:=FALSE;
   Writeln;
   Repeat;
      Writeln ('Sunspot number.');
      Readln (r);
      If (r >= 1.0) and (r <=180.0) Then correct:=TRUE
      Else Writeln(I);
   Until correct = TRUE;
   correct:=FALSE;
   Writeln;
   Repeat;
      Writeln ('Month.');
      Readln (m);
      If (m >= 1) and (m <= 12) Then correct:=TRUE
      Else Writeln(I);
   Until correct = TRUE;

{   Geometry Calculations  }

   q:=sin(xt*rd)*sin(xr*rd);
   x:=q+cos(xt*rd)*cos(xr*rd)*cos(yt*rd-yr*rd);
   la:=(-arctan(x/sqrt(-x*x+1+1E-12))+(PI/2))*d;
   l:=111.1*la;
   q:=(sin(xr*rd)-sin(xt*rd)*cos(la*rd));
   x:=q/cos(xt*rd)/sin(la*rd);
   u:=(-arctan(x/sqrt(-x*x+1+1E-12))+(PI/2))*d;
   If yt-yr <= 0 Then u:=360-u;
   h:=275+r/2;
   xs:=23.4*cos(30*(m-6.25)*rd);
   n:=n+1;
   lh:=l/n;
   While lh > 4500.0 Do
      Begin
         n:=n+1;
         lh:=l/n;
      End;  { While }
   lm:=la/n;
   a:=(arctan((cos(0.5*lm*rd)-6367.0/(h+6367.0))/sin(0.5*lm*rd)))*d;
   While a < 1.5 Do
      Begin
         n:=n+1;
         lh:=lh/n;
         While lh > 4500.0 Do
            Begin
               n:=n+1;
               lh:=l/n;
            End; { While }
         lm:=la/n;
         a:=(arctan((cos(0.5*lm*rd)-6367.0/(h+6367.0))/sin(0.5*lm*rd)))*d;
      End;

{ Plot chart on screen }

   ClrScr;
   Writeln ('From: ',transmitter,' to: ',receiver);
   Write   ('Month: ',m);
   Writeln (' SSN: ',r:3:0,' Dist: ',Round(l+0.5),' KM');
   Writeln ('Azim: ',Round(u+0.5),' degrees.  F-hops: ',n:4:2);
   ve:=4;
   ho:=1;
   GotoXY(ho,ve);
   q:=34.0;
   While q >=2.0 Do
      Begin
         Writeln ('I                         I',q:2:0);  { 25 spaces }
         q:=q-2.0;
      End; { While }
   Writeln ('---------------------------');   { 27 dashes }
   Writeln (' 0 2 4 6 8 10  14  18  22 H (UTC)');
   Writeln ('      +: MUF  -: LUF');
   ve:=4;
   ho:=32;
   GotoXY(ho,ve);
   Writeln ('mHz');
   For t:=1 to 24 Do
      Begin
         ab:=0.0;
         k:=0.5;
         interlat;
         mini_f2;
         mf:=ff;
         k:=n-0.5;
         interlat;
         mini_f2;
         If ff < mf then mf := ff;
         ve:=21-Round(mf/2.0+0.5);
         ho:=t+1;
         if ve < 4 then ve:=4;
         GotoXY(ho,ve);
         Writeln ('+');
         While k <= n-0.25 Do
            Begin
               interlat;
               e_layer;
               ab:=ab+ls;
               k:=k+0.5;
            End;
         ve:=20-Round(ab+0.5);
         If ve < 4 Then ve:=4;
         If ve > 20 Then ve:=20;
         GotoXY(ho,ve);
         Writeln('-');
      End;
      Writeln;
      Writeln;
      Writeln;
      Writeln;
      Writeln;
      Write ('          ----- Press a key to continue ----- ');
      Readln (key);
End.
