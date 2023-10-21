
;------------------------------------------------------------------------------
;	Talking console device driver for the AT&T PC6300.
;	Written by Karl Dahlke, September 1986.
;	Property of AT&T Bell Laboratories, all rights reserved.
;	This software is in the public domain and may be freely
;	distributed to anyone, provided this notice is included.
;	It may not, in whole or in part, be incorporated in any commercial
;	product without AT&T's explicit permission.
;------------------------------------------------------------------------------

;	talkcon.asm: assembly base and support for talking device driver

;	Acts as console input and output.
;	Not compatible with ansi.sys.
;	Device Driver skeleton taken from public domain,
;	written by Frank Whaley.
;	Modified to make a base for a talking device driver,
;	making the AT&T pc6300 a talking computer for the blind.

;	The real time interrupt switches stacks,
;	since there are a lot of function calls in it,
;	and it holds the CPU for quite a while.
;	The other interrupts and the device driver
;	use the existing stacks.  In general, there is very little
;	documentation regarding stacks.  How much room is required
;	for DOS interrupts, worst case?  How much room is available on system
;	stacks (e.g. when device drivers get control)?
;	It makes it difficult to write robust programs.
;	Well, MS-DOS drivers seem quite unconcerned about stacks,  so,
;	For now, we just *hope* there is enough room.
;	The real time interrupt stack tromps right over the
;	initialization code, saving memory.

	include parms.h

PGROUP  group   PROG, DATA, UDATA
	assume  cs:PGROUP, ds:PGROUP

PROG    segment para public 'PROG'

	extrn   chkfifo:near, transkey:near, keycmd:near, reading:near
	extrn ss_ready:near, ss_init:near
	extrn putfifo:near
	public incbptr, decbptr
	public crsound, click, bell

	org     0 ; Device drivers start at 0

HDR     proc    far
;	Device Header
	dd      -1 ; -> next device
	dw	0c003h ; attributes: character, ioctl, stdin, and stdout
	DW      strategy ; -> device strategy
	DW      interrupt ; -> device interrupt
	db  "CON     " ; new console driver
;	the following variables are usually, but not always,
;	accessed through cs.
rh1ptr	label word ; to access by words
rhptr   dd      0 ; -> Request Header
orig_rti dw 0,0 ; original real time interrupt vector
orig_kbi dw 0,0 ; original keyboard interrupt vector
orig_bkb dw 0,0 ; original bios keyboard interrupt vector
zerods	dw 0
charin	db 0 ; one character look ahead

strategy:
;	squirrel away pointer to the msdos request
;	I don't understand why dos doesn't just call the interrupt routine directly,
;	and bypass this intermediate, seemingly unnecessary strategy routine.
	mov    cs:rh1ptr,BX ; save request header ptr
	mov    cs:rh1ptr + 2,ES
	ret

interrupt:
;	device driver interrupt routine,  *not* a true interrupt routine.
;	Perform the action requested by dos,
;	request structure is pointed to by rhptr.
;	We save all registers, gather relevant information
;	from the request header,
;	call the appropriate routine,
;	restore registers, and return.

	pushf
	push ds
	push es
	push ax
	push bx
	push cx
	push dx
	push si
	push di
	cld
	sti
	push cs
	pop ds

;	get relevant information from request header,
;	for the functions to use
	les di,rhptr
	mov al,es:[di+2] ; ax = Command Code
	cbw
;	set default status = DONE
	mov es:[di+3],100h
	mov cx,es:[di+18]
	les di,es:[di+14]

	; call our functions
	cmp al,12
	jb cmdok
;	command out of range
	mov al,2 ; replace with an illegal command
cmdok:	mov si,ADDR:functab
	add si,ax
	add si,ax
	call [si]

	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	pop es
	pop ds
	popf
	ret

HDR     endP


;	null function, for irrelevant comands
nulf	proc near
	les di,rhptr
	mov es:[di+3],8103h ; unknown function for a character device driver
	ret
nulf	endp

;	init function, set buffer, set interrupt vectors, set memory usage
init	proc near
;	get buffer size from config.sys line
;	size specified in 1024 byte blocks, consistent with ramdisk.dev
	les di,rhptr
	les di,es:[di+18] ; es:di = config.sys command line
;	Not the most brilliant or robust parsing technique
;	Find the first digit, and crack the buffer size
;	we assume the file name has no digits in it
	mov cx,1 ; default buffer size
	mov ah,0
init1:	inc di
	mov al,es:[di]
	cmp al,10 ; linefeed terminates the line
	jz init2 ; no buffer size specified
	cmp al,'0'
	jb init1
	cmp al,'9'
	ja init1
;	digit found, size specified
	xor cx,cx
init3:	xchg ax,cx
	mov bl,10
	mul bl
	add ax,cx
	sub ax,'0'
	cmp ax,64
	jae initerr ; buffer definitely too big
	xchg ax,cx
	inc di
	mov al,es:[di]
	cmp al,'0'
	jb init2
	cmp al,'9'
	jbe init3

init2:	xchg ax,cx
	mov cl,10
	shl ax,cl
	jnz init4 ; non-zero length buffer
	mov ax,1024
init4:
	mov bufsiz,ax
	add ax,ADDR: UDATA ; add start of udata segment
	jnc initok ; within small memory model
initerr:
;	exceeds small memory model
;	set buffer to 1024, default
;	we assume there is always room for this
	mov ax,1024
	mov bufsiz,ax
	add ax,ADDR: UDATA
initok:
;	make room for word correction table
	add ax,WDFIXLEN
	jc initerr ; exceeds small memory model
	mov buftop,ax
;	set end address
	les di,rhptr
	mov es:[di+14],ax
	mov es:[di+16],ds
	sub ax,bufsiz
	mov bufbot,ax
	mov buftail,ax ; circular buffer empty
	mov bufhead,ax
	mov bufcur,ax
	xchg ax,bx
	mov byte ptr [bx],80h
	mov bx,ADDR:  UDATA ; default, empty word correction table
	mov byte ptr [bx],0

;	set interrupt vectors
;	should be done using dos calls 25h and 35h,
;	but only calls 00h through 0ch and 30h are supported
;	during device driver init phase.
;	Disk Operating System Technical Reference Programming Manual
;	[IBM inc.] chapter 3.
;	Therefore, we disable interrupts,
;	and write the lowmem vectors directly
	xor bx,bx
	mov ds,bx
	cli ; rest of init procedure has interrupts disabled
	mov ax,[bx+20h] ; save original real time interrupt vector
	mov cs:orig_rti,ax
	mov ax,[bx+22h]
	mov cs:orig_rti+2,ax
	mov word ptr [bx+20h],ADDR:ih_rti  ;  real time interrupt
	mov [bx+22h],cs
	mov ax,[bx+24h] ; save original keyboard interrupt vector
	mov cs:orig_kbi,ax
	mov ax,[bx+26h]
	mov cs:orig_kbi+2,ax
	mov word ptr [bx+24h],ADDR: ih_kbi
	mov [bx+26h],cs

;	take over BIOS keyboard interrupt,
;	to accommodate our internal type ahead buffer (long).
	mov ax,[bx+58h] ; save original bios keyboard interrupt vector
	mov cs:orig_bkb,ax
	mov ax,[bx+5ah]
	mov cs:orig_bkb+2,ax
	mov word ptr [bx+58h],ADDR:ih_bkb ; BIOS keyboard interrupt
	mov [bx+5ah],cs
	mov word ptr [bx+0a4h],ADDR:ih_putc ; putchar interrupt
	mov [bx+0a6h],cs
	mov word ptr [bx+6ch],ADDR:ih_ctrlc ; control C entered
	mov [bx+6eh],cs

;	warning: called with ds = 0
	call ss_init ; init speech synthesizer
	push cs
	pop ds

;	flush input buffer
inflush:
	cli
	mov charin,0
;	empty our own keyboard buffer
	mov kbhead,ADDR: kbbuf
	mov kbtail,ADDR: kbbuf
	sti

;	call original bios routine to clear accumulated MS-DOS characters
drain:	mov ah,1
	pushf
;	call far ptr [orig_bkb], I can't figure out the masm syntax for this
	db 0ffh, 01eh
	dw orig_bkb
	jz outflush
	mov ah,0 ; remove character
	pushf
;	call far ptr [orig_bkb]
	db 0ffh, 01eh
	dw orig_bkb
	jmp short drain

outflush: ; no output buffer, always flushed
	outstat: ; always ready
	ret
init	endp

;	the console always returns "ready" upon receiving an input status request
;	seems like a bug to me.
;	ms-dos must use the ndinput call to ascertain status
;	I combine the two calls here, they are quite similar.
ndinput	proc near
	mov dl,al ; remember the command code
	les di,rhptr
;	code stolen from console device driver, header at 800h
nd2:	mov al,charin
	or al,al
	jnz ndok
	mov bx,kbtail
	cmp bx,kbhead
	mov ax,[bx]
	jz busyset
;	I don't understand how ax could ever be 0
;	if it is, skip the character and try again
	or ax,ax
	jnz nd1
	inc bx
	inc bx
	cmp bx,ADDR: kbbuf+KBSIZ*2
	jnz wrap4
	mov bx,ADDR: kbbuf
wrap4:	mov kbtail,bx
	jmp nd2
nd1:	cmp ax,7200h
	jnz ndok
	mov ax,1910h
ndok:	cmp dl,5 ; was it a non distructive input request?
	jnz nd3
	mov es:[di+13],al
nd3:	ret
busyset:
	mov es:[di+3],200h ; set busy bit, waiting for characters
	ret
ndinput	endp

input	proc near
	jcxz noinp
	xor ax,ax
	xchg al,charin
	or al,al
	jnz rdok
;	poll keyboard buffer, until something is entered
kbin:	mov bx,kbtail
	cmp bx,kbhead
	jz kbin
	mov ax,[bx]
	inc bx
	inc bx
	cmp bx,ADDR: kbbuf+KBSIZ*2
	jnz wrap5
	mov bx,ADDR: kbbuf
wrap5:	mov kbtail,bx
;	again, skip key if ax = 0
	or ax,ax
	jz input
	cmp ax,7200h
	jnz rd1
	mov ax,1910h
;	function key expansion
rd1:	or al,al
	jnz rdok
	mov charin,ah
rdok:
	stosb
	loop input
noinp:	ret
input	endp

output	proc near
	jcxz nooutp
	mov al,es:[di]
	inc di
	push cx
	call wrbyt
	pop cx
	loop output
nooutp:
	ret
output	endp

;	write a byte to standard out
wrbyt	proc near
	cli ;	place byte in internal text buffer
	mov bx,bufhead
	mov cx,buftail
	and al,7fh
	cmp ctrlin,0
	jnz ok4buf
	cmp al,7
	jz ok4buf
	cmp al,13
	jz ok4buf
;	control h removes the last byte in the buffer
	cmp al,8
	jnz noctlh
	cmp bx,cx
	jz bufdone
	call decbptr
	cmp byte ptr [bx],13
	jz bufdone
	mov byte ptr [bx],80h
	mov bufhead,bx
	cmp bx,cx
	jz bufdone
	cmp bx,bufcur
	jnz bufdone
	call decbptr
	mov bufcur,bx
	jmp bufdone
noctlh:	cmp al,' '
	jb bufdone
ok4buf:	mov byte ptr [bx],al
	call incbptr
	mov bufhead,bx
	mov byte ptr [bx],80h
	cmp bx,cx
	jnz bufdone
	call incbptr
	mov buftail,bx
	cmp cx,bufcur
	jnz bufdone
	mov bufcur,bx
bufdone:
	sti

;	video bios stuff stolen from the console device driver in ms-dos
	mov bx,clsptr
	cmp al,[bx]
	jnz wrb1
	inc bx
	cmp bx,ADDR: clsptr
	jb wrb2
;	string esc [ 2 J  matched, clear screen
	mov ax,600h
	xor cx,cx
	mov dx,184fh
	mov bh,7
	int 10h
	mov ah,15
	int 10h
	mov ah,2
	xor dx,dx
	int 10h
	mov al,13
wrb1:	mov bx,ADDR: clscode
wrb2:	mov clsptr,bx
	cmp al,7 ; we do the bell ourselves
	jz noscr
	mov bx,7
	mov ah,14
	int 10h
noscr:

;	make click accompanying output byte
;	assumes all the previous stuff preserves al
	cmp byte ptr xparent,0
;	the "byte ptr" prevents the "phase error between passes"
;	a common problem when variables are defined after they are used.
	jnz nosnds
	cmp al,13 ; return has different sound
	jz crsound
	cmp al,7
	jz bell
	cmp al,' ' ; whitespace?
	ja click
intone:	sti ; jump here if we are in the middle of a tone
;	no sound, delay  equal to a character click
	mov cx,DELAY*2
	loop $
nosnds:	ret

click:	cli ; cannot interrupt a click or a sound with another sound
	mov dx,61h ; keyboard controller
	in al,dx
	test al,1
	jnz intone
	xor al,2
	out dx,al
	mov cx,DELAY
	loop $
	xor al,2
	out dx,al
	sti
	mov cx,DELAY
	loop $
	ret

crsound:  ; the sound of a carriage return
	mov dx,61h ; keyboard controller
	mov bx,350*CLKRATE
rs1:	cli ; again, not interruptable
;	All interrupt protected routines should be kept under 8ms.
	in al,dx
	test al,1
	jnz intone
	xor al,2
	out dx,al
	sti
	mov cx,bx
	shr cx,1 ; divide by 32
	shr cx,1
	shr cx,1
	shr cx,1
	shr cx,1
	loop $
	sub bx,CLKRATE
	cmp bx,104*CLKRATE
	ja rs1
	ret

bell: ; the sound of a control G
	mov bx,ADDR: bellfc
	jmp putfifo ; put bell sound on real time fifo
wrbyt	endp

;	inc bx pointer, around circular buffer
incbptr	proc near
	inc bx
	cmp bx,buftop
	jnz wrap1
	mov bx,bufbot
wrap1:	ret
decbptr:
	cmp bx,bufbot
	jnz wrap2
	mov bx,buftop
wrap2:	dec bx
	ret
incbptr	endp

;	read ioctl values
;	used by a calling process, to set up pronounciation
;	correction tables, etc, based on data in an ascii file.
;	remember, we cannot read a file from device driver init level.
;	just as well, since the user can change the file in mid session,
;	rerun the wordfix program, and modify the way words
;	are pronounced.
;	this ioctl call simply returns the location of
;	various tables used by the driver.
;	the calling program writes data into these tables directly,
;	as determined by the contents of the ascii file.
;	yes, there are some race conditions to deal with.
ioctl	proc near
;	returns the location of the punctuation pronounciation table,
;	the location of the word correction table,
;	the location of the internal text buffer,
;	the location of the key/function map,
;	and several sizes an llimits
;	thus only 24-byte ioctl reads are allowed
;	calling process must understand this driver implicitly
	cmp cx,24
	jz ioctlok
	les di,rhptr
	sub es:[di+18],cx ; 0 bytes transfered
	mov es:[di+3],810bh ; read fault, cannot return that many bytes
	ret
ioctlok:
	mov word ptr es:[di],ADDR:punctab
	mov es:[di+2],ds
	mov word ptr es:[di+4],ADDR: UDATA
	mov es:[di+6],ds
	mov es:[di+2],ds
	mov word ptr es:[di+8],ADDR: UDATA + WDFIXLEN
	mov es:[di+10],ds
	mov word ptr es:[di+12],ADDR: keymap
	mov es:[di+14],ds
	mov es:[di+16],WDFIXLEN ; length of word correction table
	mov word ptr es:[di+18],WDLEN ; longest possible word
	mov ax,buftop
	sub ax,bufbot
	mov es:[di+20],ax ; size of internal text buffer
	mov ax,buftail
	sub ax,bufbot
	mov es:[di+22],ax ; start of text in the buffer
	ret
ioctl	endp


;	interrupt routines

;	read the virtual screen, in real time
;	called at real time interrupt level
;	Since this mess could easily take more than 50ms,
;	and I don't feel like counting instructions for all possible paths,
;	we call the original real time interrupt right away,
;	then we interpret keyboard commands, read text, etc.
ih_rti	proc far ; user real time interrupt handler
	pushf ; call the original interrupt routine
;	call far ptr cs:[orig_rti]
	db 2eh, 0ffh, 01eh
	dw orig_rti

	cld
	push ds
	push cs
	pop ds
	push ax
	push bx
	push cx
	push dx
	push si
	push di

	call chkfifo ; sound fifo

;	check for nested real time interrupts
	dec nestrti
	jnz endrti ; previous rti routine still reading, or whatever

	cmp xparent,0
	jnz endrti

;	switch stacks and enable interrupts
	mov save_ss,ss
	mov save_sp,sp
;	New stack overlays initialization code, about 200 bytes.
	push cs
	pop ss
	mov sp,ADDR: inflush
	sti

;	no reading or key commands until the synthesizer is ready
	call ss_ready
	jz outrti

	call keycmd ; execute talking command in talkcmd

	call reading ; continuous reading

outrti:	mov ss,save_ss ; restore old stack
	mov sp,save_sp

endrti:	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	inc nestrti
	pop ds
	iret
ih_rti	endp


ih_kbi	proc far ; keyboard interrupt handler
	pushf ; call the original hardware interrupt routine
;	call far ptr cs:[orig_kbi]
	db 2eh, 0ffh, 01eh
	dw orig_kbi
;	if a key was entered into the MS-DOS buffer,
;	and the key represents a reading command,
;	remove it from the buffer, and place the corresponding
;	command into the talkcmd location
;	we assume no real time (or other) interrupt routine will
;	grab the key from the buffer before this routine can snag it.
	cld

	push ds
	push cs
	pop ds
	push ax
	push bx
	push si

;	call original bios routine to get the entered character
	mov ah,1
	pushf
;	call far ptr [orig_bkb]
	db 0ffh, 01eh
	dw orig_bkb
	jz nokey ; no new characters typed
;	now remove character
	mov ah,0
	pushf
;	call far ptr [orig_bkb]
	db 0ffh, 01eh
	dw orig_bkb

	cmp ax,5200h ; toggle transparent mode
	jnz noxp
;	only release from transparent mode here
	cmp xparent,0
	jz noxp
	mov xparent,0
	jnz nokey
noxp:	cmp xparent,0
	jnz inkey
	cmp qby,0
	jz noesc
	dec qby
	jz inkey
noesc:	call transkey ; turn key into command code
	jc nokey ; a talking command
inkey:	cli
	mov bx,kbhead
	mov [bx],ax
	inc bx
	inc bx
	cmp bx,ADDR: kbbuf+KBSIZ*2
	jnz wrap3
	mov bx,ADDR: kbbuf
wrap3:	cmp bx,kbtail
	jnz kbroom
	sti
	call bell
	jmp short nokey
kbroom:	mov kbhead,bx
nokey:	pop si
	pop bx
	pop ax
	pop ds
	iret
ih_kbi	endp

ih_ctrlc	proc far  ;  put control C  into charin
	mov cs:charin,3  ;  overwrite
	iret
ih_ctrlc	endp

;	new BIOS keyboarde routine, to accommodate new type ahead buffer
ih_bkb	proc far
	push ds
	push cs
	pop ds
	push bx
	mov bx,kbtail
	cmp ah,1
	jb getc
	jz nbget
	pop bx
	pop ds
;	jmp far ptr cs:[orig_bkb]
	db 2eh, 0ffh, 02eh
	dw orig_bkb
;	non blocking getchar
nbget:	cmp bx,kbhead
	mov ax,[bx]
	pop bx
	pop ds
	ret 2
getc:	sti
	cmp bx,kbhead
	mov ax,[bx]
	jz getc
	inc bx
	inc bx
	cmp bx,ADDR: kbbuf+KBSIZ*2
	jnz wrap6
	mov bx,ADDR: kbbuf
wrap6:	mov kbtail,bx
kbdone:	pop bx
	pop ds
	iret
ih_bkb	endp

;	put character to screen
;	original routine (in con) destroys cx and dx
;	we do the same.  hope this is ok
ih_putc	proc far
	cld
	push ds
	push ax
	push bx
;	push cx
;	push dx
	push cs
	pop ds
	push si
	sti
	call wrbyt
	pop si
;	pop dx
;	pop cx
	pop bx
	pop ax
	pop ds
	iret
ih_putc	endp

PROG    ends


DATA    segment para public 'DATA'

extrn bellfc:byte
	extrn punctab:byte, keymap:byte
	public bufhead, buftail, bufcur, buftop, bufbot
	public talkcmd
	public ctrlin, xparent, qby

;	save ss and sp when switching to real time interrupt stack
save_ss	dw 0
save_sp	dw 0

;	functions for device driver commands
functab label word
	DW  init
	dw nulf
	dw nulf
	DW  ioctl
	DW  input
	DW  ndinput
	DW  ndinput ; get input status, same routine as nd input
	DW  inflush
	DW  output
	DW  output
	DW  outstat
	DW  outflush

;	circular buffer variables
;	all output text is stored in this circular buffer
bufhead	dw 0
buftail	dw 0
bufcur	dw 0
bufsiz	dw 0
bufbot	dw 0 ; buffer limits
buftop	dw 0

;	we implement our own interrupt level keyboard buffer
;	the ms-dos buffer (15 characters) is frustratingly small
kbbuf	dw KBSIZ dup(?)
kbhead	dw ADDR: kbbuf
kbtail	dw ADDR: kbbuf

;	check for esc [ 2 J  when writing bytes
clscode	db 27,"[2J"
clsptr	dw ADDR: clscode

qby	db 0 ; is bypass enabled
xparent	db 0 ; transparent mode, no sounds or reading
talkcmd	db 0 ; command from keyboard affecting speech
ctrlin	db 0 ; are control characters placed in the buffer?
nestrti	db 1 ; check for nested real time interrupts

DATA    ends

UDATA   segment byte public 'UDATA' ; for finding end of data segment
UDATA   ends

	end hdr

-- 
	You know  ...  if it ain't patina, it's verdigris.
	Karl Dahlke   ihnp4!ihnet!eklhad
