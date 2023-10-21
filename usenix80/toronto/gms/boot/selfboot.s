/ Selfboot is the bootstrap program for the self-booting tape. It does the
/ following tasks:
/    1. Copies itself from core locations 000000-000777(octal) to core
/	locations 157000-157777(octal).
/    2. Copies 5500 1-block (512-byte) records from the tape (starting at
/	record 2) to the disk (starting at block 0).
/    3. Copies 110 1-block (512-byte) records from the tape (starting at
/	record 5502) to core memory (locations 000000-155777(octal)).
/    4. Transfers control to core location 000000(octal).
/
/ The tape should have the following format:
/ record 0 (512 bytes) - copy of this bootstrap program
/ record 1 (512 bytes) - copy of this bootstrap program
/ record 2 - record 5501 (512 bytes each) - copy of a root file system
/ record 5502 - record 5611 (512 bytes each) - copy of an operating system
/	(with the a.out header removed)
/ NOTE: the tape must be 800 BPI.
/
/ In general, the tape should be created with the makeboot program.
/
/ compile: as -o selfboot selfboot.s ; strip selfboot

/ The use of registers in this program is as follows:
/ r0 - scratch register
/ r1 - scratch register
/ r2 - loop counter
/ r3 - core location to read to from the tape
/ r4 - block number to write to on the disk

/ main program

/ set up the stack pointer
	mov	$157000,sp
/ check if the program needs to be copied up
	cmp	pc,sp
	bhis	2f
/ copy the program up
	clr	r0
	mov	sp,r1
	mov	$256.,r2
1:
	mov	(r0)+,(r1)+
	sob	r2,1b
	jmp	(sp)
2:
/ rewind the tape
	jsr	pc,rewind
/ read over the first two records
	clr	r3
	jsr	pc,taperead
	jsr	pc,taperead
/ copy the next 5500 records from tape to disk
	mov	$5500.,r2
	clr	r4
3:
	jsr	pc,taperead
	jsr	pc,diskwrite
	inc	r4
	sob	r2,3b
/ copy the next 110 records into core
	mov	$110.,r2
4:
	jsr	pc,taperead
	add	$512.,r3
	sob	r2,4b
/ rewind the tape
	jsr	pc,rewind
/ transfer control to location 0
	clr	pc

/ device drivers

/ TM11 magnetic tape device subroutines

mts   = 172520		/ status register
mtc   = 172522		/ command register
mtbrc = 172524		/ byte record counter
mtcma = 172526		/ current memory address register
mtd   = 172530		/ data buffer
mtrd  = 172532		/ TU10 read lines

/ subroutine to read a record from the tape

taperead:
/ set up registers and read tape
	mov	$mtcma,r0
	mov	r3,(r0)
	mov	$-512.,-(r0)
	mov	$60003,-(r0)
/ check to see if read is finished
1:
	tstb	(r0)
	bpl	1b
/ check for error during read
	tst	(r0)+
	bmi	2f
	rts	pc
/ backspace the tape
2:
	mov	$-1.,(r0)
	mov	$60013,-(r0)
/ check to see if backspace is finished
3:
	tstb	(r0)
	bpl	3b
/ try to read the record again
	br	taperead

/ subroutine to rewind the tape

rewind:
/ set up registers and rewind tape
	mov	$mtc,r0
	mov	$60017,(r0)
/ check to see if controller is ready
1:
	tstb	(r0)
	bpl	1b
	rts	pc

/ RJP04 disk pack device subroutines

rpcs1 = 176700		/ control and status 1 register
rpwc  = 176702		/ word count register
rpba  = 176704		/ UNIBUS address register
rpda  = 176706		/ desired sector/track register
rpcs2 = 176710		/ control and status register
rpds  = 176712		/ drive status register
rper1 = 176714		/ error register 1
rpas  = 176716		/ attention summary register
rpla  = 176720		/ look-ahead register
rpdb  = 176722		/ data buffer register
rpmr  = 176724		/ maintenance register
rpdt  = 176726		/ drive type register
rpsn  = 176730		/ serial number register
rpof  = 176732		/ offset register
rpdc  = 176734		/ desired cylinder register
rpcc  = 176736		/ current cylinder register
rper2 = 176740		/ error register 2
rper3 = 176742		/ error register 3
rpec1 = 176744		/ ECC position register
rpec2 = 176746		/ ECC pattern register

/ subroutine to write a record to the disk

diskwrite:
/ calculate the address on the disk
	mov	r4,r1
	clr	r0
	div	$22.,r0
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$19.,r0
	bisb	r1,1(sp)
/ set up registers and write disk
	mov	r0,*$rpdc
	mov	$rpda,r0
	mov	(sp)+,(r0)
	clr	-(r0)
	mov	$-256.,-(r0)
	mov	$61,-(r0)
/ check to see if write is finished
1:
	tstb	(r0)
	bpl	1b
	rts	pc
