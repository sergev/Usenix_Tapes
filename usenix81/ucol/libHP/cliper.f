      subroutine cliper(arxy,nflag1)
c
c      this subroutine is a general clipper.  the inputs
c      are as indicated.   this is used for the soft clipping
c      region and for incremental moves.
c
      dimension arxy(4,2)
	include 'gsdata.f'
c
c      enter with:   xlast      start point for the
c                    ylast      line segment
c
c                    xcurr      end point for the
c                    ycurr      line segment
c
c      exit:
c                    arxy(1,1) implies xstart physical
c                    arxy(1,2) implies ystart physical
c
c                    arxy(2,1) implies xend physical end
c                    arxy(2,2) implies yend physical end
c
c                    arxy(3,1) implies xmin boundary
c                    arxy(3,2) implies ymin boundary
c
c                    arxy(4,1) implies xmax boundary
c                    arxy(4,2) implies ymax boundary
c
c
c
c                    nflag1 = 0=valid points no change
c                         =-1=both out of bounds
c                (1) mod(nflag1)=0 no x change
c               (10) mod(nflag1)=0 no y change
c
c                     fbound=-1 imples 16383 or max intersection
c                           = 1 imples 0 or min intersecion
c                                                boundary
c
c                     vbound = the boundary value
c
c                    nside  =  determine min or max boundary
c
c
c                     naxis = 1 imples xaxis
c                             2 imples yaxis
c
c                     jaxis = 1 imples y axis
c                     jaxis =2 2 implies xaxis
c
c                      ref = is the intercept a start endpoint
c                               or end endpoint
c
c                     arxy = the array which contains the physical
c                            start and endpoint
c
c                     arxy(1,1) = xlast
c                     arxy(1,2) = ylast
c                     arxy(2,1) = xcurr
c                     arxy(2,2) = ycurr
c
c
c            start of the procedure for clipping
c               this should clip all the points for the
c                hp7221a.      the only problem yet to be
c                solved is the boundary intercept for the
c                dashline .   as some dashlines will not appear
c                to be from the same space depending on the
c                relative scaline and windowing of the line
c                cord.
c
c                also the idea of lost mode for this
c                may also be affected as the unit may not
c                be able to keep enough resolution to keep
c                both endpoints within a range such that
c                the significance of the window intersection
c                is not valid.
c
c
c
      nflag1=0
      abound=3.0
      do 3000 nside=3,4
      abound=abound-2.0
      jaxis=3
      do 3000 naxis=1,2
      jaxis=jaxis-1
      vbound=arxy(nside,naxis)
c
c     is endpoint last inside of window boundary
c
      if (((arxy(1,naxis))-vbound)*abound) 300,500,500
c
c     is endpoint current outside of window boundary
c     thus both points out of bounds no clip done
c
c
 300  if ((arxy(2,naxis)-vbound)*abound) 1700,400,400
c
c     endpoint last is outside, endpoint current is inside of
c     boundary, calculate new endpoint last
c
 400  nref=1
      nflag1=nflag1+1
      go to 1000
c
c     if endpoint current is inside both endpoints
c     are ivbounds; skip to next boundary; else
c     calculate new endpoint curr.
c
 500  if ((arxy(2,naxis)-vbound)*abound) 510,3000,3000
 510  nref=2
      nflag1=nflag1+10
c
c     calculate the intersection of boundary vbound and
c     line segment of arxy(1,i) (endpoint last) to
c     arxy(2,i) endpoint current)
c
c     npar1=1 the last endpoint jaxis coordinate is closes to 0
c          =2 the current endpoint jaxis is closest to zero
c
c     npar2=1 the last endpoint jaxis coordinate is farthest
c                               from zero
c          =2 the current endpoint jaxis coordinate is farthest
c                                         from endpoint
c
c
 1000 npar1=1
      npar2=2
c
c     find the coordinate closest to the current point
c
      if (abs(arxy(2,naxis))-abs(arxy(1,naxis))) 1025,1050,1050
c
c     switch around the parameter order
c
 1025 npar1=2
      npar2=1
c
c     calculate the line intersection
c
 1050 arxy(nref,jaxis)=arxy(npar1,jaxis)+((arxy(npar2,jaxis)-
     &arxy(npar1,jaxis))/(arxy(npar2,naxis)-arxy(npar1,naxis)
     &))*(vbound-arxy(npar1,naxis))
c
c     assign the other coordinate point with the boundary condition
c
      arxy(nref,naxis)=vbound
c
 3000 continue
c
c      normal return with nflag1 set to 0 or 10 or 1
c
      goto 9000
c
c      both points outside the area of interest
c
 1700 nflag1=-1
 9000 return
      end
