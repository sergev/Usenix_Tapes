	subroutine teread(int1,int2,int3,int4,nrd)
      character*56 jbuf
      character cprmt
	include 'gsdata.f'
      call loadch(cprmt,1,17)
      call ostr(mdevot,cprmt,1)
c      read(mdevin,10,err=100,iostat=ix) jbuf
c      if(ix.ne.0) goto 100
      call cred(mdevin,jbuf)
  10  format(a)
      if(nrd.eq.1) read(jbuf,"(i6)") int1
      if(nrd.eq.2) read(jbuf,"(i6,i6)") int1,int2
      if(nrd.eq.3) read(jbuf,"(i6,i6,i6)") int1,int2,int3
      if(nrd.eq.4) read(jbuf,"(i6,i6,i6,i6)") int1,int2,int3,int4
      call loadch(cprmt,1,20)
      call ostr(mdevot,cprmt,1)
      return
 100  print *,'teread error',mdevot,mdevin
      stop
      end
