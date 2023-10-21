20 rem
                <       main    routine       >

30 clear 500
40 defint i,j,l,n
50 dim da(20,10), sr$(34), x(20), y(20), tb(101)
60 on error goto 50000
70 sc$=
"<< pharmacologic calculation program   version  1.0  1/19/81 >>
"+string$(63,"-")
80 cls:print sc$;"    initializing program"
90 ls=12:rem             lines/screen
100 id=0:rem             data flag
110 restore
120 il=i:read i,sr$(i):if i<>0 then 120
130 sr$(il+1)="end":rem        last sr$
135 gosub 43000:rem     read probit table ?
140 i=1:lc=0
150 cls:print sc$;
160 if sr$(i)="end" then i=1:lc=0:goto 200
170 if sc$(i)<>"" then print i;"- ";sr$(i):lc=lc+1
180 i=i+1
190 if (lc/ls=int(lc/ls)) and lc<>0 then 200 else 160
200 jp=2:u=255:print
210 l$="":pn$="":px$="independent variable":py$="dependent variable"
220 print
    " enter # of procedure or press enter for next page ===> ";
230 u=0:input u:rem
240 if u>0 and u<34 then 260
250 goto 150
260 cls:tt=(54-len(sr$(u)))/2
270 print tab(tt);"< #";"- ";sr$(u);" >":print
280 on u gosub 1000,2000,3000,4000,5000,6000,
    7000,8000,9000,10000,11000,12000,13000,
    14000,15000,16000,17000,18000,19000,20000
290 if u<20 goto 310
300 on u-20 gosub 21000,22000,23000,24000,
    25000,26000,27000,28000,29000,30000,
    31000,32000,33000
310 i=1
320 goto 200
330 rem
                        < common subroutines >
500 print:input"press <enter> to continue";e$:return
510 rem                 < convert to log(10) >
520 for i=1 to n(j):da(i,j)=log(da(i,j))/log(10):nexti
    :id=0:return
530 rem                 < print calc values ? >
540 print:a$="":input"print calculated values (y/n) ";a$
    :a$=left$(a$,1)
550 print:return
560 rem                 < calc new vals ? >
570 print:print
    "enter number to calculate new value or <enter> to continue."
580 return
590 x=999:input x:return
600 rem                 < print x y message >
610 if px$<>xl$ or py$<>yl$ then print:
    print"note: x = ";px$;:printtab(36);"y = ";py$:print
620 xl$=px$:yl=py$:return
700 rem                 < input data >
710 l=0
720 id=0
730 xl$="":yl$=""
740 j=0:n(1)=0:i=0
750 gosub 610
780 if l<>0 then 800 else print"enter number of ";
785 if l$<>"" then print l$;:else
    if jp=1 then print"groups of data ";:else
    if jp=2 then print"lines or curves ";
790 input l:l=l*jp
800 for j = 1 to l step jp
805 print"enter number of ";
810 if pn$<>"" then print pn$;:else
    if jp=1 print"observations for group #";j;:else
    if jp=2 print"observations for line or curve #";(j+1)/2;
840 input n:if n<=0 then 805
850 if n>100 then print"n must be less than 100.":goto 805
860 n(j)=n:if jp=2 then n(j+1)=n
870 for i = 1 to n(j)
880   if jp=2 then print"enter x,y pair #";i;
      :input da(i,j),da(i,j+1):goto 910
890   print"enter observation #";i;
900   input da(i,j)
910 next i,j
920 if l=0 then return:rem     data  ?
930 for j=1 to l
940   print:print"group #";j;": ";
950   for i=1 to n(j):print da(i,j);:next i
960 next j
970 id=1:rem           set data flag
980 print:print
990 return

40000 data 0,"end"
41000 rem              read table data
42010 if fi$=fl$ and tb(n)<>0 then return
42020 print:print"enter ";fi$;" table value for";n;"d.f.";
42040 nl=n:fl$=fi$:in$="":input in$:tb(n)=val(in$)
42050 print
42100 return

50000 print:rem               error trap routines
50030 print tab(10)"***** error - ";
50040 iferr=20 then print"division by zero, check data '"
      :resume 50090
50050 if err=8 then print
     "illegal funtion, check data !":resume 50090
50060 iferr=14 then print"subroutine not does not exist *****"
      :resume 50090
50065 if err=12 print"out of memory (re-dimension or re-run)"
      :resume 50999
50070 print"on line #";erl;".  type #";err/2+1;".
50080 resume 50100
50090 gosub 310
50100 gosub 500:run
50999 end
1000 data 1,"dosage & concentration: drug stock solutions"
1010 rem
1020 rem                        subroutines: none
1030 gosub 1100:rem                             input
1040 gosub 1200:rem                             process
1050 gosub 1900:rem                             output
1060 return
1100 mo=0:mw=0:co=0:vo=0
1110 input"enter molecule weight of drug ";mw
1120 print"enter conc. of stock drug solution ";
1130 input"(grams,mls)";gr,ml
1140 input"enter desired molarity of drug sol'n ";mo
1150 input"enter volume or sol'n desired (mls) ";vo
1160 return
1200 co=gr/ml
1210 x=(mo*mw*vo)/(1000*co)
1220 if x>vo then print"drug stock is too dilute !":goto 1120
1230 input"is the drug an electrolyte";i$
1240 if left$(i$,1)="y" then input"enter # of ions (2-5)";io
     :if io<1 or io>5 then 1240
1250 o(1)=1:o(2)=1.8:o(3)=2.6:o(4)=3.4:o(5)=4.2
1260 e=(58.5*o(io))/(mw*1.8)
1270 g=e*((mw*mo*vo)/1000)
1280 q=(.009*vo)-g
1290 if q<0 then q=0
1300 return
1900 print:print"add";x;"mls of drug stock and";int(1000*q+.5)
     ;"milligrams of nacl to"
1910 print vo-x;"mls of distilled water";
1920 if q=0 then print". -not isotonic-"
1930 if q>0 then print" to give an isotonic solution."
1999 return
2000 data 2,"mean, standard deviation & confidence limits"
2010 rem
2020 rem                        subroutines: none
2030 gosub 2100:rem                              input
2040 gosub 2200:rem                              process
2050 gosub 2600:rem                              output
2060 return
2100 jp=1:px$="":py$="":gosub 710
2110 return
2200 for j=1 to l
2210   gosub 2660
2220 next j
2230 return
2600 n=n(1)-1:gosub 2960
2605 gosub 2790
2610 for j=1 to l
2620   gosub 2810
2630 next j
2640 return
2650 j1=j:gosub 2660:j=j+1:gosub 2660:j=j1:return
2660 n=n(j):x=0:sx=0:me=0:s2=0:de=0:ds=0:se=0:va=0
2670 for i=1 to n
2680    x=da(i,j):sx=sx+x:s2=s2+x*x
2690 next i
2700 me=sx/n
2710 for i=1 to n
2720    x=da(i,j):ds=ds+(x-me)*(x-me)
2730 next i
2740 va=ds/(n-1)
2750 de=sqr(va)
2760 se=de/sqr(n)
2770 sx(j)=sx:s2(j)=s2:de(j)=de:ds(j)=ds:
     va(j)=va:me(j)=me:se(j)=se
2780 return
2790 print"
 #  n  sum      mean     t value  +/-      std dev  std err
__ ___ ________ ________ ________ ________ ________ ________"
2800 return
2810 n=n(j):de=de(j)
2860 n=n-1:gosub 2960
2870 print j;tab(3)n(j);
2880 cl=tv*se(j)
2890 printtab(5);sx(j);tab(15);me(j);tab(24);tv;
2900 printtab(33);cl;
2910 printtab(42);de(j);tab(51);se(j)
2920 return
2950 rem      fi$ may be changed to a t table file for p=99
2960 if n>30 then tv=1.96 else fi$="ttable/p95":gosub 41000:tv=tb(n)
2999 return
3000 data 3,"linear regression i"
3010 rem
3020 rem                        subroutines: 2
3030 gosub 710:rem                              input
3040 gosub 3200:rem                             process
3050 gosub 3800:rem                             output
3060 return
3200 gosub 2200
3210 for j=1 to l step 2:gosub 3230:next j
3220 return
3230 n=n(j):x=0:sx=sx(j):xm=me(j):x2=s2(j):y=0:sy=sx(j+1):
     ym=me(j+1):y2=s2(j+1):xy=0:m=0:b=0
3240 for i = 1 to n(j)
3250    x=da(i,j)
3260    y=da(i,j+1)
3270    xy=xy+(x*y)
3280 next i
3290 m=((sx*sy/n)-xy)/((sx[2/n)-x2)
3300 m(j)=m
3310 b=ym-(m*xm)
3320 b(j)=b
3330 xy(j)=xy
3350 return
3800 for j=1 to l step 2:gosub 3830:next j
3810 gosub 3860
3820 return
3830 gosub610:print"regression line #";(j+1)/2;": ";
3840 print" y = ";m(j);"* x + ";b(j)
3850 return
3860 gosub 540:if a$<>"y" then return
3870 for j=1 to l step 2:gosub 3880:next j:return
3880 print
     "line #";(j+1)/2,"observed x","obserbed y","calculated y"
3890 for i=1 to n(j)
3900    yc=m(j)*da(i,j)+b(j)
3910    print "",da(i,j),da(i,j+1),yc
3920 next i
3930 gosub 570
3940 print"",:gosub 590:if x=999 then print:goto 3999
3950 yc=m(j)*x+b(j):print"",x,"",yc
3960 goto 3940
3999 return
4000 data 4,"linear regression ii: lines through origin"
4010 rem
4020 rem                        subroutines: 2,3
4030 gosub 710:rem                              input
4040 gosub 3200:rem                             process
4050 gosub 4800:rem                             output
4060 return
4800 for j=1 to l step 2
4810   print"line #";(j+1)/2;": ";
4820   sl=xy(j)/s2(j)
4830   gosub 610
4840   print"regression through origin : y = ";sl;"* x "
4850   gosub 540:if a$="y" then gosub 4880
4860 next j
4870 return
4880 print"observed x","observed y","calclated y"
4890 for i=1 to n(j)
4900   yc=sl*da(i,j)
4910   print da(i,j),da(i,j+1),yc
4920 next i
4930 gosub 570
4940 gosub 590:if x=999 then print:return
4950 yc=sl*x:print x,"",yc
4960 goto 4940
4999 return
5000 data 5,"analysis of the regression line"
5010 rem
5020 rem                        subroutines: 2,3
5030 gosub 3100:rem                             input
5040 gosub 5200:rem                             process
5050 gosub 5400:rem                             output
5060 return
5200 gosub 2200
5210 for j=1 to l step 2:gosub 5220:next j:return
5220 gosub 3230
5230 xx=0:ss=0:r=0:em=0:eb=0:ex=0
5240 for i = 1 to n
5250   x=da(i,j)
5260   xx=xx+(x-xm)*(x-xm)
5270   y=da(i,j+1)
5280   cy = m * x + b
5290   ss = ss + ((y-cy)*(y-cy))
5300 next i
5310 ss(j)=ss
5320 xx(j)=xx
5330 r(j)=(xy-(n*xm*ym))/sqr((x2-(n*(xm*xm)))*(y2-(n*(ym*ym))))
5340 s = sqr(ss/(n-2)):s(j)=s
5350 em(j) = s * sqr(1/xx)
5360 ey(j) = s * sqr((1/n)+(xm*xm/xx))
5370 ex(j) = abs(s/m) * sqr((1/n)+(ym/m)*(ym/m)/xx)
5380 return
5400 for j=1 to l step 2:gosub 5410:next j:gosub 3860:return
5410 gosub 3830
5420 n=n(j)-2:xi$="x-intercept"
5430 print"correlation coefficient = ";r(j)
5440 print:gosub 5600:t=tb(n):print"(stand. error)"
5460 p1$="slope":p2=m(j):p3=em(j)*t:p4=em(j):gosub 5550
5470 p1$="y-intercept":p2=b(j):p3=ey(j)*t:p4=ey(j):gosub 5550
5490 p1$=xi$:p2=-b(j)/m(j):p3=ex(j)*t:p4=ex(j):gosub 5550
5510 return
5550 fm$=
"%           % = %          % +/- %          %   (%          %)"
5555 print using fm$;p1$,str$(p2),str$(p3),str$(p4)
5560 return
5600 gosub 2960
5610 print"t table value = ";tv;tab(29);"df =";n;" p = 95%",
5620 return
5700 gosub 5600:print:print"t calculated  = ";abs(t);
5720 print tab(29);:if abs(t)<tv then print "not ";
5740 print"signifigant"
5999 return
6000 data 6,"parallel lines i: test for parallelism"
6010 rem
6020 rem                        subroutines: 2,3,5
6030 gosub 6100:rem                             input
6040 gosub 5200:rem                             process
6050 gosub 6800:rem                             output
6060 return
6100 gosub 710
6110 if l<4 then print"you must have more than one line !":
     id=0:goto 6100
6120 return
6800 gosub 3050
6810 for j=3 to l step 2
6820   t=0:sp=0
6830   sp=sqr(((n(1)-2)*s(1)[2+(n(j)-2)*s(j)[2)/(n(1)+n(j)-4))~
6840   t=abs((m(1)-m(j))/(sp*sqr((1/xx(1))+(1/xx(j)))))
6850   print:print"line # 1 vs. line #";(j+1)/2;":"
6860   n=n(1)+n(j)-4:gosub 5700
6870 next j
6999 return
7000 data 7,"parallel lines II: construction of parallel lines"
7010 rem
7020 rem                        subroutines: 2,3,5,6
7030 gosub 6100:re                              input
7040 gosub 7200:rem                             process
7050 gosub 7900:rem                             output
7060 return
7200 gosub 2200
7210 for j=1 to l step 2
7220   gosub 5220:w(j)=1/(em(j)*em(j))
7230 next j
7235 tp=0:bm=0
7240 for j=1 to l step 2:rem        common slope
7250   tp=tp+(w(j)*m(j)):bm=bm+w(j)
7260 next j
7265 cm=tp/bm
7270 return
7900 print:gosub 3050
7910 print:print"common slope = ";cm
7920 for j=1 to l step 2
7930  print "equation for line #";(j+1)/2;": ";
7940  print "y =";cm;" * ( x ";
7950 if me(j)<=0 then print"+";
7970 next j
7999 return
8000 data 8,"graded dose-response"
8010 rem
8020 rem                        subroutines: 2,3,5
8030 gosub 8100:rem                           input
8040 gosub 8200:rem                           process
8060 return
8100 py$="response"
8110 input"enter 1 for dose or 2 for log(dose) data ";op
8120 if op=1 px$="dose" else if op=2 px$="log(dose)" else 8110
8130 gosub 710
8140 return
8200 for j=1 to l step 2
8210   gosub 8240:gosub 8450:gosub 8290
8220 next j
8230 return
8240 for i=1 to n(j)
8250   x(i)=da(i,j):y(i)=da(i,j+1)
8260 next i
8270 nn(j)=n(j)
8280 return
8290 for i=1 to n(j)
8300   da(i,j)=x(i):da(i,j+1)=y(i)
8310 next i
8320 if op=1 then gosub 520
8330 gosub 8550
8340 px$="log (d)":py$="response"
8350 gosub 8570
8360 m5=mx(j)/2:a5=(m5-b(j))/m(j)
8410 se=(s/m(j))*sqr((1/n(j))+(a5-me(j))[2/xx(j))
8420 print:n=n(j)-2:gosub 5600:print
8430 p1$="log(a50)":p2=a5:p3=tv*se:p4=se:gosub 5550
8440 return
8450 for i=1 to n(j)
8460   if op=2 then da(i,j)=1/10[x(i) else da(i,j~))=1/x(i)
8470   da(i,j+1)=1/y(i)
8480 next i
8490 gosub 8550
8500 px$="1/dose":py$="1/response":print
8510 print"double reciprocal plot yields an emax of ";1/b(j)
8520 input"enter new emax of press <enter> if satisfactory ";x:
     b(j)=1/x:mx(j)=x
8530 gosub 8570
8540 print:return
8550 gosub 2650:gosub 5220:rem          sum & analyse
8560 return
8570 gosub 5410:rem                     output
8580 gosub 540:if a$<>"y" then 8999
8590 print 
8600 print px$,py$,
8610 if px$="1/dose" then print "dose","response" else print
8620 for i=1 to n(j)
8630   x=da(i,j):gosub 8690
8640 next i
8650 gosub 570
8660 gosub 590
8670 if x<>999 then gosub 8690:goto 8660
8680 return
8690 yc=m(j)*x+b(j)
8700 print x,yc,
8710 if px$="1/dose" then print 1/x,1/yc else print 
8999 return
9000 data 9,"quantal dose-response: probits"
9010 rem
9020 rem                    subroutines: 2,3,8
9030 gosub 9100:rem                             input
9040 gosub 9200:rem                             process
9050 gosub 9600:rem                             output
9060 return
9100 py$="% responding":gosub 8110
9110 return
9200 for j=1 to l step 2:gosub 9220:next j
9210 return
9220 gosub 8220:0:if op=1 then gosub 520:rem conv to log
9230 px$="log(dose)"
9240 for i=1 to n(j)
9250   n=int(da(i,j+1)+.5)
9260   da(i,j+1)=pr(n)
9270 next i
9280 id=0:py$="probit"
9290 gosub 2650:gosub 3230:gosub 3830
9300 return
9600 for j=1 to l step 2:gosub 9620:next j
9610 return
9620 b=b(j):m=m(j)
9630 e16=(4.0055-b)/m: e84=(5.9945-b)/m: e50=(5-b)/m
9640 print:print"ed50  = ";10[e5,"log(ed50) = ";e5
9650 print"ed16  = ";10[e1,"log(e16) = ";e1
9660 print"ed84  = ";10[e8,"log(e84) = ";e8
9670 e1=10[e1:e5=10[e5:e8=10[e8
9680 sf=((e8/e5)+(e5/e1))/2:print"s function = ";sf
9690 gosub 540:if a$<>"y" then return
9700 print"dose",px$,"% response",py$
9710 fori=1to n(j):x=da(i,j):print10[x,x,y(i),pr(int(y(i)+.5))
     :next i
9720 gosub 570
9730 input"calculate from: 1=log(dose) or 2=% response ";op
9740 if op<1 or op>2 then 9730
9750 on op gosub 9780,9900
9760 return
9770 rem                log (dose)
9780 print"enter log(dose) ";:gosub 590
9790 if x=999 then return
9800 print 10[x,x,
9810 gosub 9840:rem     get probit
9820 print iy,pr(iy)
9830 goto 9780
9840 p=m(j)*x+b(j)
9850 iy=1:if p<pr(1) then print"<";:return
9860 dt=(pr(iy+1)-pr(iy))/2
9870 if p~~<pr(iy)+dt then return
9880 iy=iy+1:if iy>99 then print">";:return: else 9860
9890 rem                % response
9900 print"enter % response ";:gosub 590:iy=x
9910 if iy=999 then return
9920 if iy>100 or iy<0 then print"error: enter 0-100% only."
     :goto 9900
9930 x=(pr(iy)-b(j))/m(j)
9940 print 10[x,x,iy,pr(iy)
9950 goto 9900
9999 return


4300 dim pr(100)
43010 for i=1 to 99:read pr(i):next i:return
43020 data 2.6737 , 2.9463 , 3.1192 , 3.2493 , 3.3551 , 3.4452
, 3.5242 , 3.5949 , 3.6592 , 3.7184 , 3.7735 , 3.825 , 3.8736
43030 data 3.9197 , 3.9636 , 4.0055 , 4.0458 , 4.0846 , 4.1221
, 4.1584 , 4.1936 , 4.2278 , 4.2612 , 4.2937 , 4.3255 , 4.3567
43040 data 4.3872 , 4.4172 , 4.4466 , 4.4756 , 4.5041 , 4.5323
, 4.5601 , 4.5875 , 4.6147 , 4.6415 , 4.6681 , 4.6945 , 4.7207
43050 data 4.7467 , 4.7725 , 4.7981 , 4.8236 , 4.849 , 4.8743
, 4.8996 , 4.9247 , 4.9498 , 4.9749 , 5 , 5.0251 , 5.0502
43060 data 5.0753 , 5.1004 , 5.1257 , 5.151 , 5.1764 , 5.2019
, 5.2275 , 5.2533 , 5.2793 , 5.3055 , 5.3319 , 5.3585 , 5.3853
43070 data 5.4125 , 5.4399 , 5.4677 , 5.4959 , 5.5244 , 5.5534
, 5.5828 , 5.6128 , 5.6433 , 5.6745 , 5.7063 , 5.7388 , 5.7722
43080 data 5.8064 , 5.8416 , 5.8779 , 5.9154 , 5.9542 , 5.9945
, 6.0364 , 6.0803 , 6.1264 , 6.175 , 6.2265 , 6.2816 , 6.3408
43090 data 6.4051 , 6.4758 , 6.5548 , 6.6449 , 6.7507 , 6.8808
, 7.0537 , 7.3263 , 7.3263
10000 data 10,"relative potency"
10010 rem
10020 rem                       subroutines: 2,3,5,7,8
10030 gosub 10100:rem                           input
10040 gosub 10200:rem                           process
10050 gosub 10900:rem                           output
10060 return
10100 gosub 8100
10110 if l<4 then print
   "you must enter at least 2 dose-response curves.":goto 10100
10120 return
10200 if op=2 then 10250
10210 for j=1 to l step 2
10220 gosub 520
10230 next j
10240 print
10250 if n(1)<3 then 10350
10260 print tab(20)"parallel line assay":print
10270 gosub 7200:gosub 7900
10280 s=-me(2)/cm+me(1)
10290 for j=3 to l step 2
10300   u=-me(j+1)/cm+me(j):print
10320   rp=10[(u-s):gosub 10900
10330 next j
10340 return
10350 print tab(20)"' 2 and 2 ' dose assay":print
10360 d1=da(1,1):d2=da(2,1):x=d2-d1:u1=da(1,2):u2=da(2,2)
10365 for j=3 to l step 2
10370   if d1=da(1,j) and d2=da(2,j) then 10410
10380 print"all dose response curves must use equivilent doses!"
10390 goto 10000
10410   s1=da(1,j+1):s2=da(2,j+1)
10420   v=(u2-s2+u1-s1)/2 : m=(s2-s1+u2-u1)/(2*x)
10430   rp=10[(v/m):gosub 10900
10435 next j
10440 return
10900 print"relative potency of line # 1 vs #";(j+1)/2;"= ";rp
10999 return
11000 data 11,"dissociation constant i: agonists"
11010 rem
11020 rem                       subroutines: 2,3
11030 gosub 11100:rem                           input
11040 gosub 11200:rem                           process
11050 gosub 11900:rem                           output
11060 return
11100 dc=0
11110 print "enter equiactive dose pairs (a',a)."
11120 px$="a'":py$="a"
11130 gosub 710
11140 return
11200 px$="1/a'":py$="1/a"
11210 pz$=string$(9,45):print:for j=1 to l step 2
11220 print px$,py$:print pz$,pz$
11230 for i=1 to n(j)
11240     da(i,j)=1/da(i,j):print da(i,j),
11250     da(i,j+1)=1/da(i,j+1):print da(i,j+1)
11260 next i,j
11270 gosub 3040
11280 return
11900 for j=1 to l step 2
11910   dc=(m(j)-1)/b(j)
11920   print "dissociation constant (ka) = ";dc
11930 next j
11999 return
12000 data 12,"dissociation constant ii: partial agonists"
12010 rem
12020 rem                       subroutines: 2,3,111
12030 gosub 12100:rem                           input
12040 gosub 12200:rem                           process
12050 gosub 12900:rem                           output
12060 return
12100 dc=0
12110 print"enter equiactive dose pairs.  use molar units."
12120 px$="conc. of partial agonist"
12130 py$="conc. of agonist"
12140 gosub 710
12150 return
12200 px$="1/p":py$="1/a"
12220 gosub 11210
12250 return
12900 for j=1 to l step 2
12910 print"dissociation constant of partial agonist =";
12920 print m(j)/b(j)
12930 next j
12999 return
13000 data 13,"dissociation constant iii: perturbation methods"
13010 rem
13020 rem                       subroutines: 2,3
13030 gosub 13100:rem                           input
13040 gosub 13200:rem                           process
13050 gosub 13900:rem                           output
13060 return
13100 dc=0
13110 px$="agonist concentration":py$="time constant (sec)"
13120 gosub 710
13130 return
13200 for j=1 to l step 2
13210 py$="1/time constant":px$="(agonist)":printpy$
      :print"---------------"
13220   for i=1 to n(j)
13230     da(i,j+1)=1/da(i,j+1):print da(i,j+1)
13240 next i,j
13250 gosub 2200:gosub 3040
13260 return
13900 for j=1 to l step 2
13910 print"dissociation constant =";b(j)/m(j)
13920 next j
13999 return
14000 data 14,"pa2 analysis i: schild plot"
14010 rem
14020 rem                       subroutines: 2,3,5
14030 gosub 14100:rem                           input
14040 gosub 14500:rem                           process
14050 gosub 14900:rem                           output
14060 return
14100 print"computation of pa2 uses molar units of antagonist."
14110 print
     "you may enter antagonist concentrations in several ways:"
14120 u$(1)="molar":u$(2)="ug/ml or mg/kg"
      :u$(3)="mg/ml":u$(4)="g/100ml""
14130 for x=1 to 4:print x;"= ";u$(x);",";:next x
14140 print:print"enter option:";
14150 input op
14160 if op>1 then input"enter molecular wt of antagonist ";mw
14180 px$="conc. in "+u$(op)
14190 py$="dose ratio"
14200 gosub 710
14210 if op=1 then 14320
14220 if op=2 then fa=.001 else if op=3 then fa=1 else
      if op=4 then fa=10
14230 fa=fa/mw
14240 print "doses in moles:";
14250 for j=1 to l step 2
14260 for i=1 to n(j)
14270  da(i,j)=da(i,j)*fa
14280  print da(i,j);
14290 next i:id=0
14300 next j
14310 print
14320 for j=1 to l step 2
14330  px$="- log (b)":py$="log (a'/a -1)":print px$,py$
14340  for i=1 to n(j)
14350    da(i,j)=-log(da(i,j))/log(10)  :rem  conc
14360    da(i,j+1)= log(da(i,j+1)-1)/log(10):rem  ratio-1
14370    print da(i,j),da(i,j+1)
14380  next i
14390 next j
14400 id=0:return
14500 gosub 2200:gosub 5210
14520 return
14900 for j=1 to l step 2
14910  gosub 3830
14920  n=n(j)-2:xi$="pa2"
14930  gosub 5430
14940 next j
14950 gosub 3860
14999 return
15000 data 15,"pa2 analysis ii: time-dependent method"
15010 rem
15020 rem                       subroutines: 2,3,5
15030 gosub 15100:rem                           input
15040 gosub 15200:rem                           process
15050 gosub 15900:rem                           output
15060 return
15100 print
"the time-dependent method requires dose ratio data to be
collected over several time periods for a single concentration
of antagonist."
15110 input"enter concentration of antagonist in molar units";bc
15120 pn$="time periods when dose ratio was determined "
15130 px$="time period (min)"
15140 py$="dose ratio (a'/a)"
15150 l=2:gosub 720
15160 return
15200 for j=1 to l step 2
15210  for i=1 to n(j)
15220    da(i,j+1)=log(da(i,j+1)-1)/log(10)
15230 next i,j
15250 py$="log (a'/a-1)":px$="time"
15260 gosub 2200:gosub 5200:gosub 3800
15265 id=0:return
15900 for j=1 to l step 2
15905 print"time-dependent pa2 =";b(j)-log(bc)/log(10)
15910 print"half-life of antagonist =";-log(2)/(m(1)/.434294)
15920 next j
15999 return
16000 data 16,"pa2 analysis iii: constrained plot"
16010 rem
16020 rem                       subroutines: 2,3,5,14
16030 gosub 14100:rem                           input / pa2 i
16040 gosub 16200:rem                           process
16050 gosub 16900:rem                           output
16060 return
16200 gosub 2200:gosub 5210
16220 for j=1 to l step 2
16230  xm=me(j):ym=me(j+1)
16240  ss=0
16250  for i=1 to n(j)
16260    x=da(i,j):y=da(i,j+1)
16270    ss=ss+((y-ym)+(x-xm))*((y-ym)+(x-xm))
16280  next i
16290 ss=sqr(ss/(n(j)-1))
16300 se(j)=ss/sqr(n(j))
16305 next j
16307 return
16900 for j=1 to l step 2
16910 n=n(j)-1:b=me(j)+me(j+1):print
16920 print"constrained line #";(j+1)/2;": y = -1 * x +";b
16930 print:gosub 5600:print"(stand. error)"
16940 print
16950 p1$="pa2":p2=b:p3=se(j)*tv:p4=se(j):gosub 5550
16960 next j
16999 return
17000 data 17,"enzyme kinetics i: michaelis-menten equation"
17010 rem
17020 rem                       subroutines: 2,3,11
17030 gosub 17100:rem                           input
17040 gosub 17200:rem                           process
17050 gosub 17900:rem                           output
17060 return
17100 dc=0
17110 px$ = "substrate concentration"
17120 py$ = "product velocity"
17130 gosub 710
17140 return
17200 px$="1/s":py$="1/v"
17210 gosub 11210:gosub 3200
17220 return
17900 for j=1 to l step 2
17910 print"line #";(j+1)/2;":",
17920 print"v max =";1/b(j),"k m =";m(j)/b(j)
17930 next j
17999 return
18000 data 18,"enzyme kinetics ii: competitive inhibition"
18010 rem
18020 rem                       subroutines:2,3,11,17
18030 gosub 18100:rem                          input
18040 gosub 18200:rem                          process
18050 gosub 18900:rem                          output
18060 return
18100 p$=""
18110 print"
you must input data for two curves.  the first must be without
an inhibitor, and the second must be in the presence of a "
18120 print p$;"competitive inhibitor."
18130 gosub 17030
18140 if l<>4 then goto 18110
18150 input"enter concentration of inhibitor ";ci
18160 return
18200 ck=ci/((1/b(1))*m(3)/(m(1)/b(1))-1)
18210 return
18900 print:print"k i =";ck
18999 return
19000 data 19,"enzyme kinetics iii: noncompetitive inhibition"
19010 rem
19020 rem                       subroutines: 2,3,11,17,18
19030 gosub 19100:rem                           input
19040 gosub 19200:rem                           process
19050 gosub 19900:rem                           output
19060 return
19100 p$="non-"
19110 gosub 18110
19120 if l<>4 then goto 19100
19140 return
19200 ratio=((m(3)/m(1))+(b(3)/b(1)))/2
19210 ci=ci/(ratio-1)
19220 return
19900 print:print"k i = ";:print using "####.##";ci
19999 return
20000 data 20,"first order drug decay"
20010 rem
20020 rem                       subroutines:2,3,4
20030 gosub 20100:rem                         input
20040 gosub 20200:rem                          process
20050 gosub 20900:rem                          output
20060 return
20100 px$="time (t0=0)":py$="conc in ug/g tissue"
20110 gosub 710
20120 return
20200 for j=1 to l step 2
20210 ifda(1,j)<>0then print"bad data, to must be 0":goto20000
20220 n(j)=n(j)-1
20230 to=da(1,j+1)
20240 for i=1 to n(j)
20250   da(i,j)=da(i+1,j)
20260   da(i,j+1)=log(to/da(i+1,j+1))
20270 next i,j
20280 py$="ln (a/a-x)":gosub 3200:gosub 4800
20290 return
20900 for j=1 to l step 2
20910  sl=xy(j)/s2(j)
20920  print"k = ";sl;" /min"
20930 next j
20999 return
21000 data 21,"scatchard plot"
21020 rem
21020 rem                      subroutines: 2,3,5
21030 gosub 21100:rem                          input
21040 gosub 5040:rem                           process
21050 gosub 21900:rem                          output
21060 return
21100 px$="bound drug concentration":py$="bound/free drug"
21200 gosub 710
21220 return
21900 for j=1 to l step 2
21910 print"k = ";-1/m(j)
21920 print"total conc. of binding sites (est.) = ";
21925 print using "####.##";-1*b(j)/m(j)
21930 next j
21999 return
22000 data 22, "henderson - hasselbach equation"
22010 rem
22020 rem                       subroutines: none
22030 input"enter a for weak acid or b for weak base";in$
22040 if in$<>"a" and in$<>"b" then 22030
22050 input"enter pka of drug";pk
22060 input"enter ph of medium";ph
22070 ra=10[(ph-pk)
22080 print"the ratio of ionized to unionized drug = ";
22090 if in$="a" then print ra else print 1/ra
22110 print
22120 input"run again (y/n) ";a$
22130 if left$(a$,1)="y" then 22000
22999 return
23000 data 23,"exponential growth & decay"
23010 rem
23020 rem                       subroutines: 2,3
23030 gosub 23100:rem                           input
23040 gosub 23200:rem                           process
23050 gosub 23900:rem                           output
23060 return
23100 px$="time":gosub 710
23110 return
23200 for j=1 to l step 2
23210 print:print"curve #";(j+1)/2;": "
23220 py$="ln x":print py$;" : ";
23230 for i=1 to n(j)
23240    da(i,j+1)=log(da(i,j+1))
23250    print da(i,j+1);
23260 next i,j
23270 gosub 3040
23280 return
23900 for j=1 to l step 2
23910 print:print"curve # ";(j+1)/2;": ";
23920 print" x = ";exp(b(j));" * e ";m(j);"t"
23930 next j
23999 return
24000 data 24,"constant infusion with first order elimination"
24010 rem
24020 rem                       subroutines: 2,3
24030 gosub 24100:rem                           input
24040 gosub 24800:rem                           output
24050 return
24100 gosub 24110:gosub 24170:return
24110 sc=1:sa$="saturation concentration "
24120 ke=0:ke$="elimination rate constant (/hr), ke "
24130 t2=0:t2$="elimination half/time, t1/2 "
24140 ri=0:ri$="infusion rate (mg/hr) "
24150 vd=0:vd$="apparent volume of distribution (liters) "
24155 p$="="
24160 print"enter values or press <enter> if unknown."
24165 return
24170 print sa$;:input"(default=1) ";sc
24180 print ke$;:input ke
24190  if ke<>0 then 24220
24200 print t2$;:input t2
24210 if t2<>0 then ke=log(2)/t2
24220 print ri$;:input ri
24230  if ri<>0 and ke<>0 then vd=ri/(sc*ke):return
24240 print vd$;:input vd
24250  if vd<>0 and ri<>0 and ke=0 then ke=ri/(sc*vd)
24260 if ke<>0 then return
24270 px$="time (minutes)"
24280 py$="concentration at t"
24290 gosub 710
24300 for j=2 to l step 2
24310  for i=1 to n(j)
24320    da(i,j)=log(1-(da(i,j)/sc))
24330 next i,j
24340 id=0:py$="ln (1-(c/cmax))"
24350 gosub 3200:gosub 4800
24360 ke=-1*sl
24370 return
24800 print:print"results:"
24810 if sc<>0 then print sa$;p$;sc
24820 if ke<>0 then print ke$;p$;ke
24830 if t2=0 and ke<>0 then t2=log(2)/ke
24840 if t2<>0 then print t2$;p$;t2
24850 if ri<>0 then print ri$;p$;ri
24860 if vd<>0 then print vd$;p$;vd
24870 gosub 540:if a$="n" then return else gosub 570
24880 print"","time","concentration"
24890 print"","----","-------------"
24900 print"time ";:gosub 590:t=x
24910 if t=999 then return
24920 c=-1*sc*(exp(-1*ke*t)-1)
24930 print"",t,c:goto 24900
24999 return
25000 data 25,"multiple iv injections"
25010 rem
25020 rem                       subroutines: 2,3,24
25030 gosub 25100:rem                           input
25040 gosub 25600:rem                           output
25050 return
25100 do=0:do$="dose of drug (mg) "
25110 ti=0:ti$="dosing interval (hrs) "
25120 fr=0:fr$="fraction remaining "
25130 nd=0:nd$="number of doses "
25140 cm=0:co$="plasma concentration (mg/ml) "
25150 pp=0:pp$="single dose "+co$
25160 pc=0:pc$="desired level as % of upper limit "
25170 gosub 24110
25180 print ke$;:input ke
25190 print nd$;:input nd
25200 print ti$;:input ti
25210   if ke<>0 and ti<>0 then fr=exp(-ke*ti)
25520 print pc$;:input pc:if pc=0 then 25240
25230   if ti=0 and nd<>0 then fr=exp(log(1-pc/100)/nd)
25240 if fr=0 then print fr$;:input fr:if fr=0 then return
25250 if nd=0 and pc<>0 then nd=int(log(1-pc/100)/log(fr)+.5)
25260 if ti=0 and ke<>0 then ti=-log(fr)/ke
25270 if ke=0 and ti<>0 then ke=-log(fr)/ti
25280 print pp$;:input pp
25290 if pp=0 then print do$;:input do:print vd$;:input vd:
        if vd<>0 then pp=do/(vd*1000)
25300 if pp=0 then return
25310 cu=pp*(1/(1-fr))
25320 cm=pp/-log(fr)
25330 cl=fr*cu
25340 return
25600 print:print"results:"
25610 if ke<>0 then print ke$;p$;ke
25620 if nd<>0 then print nd$;p$;nd
25630 if ti<>0 then print ti$;p$;ti
25640 if pc<>0 then print pc$;p$;pc
25650 if pp<>0 then print pp$;p$;pp
25660 if do<>0 then print do$;p$;do
25670 if vd<>0 then print vd$;p$;vd
25680 if fr<>0 then print fr$;p$;fr
25690 if cm<>0 then print co$;":":print "peak ";p$;cu;
      "  mean ";p$;cm;"  lower ";p$;cl
25999 return
26000 data 26,"area under a curve: trapezoidal & simpson's rules"
26010 rem
26012 rem                       subroutines: none
26014 gosub 26100:rem                           input
26016 gosub 26200:rem                           process
26018 return
26100 print"
the area under the curve must be divided into an even number of
equally spaced subintervals.  you must enter n+1 pairs of x and
y values from the curve.  enter x values first."
26110 gosub 710:return
26200 for j=1 to l step 2
26220 n=n(j):h = (da(n,j)-da(0,j))/(n-1)
26240 sy=da(1,j+1)+da(n,j+1):rem     trapezoidal
26250 for i = 2 to n-1
26260   sy=sy+(2*da(i,j+1))
26270 next i
26280 at=h*sy/2:print
26310 sy=da(1,j+1)+da(n,j+1):rem     simpson
26320 for i=2 to n-1
26330   if i/2=int(i/2) then f=4 else f=2
26340   sy=sy+(f*da(i,j+1))
26350 next i
26360 as=h*sy/3
26370 print
"area under  y( x )  between  y(";da(1,j);")  and  y(";da(n,j);")  :"
26380 print"    trapezoidal rule -  area =";at
26390 print"    simpson's rule -    area =";as
26400 print
26410 next j
26999 return
27000 data 27,"analysis of variance"
27010 rem
27020 rem                       subroutines: 2
27030 gosub 27100:rem                           input
27040 gosub 27200:rem                           process
27050 gosub 27900:rem                           output
27060 return
27100 px$="":py$="":gosub 2030
27110 return
27200 gn=0:gt=0:gm=0:ts=0:st=0:se=0:vp=0:vt=0:f=0
27300 for j=1 to l
27310  gn=gn+n(j)
27320  gt=gt+(me(j)*n(j))
27330  st=st+(me(j)*me(j)*n(j))
27340  se=se+(va(j)*(n(j)-1))
27350 next j
27360 gm=gt/gn
27370 st=st-(gm*gm*gn)
27380 vp=se/(gn-l)
27390 vt=st/(l-1)
27400 f=vt/vp
27410 return
27900 print
27910 print"source of       sum of     deg. of     mean        f"
27920 print"variation       squares    freedom     square      value"
29730 print string$(60,"-"):print"total",se+st
29740 print"between
means",st;tab(28);l-1;tab(38);vt;tab(50);f
27950 print"within
samples",se;tab(28);gn-l;tab(38);vp;
27999 return
28000 data 28,"t-test i: grouped data"
28010 rem
28020 rem                       subroutines: 2,3,5
28030 gosub 28100:rem                           input
28040 gosub 28200:rem                           process
28060 return
28100 px$="":py$="":gosub 2030
28110 return
28200 sp=0:t=0:df=0
28260 for j=2 to l
28270 a=1:b=j
28280 df=n(a)+n(b)-2
28290 sp=((n(a)-1)*va(a)+(n(b)-1)*va(b))/df
28300 di=sqr((n(a)+n(b))/(n(a)*n(b)))
28310 t=abs((me(a)-me(b))/(sqr(sp)*di))
28320 print:gosub 28900
28330 next j
28340 return
28900 print"group # 1 vs. #";j;tab(29);"pooled variance =";sp
28910 n=df:gosub 5700
28999 return
29000 data 29,"t-test ii: paired data"
29010 rem
29020 rem                subroutines: 2,3,5,28
29030 gosub 29100:rem                     input
29040 gosub 29200:rem                    process
29050 return
29100 l=2:pn$="pairs ":px$="":py$="":gosub 720
29110 return
29200 for j=1 to l step 2
29210   if n(j)<>n(j+1) then print"n's must be equal !":return
29215   print"difference: ";
29220   for i = 1 to n(j)
29230     da(i,j)=da(i,j)-da(i,j+1):print da(i,j);
29240   next i
29250   print:id=0:gosub 2660
29260   gosub 2790:rem                  output
29270   gosub 2810
29280   t=abs(me(j)/(de(j)/sqr(n(j))))
29290   df=n(j)-1:a=j:b=j+1
29300   print:gosub 2890
29310 next j
29999 return
30000 data 30,"chi-square test"
30010 rem
30020 rem                       subroutines: none
30030 gosub 30100:rem                           input
30040 gosub 30200:rem                           process
30050 gosub 30900:rem                           output
30060 return
30100 jp=1:l$="rows "
30110 print"note: in the contingency table:
     'rows' are 'groups' and 'columns' are 'observations'.
"
30120 px$="":py$=""
30130 gosub 710
30150 return
30200 xs=0:gt=0:for j=1 to l
30210 t(j)=0
30220  for i=1 to n(j)
30230    t(j)=t(j)+da(i,j)
30240  next i
30245 gt=gt+t(j)
30250 next j
30260 for j=1 to l: e(j)=t(j)/gt: next j
30270 for i=1 to n(1)
30275 s(i)=0
30280  for j=1 to l
30290    s(i)=s(i)+da(i,j)
30300  next j
30310 next i
30320 for j=1 to l
30330  for i=1 to n(j)
30340    e=e(j)*s(i)
30350    x=da(i,j)
30360    print x;"(";e;")",
30370    xs=xs+(x-e)[2/e
30380  next i
30390 print:print"row #";j;" total =";t(j)
30410 next j
30500 print"column totals:"
30600 return
30900 for i=1 to n(1):print s(i),:next i
30910 print:print "chi-square = ";xs,"d.f. = ";(l-1)*(n(1)-1)
30999 return
31000 data31,"dunnet's test"
31010 rem
31020 rem                      subroutines: 2
31030 gosub  31100:rem                           input
31040 gosub 31200:rem                            process
31050 return
31100 px$="":py$="":gosub 2030
31110 return
31200 df=0:ss=0:s2=0:t2=0
31202 for j=1 to l
31204   t2=t2+sx(j)[2/n(j):df=df+n(j):ss=ss+s2(j)
31206 next j
31210 df=df-l:s2=(ss-t2)/df:print
31220 print"s2 =";s2,:s=sqr(s2):print" s =";s," p =";l-1
31232 n=df:fi$="dunnet/p95":gosub 41000:d=tb(n)
31240 forj=2tol
31250 print:print"group # 1 vs. group #";j;":";
31260 a=s*d*sqr(1/n(j)+1/n(1)):print" a =";a
31280 di=(me(j)-me(1))
31290 print"
mean of group #";j;"exceeds mean of group # 1,
        by an amount between ";di-a;"and";di+a;"."
31300 nextj
31999 return
32000 data32,"mann-whitney u-test"
32010 rem
32020 rem                       subroutines: 2
32030 gosub 32100:rem                          input 
32040 gosub 32200:rem                          process
32050 gosub 32900:rem                          output
32060 return
32100 print"enter the sample with the smallest n first."
32110 gosub 2100
32120 return
32200 for j=1 to 2
32210 for i=1 to n(j):y(i)=da(i,j):next i
32212 for i=1 to n(j)
32220   for i1=1 to n(j)-i:c=y(i1):d=y(i1+1):if c<d then 32240
32230   y(i1)=d:y(i1+1)=c
32240 next i1,i:print"group #";j;" sorted: ";
32250 for i=1 to n(j):printy(i);:next i
32260 print
32270 if j=2 then 32280 else for i=1 to n(1):x(i)=y(i):next i
32280 next j
32290 print"
sequence of ranks (members of group #1 are indicated with ')"
32300 print:x=0:y=0:r=1:j=0
32310 i=0
32320 j=j+1:i=i+1
32330 if j>n(1) then 32420 else if i>n(2) then 32440
32340 if x(j)<y(i) then 32440 else if y(i)<x(j) then 32430
      else i1=2:m=j
32350 j1=i:r1=2*r+1:r=r+2:j=j+1:i=i+1
32360 if j>n(1) then 32380 else if x(j)<>x(j-1) then 32380
      else j=j+1
32370 goto 32390
32380 if i>n then 32400 else if y(i)<>y then 32400 
      else i=i+1
32390 r1=r1+r:r=r+1:i1=i1+1:goto 32360
32400 t=(j-m)*r1/i1:printt;"'";:x=x+t:t=(i-j1)*r1/i1:printt;
32410 y=y+t:goto 32330
32420 if i>n(2) then 32460
32430 y=y+r:printr;:i=i+1:goto 32450
32440 x=x+r:printr;"'";:j=j+1
32450 r=r+1:goto 32330
32460 u1=n(1)*n(2)+n(1)*(n(1)+1)/2-x
32470 u2=n(1)*n(2)+n(2)*(n(2)+1)/2-y:return
32900 print:print:print"n1 =";n(1),"n2 =";n(2),"n =";
32910 if n(1)<n(2) then print n(2) else print n(1)
32920 print"r1 =";x,"r2 =";y:print"u1,";u1,"u2 =";u2,"u =";
32930 if u1<u2 then print u1 else print u2
32999 return
33000 data 33,"litchfield & wilcoxon test"
33010 rem
33020 rem                       subroutines: 2,3,8,9
33030 gosub 9100:rem                            input
33040 gosub 33200:rem                           process
33050 gosub 33900:rem                           output
33060 return
33120 fori=1ton(j):printda(i,j+1);:nexti:print:return
33200 j=1
33220 if op=1 then gosub 520
33230 px$="log(dose)"
33240 gosub 8220
33250 nn=n(j):in=1:i=1:py$="probit"
33260 if y(i)< 10 or y(i)>90 then 33290
33270 n=int(y(i)+.5)
33280 da(in,j)=x(i):da(in,j+1)=pr(n):in=in+1
33290 i=i+1:if i<n(j) then 33260
33300 in=in-1
33310 n(j)=in:n(j+1)=in
33320 print:print "(4) ";:gosub 33120
33340 gosub 2650:gosub 3230:gosub 3830
33350 print:print "(5)"
33370 n(j)=nn:n(j+1)=nn
33380 for i=1 to n(j)
33390  da(i,j)=x(i):x=x(i):gosub 9840:print p;
33400  da(i,j+1)=iy
33410 next i
33420 print:print "(6) ";:gosub 33120
33440 print
33450 for i=1 to n(j)
33460  if y(i)>0 and y(i)<100 then 33490
33470  print"get correction value for";da(i,j+1);"%";
33480  print" from table a-12 ";:input y(i)
33490  da(i,j+1)=y(i)
33500 next i
33510 print:print "(7) ";:gosub 33120
33530 gosub 9240
33540 print:print "(9) ":gosub 33120
33560 for i=1 to n(j)
33570  da(i,j+1)=m(j)*da(i,j)+b(j)
33580 next i
33590 print:print "(10) ":gosub 33120
33610 for i=1 to n(j)
33620  x=da(i,j)
33630  gosub  9840
33640  da(i,j+1)=iy
33650 next i
33660 print:print "(11) ";:gosub 33120
33680 xs=0:print:print "(12) "
33690 for i=1 to n(j)
33710  g=(y(i)-da(i,j+1))[2/(da(i,j+1)*(100-da(i,j+1)))
33720  print g;:xs=xs+g
33740 next i
33750 print:input"enter total number of animals used ";na
33760 xs=xs*(na/n(j)):print "chi-square = ";xs
33770 print "enter chi-square value for d.f. = ";n(j)-2;
33780 input xt
33790 gosub 9620
33810 input"enter total # of animals between ed16 & ed84 ";np
33820 if xs<xt then f=sf[(2.77/sqr(np)):goto 33840
33825 n=n(j)-2:gosub 5600
33830 f=sf[(1.4*tv*sqr(xs/(n*np)))
33840 return
33900 rem               print results
33910 print "factor(ed50) = ";f
33920 print "ed50 = ";e5;" (";e5/f;"-";e5*f;")"
33999 return
