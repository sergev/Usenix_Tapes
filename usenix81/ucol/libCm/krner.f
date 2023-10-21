      subroutine krner(al,be,de,be2,norm,itype,nwal,np,bk)
c  subroutine computes the coefficients of nearly equiripple
c  linear phase shift filters.  inputs are:
c      al - stopband attenuation in db
c      be - location of center of transition band, in nyquists
c      de - width of transition band, in nyquists
c      be2 - location of center of other transition band, in
c            nyquists (used if itype = 3 or 4). be2 > be
c      norm - if norm = 1, filter will be normalized to have exactly
c             unity or zero gain at dc (as appropriate).  in this case the
c             average passband gain will not in general be 1.
c      itype -  1 for lowpass
c               2 for highpass
c               3 for bandpass (from be to be2)
c               4 for bandstop (from be to be2)
c               5 for differentiator with cutoff at be
c      nwal - dimension of bk in calling program
c  outputs are:
c      np - number of filter weights returned - 1. total number used is
c           2*np + 1
c      bk - array of filter coefficients, from "center to right"
c           (i.e., for second half of filter).  first half should
c           be symmetric (in cases 1-4) or antisymmetric (case 5).
c
c       calls ai0 (to compute modified bessel function of the first kind).
c
c   coded by duncan agnew, november 1978
c   after j.f. kaiser and w.a. reed, rev sci instr, 48, 1447 (1977)
c   and 49, 1103 (1978)
c
      dimension bk(1)
      data pi/3.1415926536/
      fk = 1.8445
c  compute number of coefficients needed to fufill given parameters
      if(al.ge.21.) fk = .13927*(al-7.95)
      if(al.le.21.) et = 0.
      if(al.gt.21.and.al.le.50) et = .58417*((al-21.)**.4)
     1   +  .07886*(al-21.)
      if(al.gt.50.) et = .1102*(al-8.7)
      np = int((fk/(2.*de)) + .75)
c  test np against limit in dimensioning
      if(np.gt.nwal) go to 70
      fnp = np
      fia = ai0(et)
      do 10 k = 1,np
      fk = k
      gk = pi*fk
      ge = et*sqrt(1.-(fk/fnp)**2)
      e = ai0(ge)/fia
      if(itype.eq.1.or.itype.eq.2) bkt = sin(be*gk)/gk
      if(itype.eq.3.or.itype.eq.4) bkt = (sin(be*gk)
     & - sin(be2*gk))/gk
      if(itype.eq.5) bkt = -(be/fk)*cos(be*gk)
     & + sin(be*gk)/(fk*fk*pi)
      bk(k) = bkt*e
 10   continue
      n = np + 1
      do 20 i = 2,n
      k = n-i+2
      j = k-1
 20   bk(k) = bk(j)
      if(itype.eq.1.or.itype.eq.2) bk(1) = be
      if(itype.eq.3.or.itype.eq.4) bk(1) = 1. + be - be2
      if(itype.eq.5) bk(1) = 0.
      if(itype.eq.5) return
c  normalize coefficients if requested
      if(norm.ne.1) go to 50
      wt = bk(1)
      do 30 i = 2,n
 30   wt = wt + 2.*bk(i)
      do 40 i = 1,n
 40   bk(i) = bk(i)/wt
 50   if(itype.eq.1.or.itype.eq.4) return
c  reset coefficients for highpass or bandpass
      do 60 i = 2,n
 60   bk(i) = -bk(i)
      bk(1) = 1. - bk(1)
      return
 70   write(0,100) np,nwal
 100  format(i7,' weights is more than the',i5,
     &'dimensioned externally'/)
      return
      end
