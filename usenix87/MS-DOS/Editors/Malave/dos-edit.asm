;	DOS - EDIT.ASM -- Resident DOS Command Line Editor
;	==================================================
;	Taken From PC Magazine on 1-20-86, by Kent Malave'

cseg		segment
		assume	cs:cseg

		org	0080h
keyboardbuffer 	label	Byte

		org	0100h
entry:		jmp	initialize

;	All Data
;	--------
		db	"(C) Copyright 1985 Ziff-Davis Publishing Co."
oldinterrupt21	dd	?		;Original Interrupt 21 vector
oldinterrupt16	dd	?		;Original Interrupt 16 vector
doingbuffkey	db	0		;Flag for doing Function call 0ah
bufferpointer	dw	keyboardbuffer  ;Pointer to Keyboard Buffer
buffercounter	db	0		;Number of Char in buffer
maxcharcol	db	?		;maximum char column on screen
originalcursor	dw	?		;Place to save cursor on full screen
inserton	db	0		;insert mode flag	

keyroutine	dw	Home,Up,PgUp,Dummy,Left,Dummy,Right
		dw	Dummy,End,Down,PgDn,Insert,Delete

;	New Interrupt 21 (Dos Function Calls)
;	-------------------------------------

newinterrupt21 	Proc	Far
		mov	cs:[doingbuffkey],0	;Turn flag off initially
		cmp	ah,0ah			;Check if doing buffered input
		jz	bufferedinput
		jmp	cs:[oldinterrupt21]	;if not do regular interrupt
bufferedinput:	mov	cs:[doingbuffkey],-1 	;If so, turn on flag
		pushf				;simulate regular interrupt
		call	cs:[oldinterrupt21]
		mov	cs:[doingbuffkey],0	;Turn flag off
		mov	cs:[buffercounter],0	;Re-set character counter
		Iret				;return to user program
newinterrupt21  EndP

;	New Interrupt 16 (BIOS Keyboard Routine)
;	----------------------------------------

newinterrupt16	Proc	Far
		sti				;Re-enable interrupts
		cmp	cs:[doingbuffkey],0	;Check if doing call 0ah
		jz	donotintercept		;If not, do old interrupt
		cmp	cs:[buffercounter],0	;Check if chars in buffer
		jnz	substitute		;If so, get them out
		cmp	ah,0			;See if doing a get key
		jz	checkthekey		;if so, get the key
donotintercept: jmp	cs:[oldinterrupt16]	;otherwise, do the old interrupt


checkthekey:	pushf				;Save flags
		call	cs:[oldinterrupt16]	;do regular interrupt
		cmp	ax,4800h		;Check if up cursor
		jnz	nottriggerkey		;if not, don't bother
		call	fullscreen		;move around the screen
		cmp	cs:[buffercounter],0	;Any chars to deliver?
		jz	checkthekey		;if not, get another key
returnbuffer:	call	getbufferchar		;otherwise, pull one out
		inc	cs:[bufferpointer]	;Kick up the pointer
		dec	cs:[buffercounter]	;and knock down the counter
nottriggerkey:	IRet				;and go back to user pgm.

;	Substitute Key from Buffer
;	--------------------------

substitute:	cmp	ah,2			;See if shift status check
		jae	donotintercept		;if so, can't be bothered
		cmp	ah,0			;see if get a key
		jz	returnbuffer		;if so, get the key above
		call 	getbufferchar		;otherwise, get a key
		cmp	cs:[buffercounter],0	;and clear zero flag
		Ret	2			;Return with existing flags
newinterrupt16	EndP

;	Get Buffer Characters
;	---------------------

getbufferchar:	push	bx
		mov	bx,cs:[bufferpointer]	;get pointer to key buffer
		mov	al,cs:[bx]		;get the key
		sub	ah,ah			;blank out scan code
		pop	bx
		ret

;	Full Screen Routine
;	-------------------

fullscreen:	push	ax			;save all these registers
		push	bx
		push	cx
		push	dx
		push	di
		push 	ds
		push	es
		mov	ax,cs			;set AX to this segment
		mov	ds,ax			;do DS is this segment
		mov	es,ax			;and ES is also

		assume	ds:cseg, es:cseg	;Tell the assembler
		mov	ah,0fh			;Get video state
		int	10h			;   through BIOS
		dec	ah			;Number of columns on screen
		mov	[maxcharcol],ah		;Save maximum column
						;BH = Page Number throughout
		mov	ah,03h			;Get cursor in DX
		int	10h			;   through BIOS
		mov	[originalcursor],dx	;and save the current position
		call	Up			;move cursor up

mainloop:	cmp	dh,Byte Ptr [originalcursor + 1]	;If at line
		jz	termfullscreen		;stated from, terminate
		mov	ah,02h			;Set cursor from DX
		int	10h			;   through BIOS
getkeyboard:	mov	ah,0			;Get the next key
		pushf				;by simulating interrupt 16h
		call	cs:[oldinterrupt16]	;  which goes to BIOS
		cmp	al,1bh			;see if escape key
		jz	termfullscreen		;if so terminate full screen

;	Back Space
;	----------

		cmp	al,08h			;See if back space
		jnz	notbackspace		;if not, continue test
		or	dl,dl			;check if cursor is at left
		jz	mainloop		;If so do nothing
		dec	dl			;Otherwise, move cursor back
		call	shiftleft		;and shift line to the left
		jmp	mainloop		;and continue for next key

;	Carriage Return
;	---------------

notbackspace:	cmp	al,0dh			;See if carriage return
		jnz	notcarrret		;If not, continue test
		call	End			;move line into buffer
		mov	al,0dh			;Tack on Carriage return
		stosb				;by writing to buffer
		inc	[buffercounter]		;one more character in buffer
		jmp	mainloop		;and continue

;	Normal	Character
;	-----------------

notcarrret:	cmp	al,' '			;See if normal character
		jb	notnormalchar		;if not, continue test
		cmp	[inserton],0		;check for insert mode
		jz	overwrite		;if not, overwrite
		call 	shiftright		;shift line right for insert
		jmp	Short normalcharend	;and get ready to print
overwrite:	mov	cx,1			;write one character
		mov	ah,0ah			;by calling BIOS
		int	10h			;using interrupt 10h
normalcharend:	call	Right			;Cursor to right and print
		jmp	mainloop		;Back for another key

;	Cursor Key, Insert, or Delete Subroutine
;	----------------------------------------

notnormalchar:	xchg	al,ah			;Put extended code in AL
		sub	ax,71			;see if it's a cursor key
		jc	getkeyboard		;if not, no good
		cmp	ax,12			;Another check for cursor
		ja	getkeyboard		;if not, skip it
		add	ax,ax			;Double for index
		mov	di,ax			;  into vector table
		call	[keyroutine + di]	;do the routine
		jmp	mainloop		;back for another key

;	Terminate Full Screen Movement
;	------------------------------

termfullscreen:	mov	dx,[originalcursor]	;Set cursor to original
		mov	ah,2			;and set it
		int	10h			;   through BIOS
		pop	es			;Restore all registers
		pop	ds
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret				;Return to new interrupt 16h

;	Cursor Movement
;	---------------

Home:		mov	dl,Byte Ptr [originalcursor] ;move cursor to 
		ret				; original column

Up:		or	dh,dh			;check if at top row
		jz	UpEnd			;If so, do nothing
		dec	dh			;If not, decrement row
UpEnd:		ret

PgUp:		sub	dl,dl			;move cursor to far left
		ret

Left:		or	dl,dl			;Check if cursor at far left
		jnz	gowest			;If not, move it left
		mov	dl,[maxcharcol]		;move cursor to the right
		jmp	Up			;and go up one line
gowest:		dec	dl			;Otherwise, decrement column
		ret

Right:		cmp	dl,[maxcharcol]		;Check if cursor is far right
		jb	goeast			;If not, move it right
		sub	dl,dl			;Set cursor to left of screen
		jmp	Down			;and go down a line
goeast:		inc	dl			;Otherwise, increment column
		ret

End:		call	transferline		;move line to buffer
		mov	dx,[originalcursor]	;set cursor to original
		ret

Down:		inc	dh			;move cursor down one row
		ret

PgDn:		mov	cl,[maxcharcol]		;Get last column on screen
		inc	cl			;Kick it up by one
		sub	cl,dl			;subtract current column
		sub	ch,ch			;set top byte to zero
		mov	al,' '			;character to write
		mov	ah,0ah			;write blanks to screen
		int	10h			;   throught BIOS
Dummy:		ret

;	Insert and Delete
;	-----------------

Insert:		xor	[inserton],-1		;Toggle the inserton flag
		ret				; and return

Delete:		call	shiftleft		;shift cursor line left
		ret				; and return

;	Transfer Line on Screen to Keyboard Buffer
;	------------------------------------------

transferline:	sub	cx,cx			;count characters in line
		mov	di,Offset keyboardbuffer	;Place to store'em
		mov	[bufferpointer],di	;save that address
		cld				;string direction forward

getcharloop:	mov	ah,02h			;set cursor at DX
		int	10h			;   through BIOS
		mov	ah,08h			;Read character and attribute
		int	10h			;   through BIOS
		stosb				;save the character
		inc	cx			;increment the counter
		inc	dl			;increment the cursor column
		cmp	dl,[maxcharcol]		;see if at end of line yet
		jbe	getcharloop		;if not, continue

		dec	di			;Points to end of string
		mov	al,' '			;character to search through
		std				;searching backwards
		repz	scasb			;search for first non-blank
		cld				;forward direction again
		jz	setbuffercount		;if all blanks, skip down
		inc	cl			;number of non-blanks
		inc	di			;at last character
setbuffercount:	inc	di			;after last character
		mov	[buffercounter],cl	;save the character count
		ret				;return from routine

;	Shift line one Space Right (For Insert)
;	---------------------------------------

shiftright:	push	dx			;Save original cursor
		mov	di,ax			;character insert
shiftrightloop: call	readandwrite		;read character and write
		inc	dl			;kick up cursor column
		cmp	dl,[maxcharcol]		;check if it's rightmost
		jbe	shiftrightloop		;if not, keep going
		pop	dx			;get back original cursor
		ret

;	Shift line one Space Left (For Delete)
;	--------------------------------------

shiftleft:	mov	di,0020h		;blank at end
		mov	bl,dl			;save cursor column
		mov	dl,[maxcharcol]		;set cursor to end of line
shiftleftloop:	call	readandwrite		;read character and write
		dec	dl			;kick down cursor column
		cmp	dl,bl			;see if at original yet
		jge	shiftleftloop		;if still higher, keep going
		inc	dl			;Put cursor back to original
		ret				;return to routine

;	Read and Write Character for Line Shift
;	---------------------------------------

readandwrite:	mov	ah,2			;set cursor from DX
		int	10h			;    through BIOS
		mov	ah,08h			;read character and attribute
		int	10h			;    through BIOS
		xchg	ax,di			;switch with previous char
		mov	cx,1			;one character to write
		mov	ah,0ah			;write character only
		int	10h			;    through BIOS
		ret				;return from routine

;	Initialization on Entry
;	-----------------------

initialize:	sub	ax,ax			;make AX equal zero
		mov	ds,ax			;to point to vector segment
		les	bx,dword ptr ds:[21h * 4] ;get and save int. 21h
		mov	Word Ptr cs:[oldinterrupt21],bx 
		mov	Word Ptr cs:[oldinterrupt21 + 2],es

		les	bx,dword ptr ds:[16h * 4] ;get and save int. 16h
		mov	Word Ptr cs:[oldinterrupt16],bx
		mov	Word Ptr cs:[oldinterrupt16 + 2],es

		push	cs			;restore DS register
		pop	ds			; by setting to CS

		mov	dx,Offset newinterrupt21
		mov 	ax,2521h		;set new interrupt 21h
		int	21h			; through DOS

		mov	dx,Offset newinterrupt16
		mov 	ax,2516h		;set new interrupt 16h
		int	21h			; through DOS

		mov	dx,Offset initialize	;number of bytes to stay
		int	27h			;Terminate and remain resident

cseg		EndS
		End	Entry
