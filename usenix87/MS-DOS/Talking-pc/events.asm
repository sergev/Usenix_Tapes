
;------------------------------------------------------------------------------
;	Talking console device driver for the AT&T PC6300.
;	Written by Karl Dahlke, September 1986.
;	Property of AT&T Bell Laboratories, all rights reserved.
;	This software is in the public domain and may be freely
;	distributed to anyone, provided this notice is included.
;	It may not, in whole or in part, be incorporated in any commercial
;	product without AT&T's explicit permission.
;------------------------------------------------------------------------------

;	events.asm: handle events at real time and keyboard interrupt level

	include parms.h

PGROUP	group	PROG, DATA
	assume cs:PGROUP, ds:PGROUP

DATA	segment	word public 'DATA'

	extrn bufhead:word, buftail:word, bufcur:word
	extrn rdflag:byte, inspeech:byte, onesymb:byte
	extrn rdlines:byte, ctrlin:byte, xparent:byte
	extrn talkcmd:byte, qby:byte
	public punctab, keymap
	public wdbuf
	public bellfc, buzzfc

;	fifo containing events to be done at real time interrupt level
fifobot	db FIFOLEN dup(?)
fifotop	label byte
fifohead dw ADDR: fifobot
fifotail dw ADDR: fifobot
;	lengths of commands in fifo
fclens	db 0,1,3,1,2

;	codes for real time fifo commands
bellfc	db 2,150,2,1,3,0 ; sound of control G
boundfc	db 2,1,2,2,190,1,2,120,1,2,60,1,3,0 ; code for error sound
tonefc	db 2,1,8,1,1,3,0
buzzfc	db 2,1,35,1,1,3,0

wflflag	db 0 ; use word instead of letter
announc	db 0 ; announce the function of the next key

wdbuf	db 2*WDLEN dup(?) ; word buffer
ctrlstr	db "controal x",0

;	extracted command, from the fifo
fcout	db 0
fcpar1	db 0
fcpar2	db 0

;	punctuation pronounciation table
;	set to default values
;	at most ten letters per symbol.
;	it turns out, you don't want long names for these anyways.
;	it slows you down, reducing productivity
punctab	label byte
	db "null~~~~~~"
	db "escape~~~~"
	db "askie 1c~~"
	db "askie 1d~~"
	db "askie 1e~~"
	db "askie 1f~~"
	db "space~~~~~"
	db "bang~~~~~~"
	db "quoat~~~~~"
	db "pound~~~~~"
	db "doller~~~~"
	db "percent~~~"
	db "and~~~~~~~"
	db "single~~~~"
	db "left paren"
	db "rite paren"
	db"star~~~~~~"
	db"plus~~~~~~"
	db"comma~~~~~"
	db"mighnus~~~"
	db"periud~~~~"
	db"slash~~~~~"
	db"colen~~~~~"
	db"semmy~~~~~"
	db"less~~~~~~"
	db"eequal~~~~"
	db"greater~~~"
	db"question~~"
	db"at sign~~~"
	db "left b~~~~"
	db "backslash~"
	db "rite b~~~~"
	db "up airow~~"
	db "underline~"
	db "backquoat~"
	db "left brace"
	db "vertical~~"
	db "rite brace"
	db "tillda~~~~"
	db "deleet~~~~"

;	phonetic alphabet, words for letters to avoid ambiguity
;	important when you need to know exactly which letter  (e.g. m or n)
;	such as variables for equations or programs
;	words taken from the NATO standard, established in the 1960's
wfl	label byte
;	10 bytes per entry
	db "al fa~~~~~brohvo~~~~charlie~~~"
	db "delta~~~~~eko~~~~~~~foxtrot~~~"
	db "gawlf~~~~~hoatel~~~~india~~~~~"
	db "juleyet~~~killo~~~~~liema~~~~~"
	db "mike~~~~~~noavember~oscar~~~~~"
	db "popa~~~~~~kebeck~~~~roamio~~~~"
	db "seeara~~~~tango~~~~~uniform~~~"
	db "victor~~~~wiskey~~~~x ray~~~~~"
	db "yangkey~~~zoolu~~~~~"

;	map keys into talking functions
keymap	label byte
	db 0, 0, 22, 21, 8, 20, 6, 0
	db 0, 0, 19, 18, 17, 0, 14, 11
	db 23, 5, 16, 7, 24, 0, 10,15
	db 0, 0, 0, 0, 0, 0, 0, 0
	db 0, 0, 0, 0, 0, 0, 0, 0
	db 0, 0, 0, 0, 0, 0, 0, 0
	db 0, 0, 0, 0, 0, 0, 0, 0
	db 0, 0, 0, 9, 12, 10, 11, 2
	db 13, 0, 14, 3, 1, 0, 0, 0
	db 0, 0, 0, 0, 0, 0, 0, 0
	db 0, 0, 4, 0, 0, 0, 0, 0
	db 0, 0, 0, 0, 0, 0, 0, 0

;	table of description strings
fnames	label word
	dw ADDR: fd1, ADDR: fd2, ADDR: fd3, ADDR: fd4
	dw ADDR: fd5, ADDR: fd6, ADDR: fd7, ADDR: fd8
	dw ADDR: fd9, ADDR: fd10, ADDR: fd11, ADDR: fd12
	dw ADDR: fd13, ADDR: fd14, ADDR: fd15, ADDR: fd16
	dw ADDR: fd17, ADDR: fd18, ADDR: fd19, ADDR: fd20
	dw ADDR: fd21, ADDR: fd22, ADDR: fd23, ADDR: fd24

fd1	db "reed one line at a time",0
fd2	db "clear the buffer",0
fd3	db "retain controal karecters",0
fd4	db "transparent mode",0
fd5	db "pass next karecter threw",0
fd6	db "reed the next karecter",0
fd7	db "reed the preivious karecter",0
fd8	db "reed the current karecter",0
fd9	db "start of buffer",0
fd10	db "end of buffer",0
fd11	db "reed the current line",0
fd12	db "reed the preivious line",0
fd13	db "reed the next line",0
fd14	db "reed the line after next",0
fd15	db "current cohllumm number",0
fd16	db "upper or lower case",0
fd17	db "reed the next word",0
fd18	db "reed the current word",0
fd19	db "reed the preivious word",0
fd20	db "upp one row",0
fd21	db "down one row",0
fd22	db "reed the last cohmpleit line",0
fd23	db "announce the function of the next key entered",0
fd24	db "reed the current karecter as a word",0

DATA	ends


PROG	segment	byte public 'PROG'

	extrn ss_text:near, ss_shutup:near
	extrn crsound:near, click:near, bell:near
	extrn incbptr:near, decbptr:near
	public keycmd, transkey
	public putfifo, chkfifo
	public curchar

;	turn keyboard entry into talking command
transkey proc near
	mov si,ax ; save
	cmp al,26
	ja nofunc ; not a function or alt key
	cmp al,0
	jnz noalts
	mov al,ah
	cmp al,3bh
	jb nofunc
	cmp al,5fh
	ja nofunc
noalts:	mov bl,al
	mov bh,0
	mov al,keymap[bx]
	or al,al
	jz nofunc
	mov talkcmd,al
	stc
	ret
nofunc:	mov ax,si
	clc
	ret
transkey endp

keycmd	proc near ; execute keyboard command
	cli
	mov al,talkcmd
	or al,al
	jnz cmdin
	sti
	ret ; no command
cmdin:
;	talking keyboard comand, check for interrupted speech
	cmp inspeech,0
	jz nobrk
	call ss_shutup
	dec inspeech
	mov rdflag,0 ; stop reading
	mov onesymb,0
	ret; enough real time has been taken up already
nobrk:

;	execute keyboard command
	mov talkcmd,0
	mov bl,al
	mov bh,0
	call click ; makes click and enables interrupts


;	if announce flag set, simply describe the function
	cmp announc,0
	jz regular
	dec bx
	add bx,bx
	mov bx,fnames[bx] ; pointer to description
	mov announc,0
	jmp spksym

regular:
	dec bx
	jnz nt1
;	toggle reading mode, one line, or to the end
	xor rdlines,1
	jz notone
mktone:	mov bx,ADDR: tonefc
	jmp putfifo

nt1:	dec bx
	jnz nt2
;	clear buffer
	mov ax,buftail
	mov bufhead,ax
	mov bufcur,ax
	xchg ax,bx
	mov byte ptr [bx],80h
	jz mktone

nt2:	dec bx
	jnz nt3
;	toggle mode for retaining control characters in the buffer
	xor ctrlin,1
	jnz mktone
notone:	ret

nt3:	dec bx
	jnz nt4
;	toggle transparent mode, only set here
	inc xparent
	jnz mktone

nt4:	dec bx
	jnz nt5
;	enable bypass
	inc qby
	ret

nt5:
;	the remaining comands are meaningless when the buffer is empty
;	check for this here
	mov si,bufhead
	cmp si,buftail
	jz boundsnd ; empty buffer

	dec bx
	jnz nt6
;	speak next character
	mov bx,bufcur
	call incbptr
	cmp bx,bufhead
	jz boundsnd
	mov bufcur,bx
	jmp curchar

nt6:	dec bx
	jnz nt7
;	speak previous character
	mov bx,bufcur
	cmp bx,buftail
	jz boundsnd ; top of buffer
	call decbptr
	mov bufcur,bx
	jmp curchar

; error, boundary condition, off the end of the buffer, or empty buffer
boundsnd: 
	mov bx,ADDR: boundfc
	jmp putfifo

nt7:	dec bx
	jz curchar
	jmp nt8
;	speak current character
curchar:
	mov bx,bufcur
	mov al,[bx]
	mov ah,0
	or al,al
	mov bx,ADDR: punctab
	jnz notnull
mvstr: ; move string to speaking buffer.  10 characters, or '~' terminated
	mov cx,10
	mov di,ADDR: wdbuf
str1:	mov al,[bx]
	cmp al,'~'
	jz str2
	mov [di],al
	inc bx
	inc di
	loop str1
str2:	mov byte ptr [di],0
	mov bx,ADDR: wdbuf
	jmp spksym

notnull:
	cmp al,7
	jnz nobell
	mov wflflag,0
	jmp bell
nobell:
	cmp al,13
	jnz notcr
	mov wflflag,0
	jmp crsound
notcr:
	cmp al,27
	jae notctrl
	mov bx,ADDR: ctrlstr
	or al,40h
	mov [bx+9],al
spksym:	mov wflflag,0
	jmp ss_text

notctrl:
	mov dl,10
	mov cx,ax ; save char
	sub al,26
	mul dl
	add bx,ax
	xchg ax,cx
	cmp al,'0'
	jb mvstr
	cmp al,'9'
	jbe letter
	sub bx,10*10
	cmp al,'A'
	jb mvstr
	cmp al,'Z'
	jbe letter
	sub bx,26*10
	cmp al,'a'
	jb mvstr
	cmp al,'z'
	jbe letter
	sub bx,26*10
	jmp mvstr

letter:
	mov bx,ADDR: wdbuf
	or al,20h
	mov [bx],al
	mov byte ptr [bx+1],0
	cmp wflflag,0
	jz spksym
	cmp al,'9'
	jbe spksym
	sub al,'a'
	mov cl,10
	mul cl
	xchg ax,bx
	add bx,ADDR: wfl
	jmp mvstr

nt8:	dec bx
	jnz nt9
;	move cursor to top of buffer
	mov bx,buftail
	mov bufcur,bx
	ret

nt9:	dec bx
	jnz nt10
;	mov cursor to bottom of buffer
	mov bx,bufhead
	call decbptr
	mov bufcur,bx
	ret

nt10:	dec bx
	jnz nt11
;	start reading at current line
	mov bx,bufcur
rdstart:
	call backnl
	mov bufcur,bx
	mov rdflag,1
	mov inspeech,1
	ret

nt11:	dec bx
	jnz nt12
;	read previous line
	mov bx,bufcur
	call backnl
	jz bnd1snd
	call decbptr
	jmp rdstart

nt12:	dec bx
	jnz nt13
;	read next line
	mov bx,bufcur
	call nextnl
	jz bnd1snd
	jmp rdstart

nt13:	dec bx
	jnz nt14
;	read line after next
	mov bx,bufcur
	call nextnl
	jz bnd1snd
	call nextnl
	jnz rdstart
bnd1snd:
	jmp boundsnd

nt14:	dec bx
	jnz nt15
;	read column number
	mov bx,bufcur
	call backnl
	xchg ax,cx
	mov si,ADDR: wdbuf + 4
	mov byte ptr [si+1],0
	mov di,si
	mov cx,5
	mov bx,10
cn1:	xor dx,dx
	div bx
	add dl,'0'
	mov [si],dl
	cmp dl,'0'
	jz insig
	mov di,si
insig:	dec si
	loop cn1
	mov cx,6
cn2:	inc si
	mov al,[di]
	inc di
	mov [si],al
	loop cn2
	mov bx,ADDR: wdbuf
	jmp spksym

nt15:	dec bx
	jnz nt16
;	indicate case
	mov bx,bufcur
	mov al,[bx]
	mov ah,al
	or al,20h
	cmp al,'a'
	jb caserr
	cmp al,'z'
	jbe caseok
caserr:	jmp bell
caseok:	and ah,20h
	jnz lower
	jmp mktone
lower:	ret

nt16:dec bx
	jnz nt17
;	speak next symbol
	mov bx,bufcur
	call nextsym
l161:	call incbptr
	mov al,[bx]
	cmp al,80h
bnd2snd: jz bnd1snd
	cmp al,' '
	jz l161
onsymb:	call backsym
	mov bufcur,bx
	inc rdflag
	inc onesymb
	ret

nt17:	dec bx
	jnz nt18
;	speak current symbol
	mov bx,bufcur
	cmp byte ptr [bx],' '
	jz col1
	jmp onsymb

nt18:	dec bx
	jnz nt19
;	speak previous symbol
	mov bx,bufcur
	call backsym
l181:	cmp bx,buftail
	jz bnd2snd
	call decbptr
	cmp byte ptr [bx],' '
	jz l181
	jmp onsymb

nt19:	dec bx
	jnz nt20
;	up a row
	mov bx,bufcur
	call backnl
	jz bnd2snd
	xchg ax,cx
	call decbptr
	call backnl
	xchg ax,cx
	dec cx
	jcxz col1
advc:	cmp byte ptr [bx],13
	jnz notpast
pastnl:	jmp bell
notpast:
	call incbptr
	loop advc
col1:	mov bufcur,bx
	jmp curchar

nt20:	dec bx
	jnz nt21
;	down a roe
	mov bx,bufcur
	call backnl
	xchg ax,cx
	call nextnl
	jz bnd2snd
	xchg ax,cx
	dec cx
	jcxz col1
advc1:	cmp byte ptr [bx],13
	jz pastnl
	call incbptr
	cmp byte ptr [bx],80h
	jz pastnl
	loop advc1
	jmp short col1

nt21:	dec bx
	jnz nt22
;	read last non-trivial line
	mov bx,bufhead
	call backnl
	jz lastln
ll1:	call decbptr
	call backnl
	jz lastln
	dec cx
	jz ll1
lastln: jmp rdstart

nt22:	dec bx
	jnz nt23
;	announce function of next key entered
	inc announc
	ret

nt23:	dec bx
	jnz nt24
;	speak word for the current character
	inc wflflag
	jmp curchar

nt24:
;	default
	ret
keycmd	endp

backnl	proc near
;	back up bx until new line is reached
	mov cx,1
bkn1:	cmp bx,buftail
	jz nlbnd
	call decbptr
	inc cx
	cmp byte ptr [bx],13
	jnz bkn1
	call incbptr
	dec cx
nlbnd:	ret
nextnl:
;	advance bx until newline is reached
	cmp byte ptr [bx],13
	jz nl1
	cmp byte ptr [bx],80h
	jz nlbnd
	call incbptr
	jmp nextnl
nl1:	call incbptr
	cmp byte ptr [bx],80h
	ret
backnl	endp

backsym	proc near
;	back up bx to the beginning of the current symbol
	mov al,[bx]
	cmp al,'0'
	jb bks
	or al,20h
	cmp al,'z'
	ja bks
	cmp al,'a'
	jae ws1
	cmp al,'9'
	ja bks
ws1:	cmp bx,buftail
	jz bks
	call decbptr
	mov al,[bx]
	cmp al,'0'
	jb ws3
	or al,20h
	cmp al,'z'
	ja ws3
	cmp al,'9'
	jbe ws1
	cmp al,'a'
	jae ws1
ws3:	call incbptr
bks:	ret
;	advance bx to the end of the current symbol
nextsym:
	mov al,[bx]
	cmp al,'0'
	jb bks
	or al,20h
	cmp al,'z'
	ja bks
	cmp al,'a'
	jae wm1
	cmp al,'9'
	ja bks
wm1:	call incbptr
	mov al,[bx]
	cmp al,'0'
	jb wm3
	or al,20h
	js wm3
	cmp al,'z'
	ja wm3
	cmp al,'9'
	jbe wm1
	cmp al,'a'
	jae wm1
wm3:	call decbptr
	ret
backsym	endp

chkfifo proc near ;	get event from fifo
;	runs with interrupts disabled
	mov si,fifotail
	cmp si,fifohead
	jz noev ; no events present
	mov bl,[si]
	mov bh,0
	mov cl,[bx+fclens]
	mov ch,0
	mov di,ADDR: fcout
evget:	mov al,[si]
	mov [di],al
	inc di
	inc si
	cmp si,ADDR: fifotop
	jnz wrap2
	mov si,ADDR: fifobot
wrap2:	loop evget
	mov fifotail,si

;	interpret command
	mov bl,fcout
	dec bx
	jnz not1
;	no op, delay one time interval
noev:	ret

not1:	dec bx
	jnz not2
;	start speaker singing
	mov al,0b6h ; set timer 2
	out 43h,al
	mov al,fcpar1
	nop ; nops for timing
	nop
	out	42h,al
	mov al,fcpar2
	nop
	nop
	out	42h,al
	mov al,73h
	out 61h,al
	ret

not2:	dec bx
	jnz not3
;	stop tone
	mov al,70h
	out 61h,al

;	default
not3:	ret
chkfifo	endp

;	put events onto the real time fifo, if there is room
;	if not, into the old bit bucket
putfifo	proc near
	cli
	mov dx,fifohead ; save value
	mov si,dx
put1:
	mov al,[bx]
	inc bx
	or al,al
	jz putok
	mov [si],al
	inc si
	cmp si,ADDR: fifotop
	jnz wrap1
	mov si,ADDR: fifobot
wrap1:	cmp si,fifotail
	jnz put1
;	buffer is full
	mov si,dx ; restore original head pointer
putok:	mov fifohead,si
	sti
	ret
putfifo	endp

PROG	ends

	end

