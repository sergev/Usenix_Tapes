program dtest
#
#	exercise disvec routines
#	writing is sequential.  reading is randomized.
#
#	a test run using the block file system with
#		kw = 1 000 000
#	took 7:09 system time,  2:26 user time,  81:28 real time.
#	the job issued 1975252 reads and 15626 writes.
#
integer*4nw,kw,jw,irand
integer set,put
real*8xx,yy,get
#
#	omitting the next call produces a scratch file which
#	disappears on program exit.
#
repeat {
	read(5,*)kw
	if (kw<=0)
		break 1
	write(6,*)" begin kw =",kw
	nerror = 0
	npass = 0
#
#	here we define two arrays, each of length kw
#
	ii = set(kw)
	jj = set(kw)
	t = secnds(0.)
#
#	fill vectors
#
	do nw = 1,kw {
		xx = (dfloat(nw)**2)
		yy = 1.d0/nw
		i = put(ii,nw,xx)
		j = put(jj,nw,yy)
		if (i<1||j<1)
			write(6,*)" x",i,j,nw,xx,yy
		}
#
#	check contents in a random order
#
	do nw = 1,kw {
		jw = irand(nw)
		xx = get(ii,jw)
		yy = get(jj,jw)
		if (xx==dfloat(jw)**2)
			if (yy==1.d0/jw)
				next 1
		write(6,*)" error",nw,jw,kw,xx,yy
		nerror = nerror+1
		if (nerror>10)
			break 2
		}
#
#	finished
#
	npass = npass+1
	write(6,*)" end of pass",npass,"--distst",secnds(t)," seconds."
	write(6,*)nerror," errors."
#
#	print some statistics on the standard output
#
	call dsvprt()
#
#	reset disvec to its initial condition to start all
#	over again.
#
	call dreset()
	}
stop
end



integer*4function irand(kr)
integer*4kr,jx
static i,j
data i,j/1,1/
repeat {
	jx = ran(i,j)*(kr+1)
	if (jx>0)
		if (jx<=kr)
			break 1
	}
irand = jx
return
end



