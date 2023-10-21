/ C library -- gprocs   (rand:rbg)

/ numproc = gprocs(buf)
/ buf is size NPROC * sizeof(proc)

	gprocs = 61.

.globl  _gprocs

_gprocs:
	mov     2(sp),r0
	sys     gprocs
	rts     pc
