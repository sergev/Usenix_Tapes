	subroutine terstr(mdevot,length,line)
c
c	length bytes starting at line are written
c	to unit mdevot using qio with wal.
c
c	control is not returned until the write
c	has been sucessfully completed.
c
	character*202 line
	if(length.le.0) return
	if(mdevot.ge.0) go to 200
100	print *,' terstr error ',mdevot
        stop
 200  call ostr(mdevot,line,length)
      return
      end
