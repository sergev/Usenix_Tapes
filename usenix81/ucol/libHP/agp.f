      subroutine agp(x)
	include 'gsdata.f'
c
c      this is used to convert the passed parameter value
c      to the agp binary format.
c
c
c
       if (x.lt.0.0) x=0.0
       if (x.gt.32767.0) x=32767.0
 25    numb=ifix(x+.5)
       call datout(3)
c
c      numb=no is the number to be converted
c
       nopts=1
       if (numb.gt.15) nopts=2
       if (numb.gt.1023) nopts=3
       do 100 j=mcount+nopts-1,mcount,-1
       new=numb/2**6
       moutar(j)=numb-new*2**6
       if (j.eq.mcount) moutar(mcount)=moutar(mcount)+96
       if(moutar(j).lt.32) moutar(j)=moutar(j)+64
       numb=new
 100   continue
       mcount=mcount+nopts
       mloccm=16
 200   return
       end
