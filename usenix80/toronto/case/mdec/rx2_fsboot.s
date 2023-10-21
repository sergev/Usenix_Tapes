/ disk boot program to load and transfer
/ to a unix file system entry
/ must be assembled with fstty.s and
/ appropriate disk driver

/ Normal operation asks for the system path name.
/ However, a default path name can be specified
/ at location "reboot".  The first word is a pointer
/ to a string containing the system path name,
/ terminated by a newline.  The default name is 
/ specified in the following bytes.  The reboot
/ system call uses this by putting a pointer to the
/ next byte in the reboot location for default operation.
/ It clears reboot to allow the system path name to be
/ specified from the console.  To reboot a system other
/ than the default system, the name is put in reboot+2
/ and a pointer is put in reboot.
/ The system path name must be 17 or fewer characters.

/ It is now possible to set the software switch register before
/ booting a program (usually unix, to bring it up single user)
/ a companion program, csw, reads the new value for the switch
/ register and saves it via the subroutine setsw in fstty.s
/ which is entered through trvect ( 6(r5) ).

/ This program must fit in one disk sector; therefore
/ it must be less than 01000 bytes long.  To put a new
/ boot on rk0 run "wrboot <a.out >/dev/rrk0".

/ reboot mods by Bill Shannon  12/14/77
/ based on NPS reboot command by Gerry Barksdale

/ software switch register mods by Bill Shannon   3/35/79

/ entry is made by jsr pc,*$0
/ so return can be rts pc

core = 24.	/ first core loc (in KW) not used
.. = [core*2048.]-1024.
start:
	br	start0
	0
reboot:	0		/ path name pointer
	<unix>		/ system path name (17 chars or less)
	<\n>		/ path name terminator
. = .+13.		/ 17 - length(path name)

/ copy self to 'core' - 512. bytes
/ strip off UNIX execute header if present

start0:
	mov	$..,sp
	mov	sp,r1
	clr	r0
	cmp	pc,r1
	bhis	2f
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	mov	(r0)+,(r1)+
	cmp	r1,$core*2048.
	blo	1b
	jmp	(sp)

/ clear all but low core, path name
/ stays there when we're relocated.

2:
	mov	$inod,r0
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b

/ prompt
/ read in path name
/ breaking on '/' into 14 ch names

	mov	$trvect,r5
	jsr	pc, mesg; <\r\n@\0>; .even
	mov	$names,r2
1:
	mov	r2,r1
2:
	jsr	pc,getc
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'/
	beq	3f
	movb	r0,(r1)+
	br	2b
3:
	cmp	r2,r1
	beq	2b
	add	$14.,r2
	br	1b
1:

/ start of path name decoding
/ start with first name  and root ino

	clr	reboot		/ one try is enough
	mov	$names,r2
	mov	$1,r0

/ get next inode

1:
	clr	bno
	jsr	pc,iget
	tst	(r2)
	beq	1f

/ read next directory looking for next name

2:
	jsr	pc,rmblk
	br	start
	mov	$buf,r1
3:
	mov	r2,r3
	mov	r1,r4
	add	$16.,r1
	tst	(r4)+
	beq	5f
4:
	cmpb	(r3)+,(r4)+
	bne	5f
	cmp	r4,r1
	blo	4b
	mov	-16.(r1),r0
	add	$14.,r2
	br	1b
5:
	cmp	r1,$buf+512.
	blo	3b
	br	2b

/ last entry was found
/ read into 0.

1:
	clr	r2
1:
	jsr	pc,rmblk
	br	callout
	mov	$buf,r1
2:
	mov	(r1)+,(r2)+
	cmp	r1,$buf+512.
	blo	2b
	br	1b

/ subroutine will read in inode
/ number specified in r0
iget:
	add	$31.,r0
	mov	r0,r5
	ash	$-4.,r0
	jsr	pc,rblka
	bic	$!17,r5
	ash	$5.,r5
	add	$buf,r5
	mov	$inod,r4
1:
	mov	(r5)+,(r4)+
	cmp	r4,$addr+16.
	blo	1b
	rts	pc

/ routine to read in block
/ number specified by bno
/ after applying file system
/ mapping algorithm in inode.
/ bno is incremented, success
/ return is a skip, error (eof)
/ is direct return.
rmblk:
	add	$2,(sp)
	mov	bno,r0
	inc	bno
	bit	$LRG,mode
	bne	1f
	asl	r0
	mov	addr(r0),r0
	bne	rblka
2:
	sub	$2,(sp)
	rts	pc
/ large algorithm
/ huge algorithm is not implemented
1:
	clr	-(sp)
	movb	r0,(sp)
	clrb	r0
	swab	r0
	asl	r0
	mov	addr(r0),r0
	beq	2b
	jsr	pc,rblka
	mov	(sp)+,r0
	asl	r0
	mov	buf(r0),r0
	beq	2b
rblka:
	mov	r0,dska
	br	rblk

ba:	buf
wc:	-256.
sw:	0		/ for software switch register
.bss
end:
inod = ..-1024.
mode = inod
addr = inod+8.
buf = inod+32.
bno = buf+514.
dska = bno+2
names = dska+2
LRG = 10000
reset = 5
.text
