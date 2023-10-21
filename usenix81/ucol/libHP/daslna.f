      subroutine daslna(lintyp,patlen)
	include 'gsdata.f'
c
c     this is used to invoke the dashline pattern of the
c     plotter.
c
c     put the tilda and d to imply dash line absolute
c     type in.
c
      mloccm=91
c
      if ((lintyp.le.0).or.(lintyp.gt.9)) lintyp=1
c
      ylen=0.0
      tdllen=abs(patlen)
      call delta(tdllen,ylen)
      if (tdllen.lt.20.0) tdllen=20.0
      if (tdllen.gt.32767.0) tdllen=32767.0
c
      if ((lintyp.eq.1).and.(mlntyp.eq.1)) goto 6000
      if ((lintyp.eq.mlntyp).and.(xdllen.eq.tdllen)) goto 6000
c
c
c
c
c      check to make sure there is enough room in the host
c      buffer
c
      call datout(22)
c
c
      moutar(mcount)=126
      mcount=mcount+1
      moutar(mcount)=81
      mcount=mcount+1
c
c
c
c
c
c     branch to the proper dash line pattern
c
      goto (5000,200,300,400,500,600,700,800,900),lintyp
c
c     the first one going to 5000 is solid line which
c     has not parameters to fill in
c
c                                            endpoints
c                                    start                end
c                                      .                   .
c
c     solid lines is line type #1      _____________________
c
c
c     short dash lines    type #2      _____
c                                       5           15
 200  call nmp(37)
      call nmp(15)
      goto 5000
c
c     median dashlines    type #3      __________
c                                         10          10
 300  call nmp(42)
      call nmp(10)
      goto 5000
c
c     long dashlines      type #4      _______________
c                                             15          5
 400  call nmp(47)
      call nmp(5)
      goto 5000
c
c     long dot dashline   type #5      ______________  .
c                                            16      2 0 2
 500  call nmp(48)
      call nmp(2)
      call nmp(32)
      call nmp(2)
      goto 5000
c
c     partial dashlines   type #6      ______________  __
c                                             14     2 2 2
 600  call nmp(46)
      call nmp(2)
      call nmp(34)
      call nmp(2)
      goto 5000
c
c
c     double partial dash type #7      __________  __  __
c                                           10   2 2 2 2 2
 700  call nmp(42)
      call nmp(2)
      call nmp(34)
      call nmp(2)
      call nmp(34)
      call nmp(2)
      goto 5000
c
c     endpoints dashline  type #8      .                     .
c                                      0         20         0
 800  call nmp(32)
      call nmp(20)
      call nmp(32)
      goto 5000
c
c
c     if you wish to set up alternative dashline
c     dashline pattern.  set the format here or
c     you may change the above if you know what
c     which types you do not want.
c
c
c
c     currently this is used for the user defined.
c     if you wish to define one here please remove the
c     statement lintyp=1 as it implies that there is
c     no patlen to be sent out
c
c     insert the call to nmp for the various patterns you
c     want.  you may even change the standard line types
c     if you wish.
c
c
 900  lintyp=1
c
 5000 if (lintyp.eq.1) goto 6000
      call agp(tdllen)
 6000 mlntyp=lintyp
      xdllen=tdllen
      return
      end
