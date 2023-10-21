22a
	cmp	buf,r1
	bne	er1
	sys	open; dev; 0
	bes	er1
.
21c
	mov	buf,r1
	sys	stat;dev;buf
.
4a
.globl	ttyn, _ttyn

.
3c
/	modified 05-Jun-1980 by D A Gwyn:
/	1) check device as well as inode for match.
.
1c
/ ttyn.s - return name of current tty
.
w
q
