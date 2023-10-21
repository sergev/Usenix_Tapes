/	fsboot -- disk boot program to load and transfer (returnably)
/			to a UNIX file system entry
/
/	last edit: 16-Mar-1981	D A Gwyn
/
/ 1) removed "transfer vector" for tty;
/ 2) implemented backspace and Ctrl/U erase & kill characters.
/
/	must be assembled with tty.s, rXX.s, & XX.s where "XX" is name of disk
/
/	entry to UNIX program is made by jsr pc,*$0 so return can be rts pc

HIMEM = 24.*2048.		/ first memory loc not used (below ABSLOD)
BLOCK = 512.			/ size of disk block
.. = HIMEM-BLOCK		/ starting address of relocated bootstrap
				/ also assembler relocation counter


/ relocate bootstrap code; strip UNIX exe header if present

start:
	mov	$..,sp		/ initial stack pointer
	mov	sp,r1		/ as well as new absolute start address

	clr	r0		/ -> loc. 0
	cmp	pc,r1
	bhis	2f		/ if relocated, skip relocation!

	cmp	(r0),$407
	bne	1f		/ test for exe header

	mov	$20,r0		/ -> first text loc.
1:
	mov	(r0)+,(r1)+	/ relocate word
	cmp	r1,$HIMEM
	blo	1b		/ until all BLOCK bytes are moved

	jmp	(sp)		/ jump to new "start" entry

/ begin execution: clear all of memory below bootstrap (includes `names')

2:
	clr	(r0)+		/ clear word
	cmp	r0,sp		/ stop before clobbering the bootstrap
	blo	2b

	jsr	pc,mesg; <\n@\/\0>; .even	/ prompt user "@/"

/ read in pathname, breaking on `/' into successive 14-char portions

	mov	$names,r2	/ storage for 14-char pathname portions
1:
	mov	r2,r1		/ beginning of next portion
2:
	jsr	pc,getc		/ get char from user

	cmpb	r0,$25		/ ctrl/U
	beq	start		/ start over on line-kill

	cmpb	r0,$10		/ ctrl/H (backspace)
	bne	5f

/ character-erase processing.  complicated because of / portions

	jsr	pc,mesg; .byte '\s,10,0; .even	/ space erases char
	cmp	r1,r2		/ see if entire portion already erased
	bhi	4f

	cmp	r2,$names	/ yes, see if entire pathname erased
	beq	start		/ yes, restart same as for line-kill

	sub	$14.,r2		/ -> previous portion
3:
	tstb	-(r1)		/ find end of previous portion
	beq	3b		/ loop until found (must be one)

	inc	r1		/ -> next empty char slot
	br	2b		/ try again (beginning of portion)
4:
	clrb	-(r1)		/ remove previous input char from `names'
	br	2b		/ try again (inside same portion)
5:
	cmpb	r0,$'\n
	beq	1f		/ if done

	cmpb	r0,$'/
	beq	3f		/ each `/' starts a new pathname portion

	movb	r0,(r1)+	/ stash away and get next char
	br	2b
3:
	cmp	r1,r2		/ make sure there is something in portion
	bhi	3f

	jsr	pc,mesg; .byte 10,'\s,10,0; .even	/ erase `/'
	br	2b		/ try again (beginning of same portion)
3:
	add	$14.,r2		/ point to next name storage
	br	1b		/ get next portion of pathname

/ start of path name decoding; start with first portion and root inode

1:
	mov	$names,r2	/ -> pathname portions
	mov	$1,r0		/ root inode is always # 1

/ get next inode

1:
	clr	bno		/ first block address in inode
	jsr	pc,iget
	tstb	(r2)
	beq	1f		/ if no more portions of pathname, roll in

/ read next directory block, looking for next portion of pathname

2:
	jsr	pc,rmblk	/ get next directory block
		br start	/ if no more (resets SP)
	mov	$buf,r1
3:
	mov	r2,r3		/ -> name to be found
	mov	r1,r4		/ -> current directory entry
	add	$16.,r1		/ -> next directory entry
	tst	(r4)+		/ skip i-number
	beq	5f		/ skip if empty directory entry
4:
	cmpb	(r3)+,(r4)+
	bne	5f		/ not a match
	cmp	r4,r1
	blo	4b		/ if more chars to check

	mov	-16.(r1),r0	/ address of matching entry
	add	$14.,r2		/ -> next name to look for
	br	1b		/ successful search
5:
	cmp	r1,$buf+BLOCK
	blo	3b		/ more entries to be searched
	br	2b		/ end of directory block

/ last portion of pathname was found; read into 0.

1:
	clr	r2		/ starting at loc. 0
1:
	jsr	pc,rmblk	/ read next block according to inode
		br callout	/ when done, execute
	mov	$buf,r1
2:
	mov	(r1)+,(r2)+	/ copy from input buffer to final loc.
	cmp	r1,$buf+BLOCK
	blo	2b		/ more input to copy

	br	1b		/ read next block


/	iget -- read in inode
/
/	To use: load R0 with inode number; then jsr pc,iget

iget:
	add	$31.,r0		/ there are 16 inodes per block; 32 bytes each
	mov	r0,r5
	ash	$-4.,r0		/ get inode's block number
	jsr	pc,rblka	/ read in one block full of inodes
	bic	$!17,r5		/ get inode's offset within block
	ash	$5.,r5		/ there are 32. bytes per inode
	add	$buf,r5		/ -> correct inode
	mov	$inod,r4	/ storage for inode
1:
	mov	(r5)+,(r4)+	/ save inode in buffer
	cmp	r4,$addr+16.	/ see if we have enough
	blo	1b

	rts	pc		/ now have inode saved


/	rmblk -- read in block number specified by bno (which is incremented)
/		 after applying file system mapping algorithm in inode.
/
/	To use:	jsr	pc,rmblk
/			(error/EOF return)
/		(succesful read return)

rmblk:
	add	$2,(sp)		/ assume success
	mov	bno,r0		/ inode block-address index
	inc	bno
	bit	$LRG,mode	/ test for "large file" algorithm
	bne	1f

/ "small file" algorithm:

	asl	r0		/ to index words
	mov	addr(r0),r0	/ get physical block number
	bne	rblka		/ read the block from disk and return
2:
	sub	$2,(sp)		/ no more blocks -- EOF.
	rts	pc

/ "large file" algorithm ("huge file" algorithm not implemented):
1:
	clr	-(sp)
	movb	r0,(sp)		/ index into indirect block
	clrb	r0
	swab	r0		/ indirect block index
	asl	r0		/ to index words
	mov	addr(r0),r0	/ get physical indirect block number
	beq	2b		/ no more indirect blocks
	jsr	pc,rblka	/ read the indirect block from disk
	mov	(sp)+,r0	/ index into indirect block
	asl	r0		/ to index words
	mov	buf(r0),r0	/ get physical data block number
	beq	2b		/ no more data blocks
	/ fall through to "rblka"


/	rblka -- read disk block specified by R0 into `buf'

rblka:
	mov	r0,dska
	br	rblk		/ read the block then return

ba:	buf			/ absolute address of buf
wc:	-256.			/ words to transfer on read

.bss
end:				/ MUST be <= 512.

inod = ..-BLOCK-BLOCK		/ inode buffer
mode = inod			/ -> flags
LRG = 10000			/ "large file" algorithm
addr = inod+8.			/ -> block numbers
buf = inod+32.			/ 1-block disk input buffer
bno = buf+BLOCK			/ 1-word disk block number
dska = bno+2			/ 1-word disk block address
names = dska+2			/ sequence of 14-char parts of pathname
/ from here to .. is used for a stack
.text
