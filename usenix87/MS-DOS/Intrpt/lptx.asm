title LPTx : Line Printer Output Capture Routine
page	66,132
;-------------------------------------------------------------
;
;	MAIN PROGRAM	Version 3.0
;
;  (C)	Copyright 1985 by Mark DiVecchio, All Rights Reserved
;
; You may use and freely distribute this program for
; non-commercial applications.
;
; Mark C. DiVecchio
; 9067 Hillery Drive
; San Diego, CA 92126
; 619-566-6810
;-------------------------------------------------------------
; This program intercepts the BIOS interrupt 17, the line printer
; interrupt. It will redirect the output of LPT1, LPT2, or LPT3 to a disk
; file. All three redirects may be active at the same time.
;
; This version (3.0) is fully compatible with IBM's PRINT command and
; hopefully most other print spoolers. I changed the method by which
; LPTX determines if a resident copy of itself is already in memory.
;
; Calling sequence:
; lptx -1 -o <d:[pathname]filename>
;
; where -1 means redirect LPT1, -2 means redirect LPT2, -3 means redirect
;	   LPT3
;	   This option must appear first
;
;       -o means start the redirect to file speicfied. If rerouting
;          is already in progress for the selected line printer,
;	   the old file will be closed first.
;	   (If you do not specify -o but you do specify a line printer,
;	   LPTx will use either the last file name that you gave when
;	   you loaded LPTx or will use the file named LPTX1.LST which it
;	   will create in the root directory 
;	   on the default drive - where x is 1, 2, or 3.)
;
;	   It is not necessary that you specify the complete path name
;	   for the file. LPTx opens and closes the file each time that it
;	   writes out a block. If you change directories, LPTx will 
;	   be able to find the file because it save the complete path.
;
;	-c means close the file and send all furthur output directly to the
;	   line printer.
;
; if neither option is specified, LPTx just displays the program status.
;
; note: -1, -2, and -3 are mutually exclusive
;   	-o and -c are mutually exclusive
;
; examples:
;
; lptx				Displays the program status
;
; lptx ?			Displays a HELP screen
;
; lptx -1			routes LPT1 output to file named
;				LPTX1.LST on the default drive or the last
;				named file.
;
; lptx -o a:\able.xxx		routes LPT1 output to file named
;	or			a:\able.xxx. Any open redirection
; lptx a:\able.xxx		disk file for LPT1 is closed.
;
; lptx -2 b:xx.lst		routes LPT2 output to file named
;				XX.LST in the default directory
;				on drive B:. Any open redirection
;				disk file for LPT2 is closed.
;
; lptx -3 d:\ab\cd\file.lst	redirects LPT3 output to the file named
;				file.lst in the directory ab\cd on drive
;				d:.
;
; lptx -c			closes any disk files open for LPT1 and sends
;	or			the output back to the line printer
; lptx -1 -c			If no rerouting is taking place to LPT1,
;				this is	a NOP. LPT2 and LPT3 are not
;				affected.
;
; lptx -2 -c			closes any disk file open for LPT2 and
;				sends the output back to line printer.
;				if no rerouting is taking place to LPT2,
;				this is a NOP. LPT1 and LPT3 are not
;				affected.
;
; By rerouting LPT2 or LPT3 to a disk file, you can in effect have 2 or 3
; printers on your system. LPT1 can be your physical printer and you can
; have LPT2 output going to disk. When you redirect LPT2 or LPT3, LPT1 works
; normally.
;
; If you are rerouting to a diskette file, do not remove the diskette
; once the rerouting starts. I recommend rerouting to a hard disk or
; a RAM disk.
;
; If LPTx encounters any kind of error during the rerouting, it terminates
; operation and sends output back to the line printer. It does not display
; anything but beeps the speaker four times. This prevents your currently
; running program from possibly getting destroyed.
; An error on LPT1 redirect does not shut down LPT2 or LPT3 redirect.
;
; LPTx captures the int 17h interrupt vector. It may not operate correctly
; with other routines what also intercept that vector.
;
; Problems may occur with print spoolers which also take over the int 17h 
; vector. You can be sure that LPTX works correctly by running LPTX after
; you have run your print spooler. LPTX will be transparent to the print
; spooler but your print spooler may not be transparent to LPTX.
; LPTX works fine with IBM's PRINT command.
;
; LPTx also captures the int 24h critical error interrupt vector. This is
; done only for the period that LPTx is using the disk. This prevents
; the generation of funny error messages in the middle of other programs
; that you may be running. (LPTx justs beeps 4 times and clears itself
; out of way if a disk error occurs).
;
; This version of LPTx can redirect all three printers to three different
; files with all 3 active at the same time.
;
; LPTx uses about 7K of memory for the resident data buffers and 
; interrupt handler.
;
; My thanks to Don D. Worth of UCLA/OAC for his program named SPOOL.
; He included the concept of the saving the DOS stack when the interrupt
; routine called DOS on its own. I stole the idea for this program.
;
; If you modify or find any bugs in this program, I would appreciate
; it if you would drop me a line with the changes. Use the address
; above.
;
if1
	%out Pass 1
else
	%out Pass 2
endif
;
;-----------------------------------------------------------------
null		equ	0
off		equ	0
on		equ	1
empty		equ	0
cr		equ	13
lf		equ	10
dollar		equ	'$'
colon		equ	':'
backslash	equ	'\'
blank		equ	' '
dash		equ	'-'
dos_call	equ	21h
bufsize		equ	200H	;size of DMA buffer
display_output	equ	9	;for DOS call
def_drive	equ	19h
create_file	equ	3Ch
open_file	equ	3Dh
close_file	equ	3Eh
write_file	equ	40h
delete_file	equ	41h
lseek_file	equ	42h
def_path	equ	47h
find_file	equ	4Eh
;-----------------------------------------------------------------
;
; Macros
display	macro	msg
	mov	DX,offset msg
	mov	AH,display_output
	int	dos_call
	endm
;
;-----------------------------------------------------------------
;
p_block	struc
;
; data structure - these variables are used only in the
; memory resident copy of LPTx. BX is set to point to the offset of the
; allocation of this structure for the selected LPT
;
active	db	off		;1 = this LPTx is on, 0 = off
handle	dw	null		;handle of disk file used by this LPT
;
; space for redirection disk file name
;
filen	db	'a:\lptx'
ptr	db	blank
	db	'.lst',null
	db	'                          '
	db	'                        '
;
sp_left	dw	empty		;bytes left in DMA buffer for this LPT
buffer	db	bufsize dup(?)	;data buffer for this LPT
;
p_block	ends
;
;
;
;-----------------------------------------------------------------
;
subttl	Main Code
page
%out Assembling CODE Segment
cseg	segment para public 'CODE'
	assume  CS:cseg,DS:nothing,SS:nothing
;
	org	100h
lptx:	jmp	l_start
id	dw	03579h		;unique ID for this program
;
; What follows is three allocations of the Structure p_block
; One for each line printer that we can support.
; With this, all three line printers have DMA buffers and flag
; variables.
; BX is used to point to the offset of the allocation currently in use
;
; Line printer 1
lpt1	p_block	<,,,'1'>
;
; Line printer 2
lpt2	p_block	<,,,'2'>
;
; Line printer 3
lpt3	p_block	<,,,'3'>
;
lptxe	db	7,7,7,7,dollar	;ring bell four times
crit_flag	db	0	;set to one if critical error occured
off_crit	dw	0	;save old critical error address
seg_crit	dw	0
;
dosstk	db	0C80H dup(?)	;place to save INT 21H's stack
stksav	dd	0		;caller's stack EA
	db	64 dup('STACK   ')
stk	equ	this byte
;-----------------------------------------------------------------
;
; Interrupt handler
;
prt_int:
	cmp	DX,0F0Fh	;my flag to detect that LPTX is
				;already loaded and alive.
	jne	reg_call	;This is a regular print call
	jmp	ret_ack
reg_call:			; set up BX
	push	BX
	cmp	DX,0		;lpt1?
	jne	chk_lpt2	;no
	mov	BX,offset lpt1	;offset to LPT1
	jmp	short bx_set
chk_lpt2:
	cmp	DX,1		;lpt2?
	jne	chk_lpt3	;no
	mov	BX,offset lpt2	;offset to LPT2
	jmp	short bx_set
chk_lpt3:
	cmp	DX,2		;lpt3?
	jne	ill_ptr		;no - bad printer number
	mov	BX,offset lpt3	;offset to LPT3
bx_set:
	cmp	CS:[BX].active,off	;are we active?
	je	sleep		;no
	cmp	AH,1		;initialize call?
	je	do_nix		;yes	
	cmp	AH,2		;status call?
	je	do_nix		;yes
	cmp	AH,0		;print call?
	jne	do_nix		;no
	jmp	prt_it		;we are active
do_nix:	mov	AH,90h		;Ready Status
	pop	BX
	iret
;
ill_ptr:mov	AH,0
	pop	BX
	iret			;return with error status
;
ret_ack:			;return acknowledgement that I'm here
	mov	DX,05555h
	mov	AX,0AAAAh
	push	CS		;now set up ES to point to the resident
	pop	ES		; data area
	iret
;
sleep:	pop	BX		;restore BX before we go to sleep
	db	0EAh		;jump immediate to next handler
oldint	dd			;address of old int 17 routine
;-----------------------------------------------------------------
;
; Start the print process
;
prt_it:	push	AX
	push	BX
	push	CX
	push	DX
	push	DS
	push	ES
	push	SI
	push	DI
	push	BP
				; DS is used as the segment register
				; for all data during the interrupt
;
	push	CS
	pop	DS		;set up DS
;
	cli
	mov	SI,SS
	mov	word ptr stksav+2,SI	;save caller's stack
	mov	SI,SP
	mov	word ptr stksav,SI
	mov	SI,CS
	mov	SS,SI			;give me new bigger stack
	mov	SI,offset stk
	mov	SP,SI
	sti

	call	prnt			;print the character
;
	cli
	mov	SI,word ptr stksav
	mov	SP,SI			;restore caller's stack
	mov	SI,word ptr stksav+2
	mov	SS,SI
	sti
;
	pop	BP
	pop	DI
	pop	SI
	pop	ES
	pop	DS
	pop	DX
	pop	CX
	pop	BX
	pop	AX
	jmp	do_nix
;-----------------------------------------------------------------
;
; Critical Error Handler
;
crit_int:				;got critical error
	mov	CS:crit_flag,on		; set flag
	mov	AL,0			;tells DOS to ignore the
	iret				;error
;-----------------------------------------------------------------
;
; Print a character in AL
;
prnt	proc	near
	cmp	DS:[BX].active,off
	je	prtext			;nothing there?
	push	AX
	cmp	DS:[BX].sp_left,bufsize	;buffer full
	jne	intadd			;no
	call	flush			;yes, flush buffer
intadd:	pop	AX
	mov	DI,BX			;offset of this printer's allocation
	add	DI,offset buffer	;add in offset of buffer
	add	DI,DS:[BX].sp_left	;add in current byte count
	mov	DS:[DI],AL		;stuff it
	inc	DS:[BX].sp_left
prtext:	ret				;done
prnt	endp
;
;	Flush print buffer to disk file
;
flush	proc	near
	cmp	DS:[BX].sp_left,empty	;buffer non-empty?
	jne	flush_buf		;empty, skip it
	ret				;exit
flush_buf:
	mov	DS:[BX].sp_left,empty	;else, reset it
;
	push	ES
	push	DS
;
;	Preserve a chunk of DOS 2.0 across int 21h
;	See PC Technical reference manual page D-7 for hint.
;	It comments that only DOS calls 0 - 12 can be safely made
;	from an interrupt handler. "Use of any other call will
;	destroy the DOS stack and will leave DOS in an 
;	unpredictable state." What we do here is save and restore
;	3200 bytes of the DOS stack and restore it later. We only
;	do it for the DOS stack. If this was invoked by a user
;	program, we won't save the DOS stack or the user stack.
;	It is not necessary.
;
	mov	AX,word ptr DS:stksav+2	;get callers stack segment
	cmp	AX,0100h		; is it DOS?
	ja	flusha			;no, don;t bother to save it
	mov	AX,DS			;copy to my segment
	mov	ES,AX
	mov	AX,word ptr DS:stksav+2	;copying from caller's stack
	mov	DS,AX
	mov	SI,0			;offset into DOS's stack
	mov	DI,offset dosstk
	mov	CX,0C80h		;length to save
	cld
	rep	movsb			;copy DOS's stack
;
	pop	DS
	push	DS
;
flusha:
;
	push	AX			;save the character
	push	BX
	push	ES
	mov	AX,3524h		;get old critical error vector
	int	dos_call
	mov	DS:off_crit,BX
	mov	DS:seg_crit,ES
	mov	DX,offset crit_int
	mov	AX,2524h	
	int	dos_call			;trap critical error vector
	mov	DS:crit_flag,off		;clear critical error flag
	pop	ES
	pop	BX
	pop	AX
;					open file
	mov	DX,BX
	add	DX,offset filen		;filename
	mov	AL,1			;open for writing
	mov	AH,open_file
	int	dos_call
	mov	DS:[BX].handle,AX	;file handle
	jc	flush_err		;error
	cmp	DS:crit_flag,on		;critical error?
	je	flush_err		;yes
;
	push	BX
	mov	AH,lseek_file
	mov	AL,2			;end of file
	mov	CX,0			;offset 0
	mov	DX,0
	mov	BX,DS:[BX].handle
	int	dos_call
	pop	BX
	jc	flush_err		;some seek error
	cmp	DS:crit_flag,on		;critical error?
	je	flush_err		;yes
;
	mov	CX,bufsize		;buffer length
	mov	DX,BX			;offset of structure allocation
	add	DX,offset buffer	;add offset of buffer within the
					;	allocation
	push	BX
	mov	AH,write_file
	mov	BX,DS:[BX].handle	;file handle
	int	dos_call		;buffer address is DS:DX
	pop	BX
	jnc	flush_ok
	cmp	DS:crit_flag,on		;critical error?
	je	flush_err		;yes
	cmp	AX,bufsize		;did DOS write it all?
	je	flush_ok		;yes
;
flush_err:
	display lptxe			;ring bell
	mov	DS:[BX].active,off	;turn us off
	mov	DS:crit_flag,off	;clear error flag
;					;then try to close the file
;					;to save what we can
flush_ok:
	push	BX
	mov	BX,DS:[BX].handle
	mov	AH,close_file		;close the file
	int	dos_call
	pop	BX
;
flush_exit:
	pop	DS
	pop	ES
;
	push	DS
	lds	DX,dword ptr DS:off_crit
	mov	AX,2524h		;restore critical error vector
	int	dos_call
	pop	DS
;
	mov	AX,word ptr DS:stksav+2	;copying to DOS's workarea
	cmp	AX,100H
	ja	flushe			;must be DOS's segment
	push	ES
	mov	ES,AX
	mov	DI,0			;restore data areas
	mov	SI,offset dosstk
	mov	CX,0C80H		;length to restore
	cld
	rep	movsb			;copy DOS's stack
	pop	ES			;restore ES
;
flushe:	ret
flush	endp
;
end_res	db	0
;
;
; This is the end of the memory resident portion of LPTx
;
;
;--------------------------------------------------------------------
;--------------------------------------------------------------------
;--------------------------------------------------------------------
;
; all following data is in the Code Segment
;
mach_type	db	0
save_psp	dw	0
DOS_version	db	0		;Major Version Number
		db	0		;Minor Version Number
drive		db	0		;default drive number 0=A etc.
flag_27		db	0		; 1=make this copy resident
wrong_dos	db	'DOS 2.0 or later required for LPTx',lf,cr,dollar

up_msg		db	'LPTx - Line Printer Redirection Program - V3.00'
		db	lf,cr,'   Copyright 1985 Mark C. DiVecchio',lf,cr
		db	dollar
resident	db	lf,cr,'Resident Portion of LPTx Loaded',lf,lf,cr
		db	dollar
lptx_err_3	db	'Could not delete file',lf,cr,dollar
lptx_over	db	cr,lf,'File already exists. Do you want to overwrite '
		db	'it? (y or n)  :$'
lptx_nc		db	'File selection canceled',cr,lf,dollar
lptx_del	db	'File is being overwritten',lf,cr,dollar
lptx_cr		db	lf,cr,dollar
lptx_bad	db	'Invalid Option',lf,cr
		db	'Calling sequence:',lf,cr
		db	'lptx {-1,-2,-3} {-c -o <d:[pathname]filename>}'
		db	lf,cr,dollar
lptx_on		db	lf,cr,'Redirection started. Disk file opened.'
		db	lf,cr,dollar
lptx_off	db	lf,cr,'Redirection ended. Disk file closed.'
		db	lf,cr,dollar
lptx_creat	db	'Could not create the disk file',lf,cr,dollar
;
; HELP screen
;
help_msg	db	lf,cr,'Calling sequence : ',lf,lf,cr
		db	'LPTX -p -f <[d:][\pathname\pathname]filename>'
		db	lf,lf,cr
		db	'    where  p = printer number : 1, 2, or 3',lf,cr
		db	'           f = function : o for open a print file'
		db	lf,cr
		db	'                          c for close a print file'
		db	lf,cr
		db	'           drive letter & pathname are optional'
		db	lf,cr
		db	'    defaults : p = 1',lf,cr
		db	'               f = o',lf,cr,dollar
;
; messages for STAT proc
;
stat_stat	db	cr,lf,'LPTx Status :',cr,lf,dollar
stat_lp		db	'lpt'
stat_ptr	db	' : $'
stat_off	db	' not redirected',cr,lf,dollar
stat_dir	db	' redirected to disk file '
stat_fn		db	60 dup (blank)
;
;
yn_max		db	2	;max # of char
yn_act		db	0
yn_in		db	2 dup (0)
;
;--------------------------------------------------------------------
;
; This is the main routine which is executed each time that LPTx is 
; called. In this routine, DS points to the data segment which is
; transient. ES points to the data segment which is permanently
; resident. ES:BX points to the data structure for the selected 
; line printer, 1, 2, or 3.
; The offsets are the same for both. If this is the first
; time that LPTx is run, then ES=DS.
;
l_start:
	sti		;interrupts on
	push	DS	;Save DS
	xor	AX,AX	;clear AX for return IP
	push	AX	;put 0 on stack
;
;to check for machine type look at
; F000:FFFE
;	= FF	IBM PC
;	= FE	IBM XT
;	= FD	IBM PCjr
;	= FC	IBM PC AT
;
	mov	AX,0F000h
	mov	ES,AX
	mov	BX,0FFFEh
	mov	CL,ES:[BX]	;get machine type
	mov	mach_type,CL	;save machine type
	mov	save_psp,DS	;segment address of PSP
;
; get the DOS version number
; returns zero for pre DOS 2.0 releases
	mov	AH,30h
	int	dos_call	;call DOS
	mov	word ptr DOS_version,AX	
;
	cmp	DOS_version,2	;is it DOS 2.+
	jge	dos_ok		;yes
	display	wrong_dos	;print error message
	mov	AH,0
	int	dos_call	;terminate
dos_ok:
;
	mov	AH,def_drive	;get current default drive
	int	dos_call
	mov	drive,AL	;save the drive number
	display	up_msg		;print program ID
;
; get old interrupt handler
;
	mov	flag_27,off	;to not make resident
	mov	AL,17h		;get current vector address
	mov	AH,35h
	int	dos_call
	mov	word ptr oldint,BX
	mov	word ptr oldint[2],ES	;save it for later use
;
; are we already resident in memory?
;
	mov	DX,0F0Fh		;check if LPTX is already resident
	mov	AX,2			;get status
	int	17h			;call int 17h - BIOS
	cmp	DX,5555h		;my handler sets DX to 5555h
					;and sets ES 
	je	in_core			;yes - ES has segment address
	mov	flag_27,on		;to make this copy resident
	push	CS
	pop	ES			;set ES to CS for segment address
;
	mov	AL,drive
	add	AL,'a'			;make it a letter
	mov	BX,offset lpt1
	mov	ES:[BX].filen,AL	;put it into the filename
	mov	BX,offset lpt2
	mov	ES:[BX].filen,AL	;put it into the filename
	mov	BX,offset lpt3
	mov	ES:[BX].filen,AL	;put it into the filename
in_core:				;ES is ok
; ----------------------------------------------------
; ES now points to resident data area
;
; set up ES:BX to point to default data structure
;
	mov	BX,offset lpt1		;offset - default to LPT1
;
;get options and file name
;scan input line for line printer number
;
	mov	SI,81h			;starting offset
	mov	CL,DS:80h		;length of input line
	mov	CH,0
	cmp	CX,0			;nothing?
	jne	inp_lp			;no
	jmp	nor_exit		;yes, then just display status
inp_lp:
	cmp	byte ptr DS:[SI],'?'	;a ?  ?
	jne	cont_scan		;no
	jmp	help			;yes - go show help data
cont_scan:
	cmp	byte ptr DS:[SI],dash	;a dash ?
	je	got_opt			;yes
	cmp	byte ptr DS:[SI],cr	;a carriage return?
	je	scan_done		;yes
	cmp	byte ptr DS:[SI],blank	;a blank?
	je	inp_ret			;yes
	jmp	no_b			;assume that we got a file name
					;without the -o option
inp_ret:
	inc	SI			;ignore blanks
	loop	inp_lp			;continue to scan
;
; scan of whole line is complete, if options were not found, we
; use defaults : LPT1 and file LPTX1.LST on the default drive.
; note : at least one option must be specified
;
scan_done:
	jmp	lptx_make		;go create the file
;
got_opt:				;we got an option
	inc	SI			;to option
	cmp	byte ptr DS:[SI],'1'	;LPT1?
	jne	chk_2
	mov	BX,offset lpt1		;offset from ES
	jmp	short inp_ret
chk_2:	cmp	byte ptr DS:[SI],'2'	;LPT2?
	jne	chk_3
	mov	BX,offset lpt2		;offset from ES
	jmp	short inp_ret
chk_3:	cmp	byte ptr DS:[SI],'3'	;LPT3?
	jne	chk_fil
	mov	BX,offset lpt3		;offset from ES
	jmp	short inp_ret
chk_fil:				;is it file?
	cmp	byte ptr DS:[SI],'o'	;open a file
	je	file_op			;yes
	cmp	byte ptr DS:[SI],'c'	;close a file
	je	file_cl			;yes
	display	lptx_bad		;incorrect option
	jmp	nor_ex
;
file_cl:				;close the output file
	cmp	ES:[BX].active,on	;are we active?
	jne	no_close		;no
	mov	AL,1AH			;CTRL-Z
	mov	word ptr ES:stksav+2,AX	;do this so that prnt does
					;not bother to save the DOS stack
	push	DS
	push	ES
	pop	DS			;set DS to point to resident
					;data segment
	call	prnt			;print end of file mark
	call	flush			;flush out write buffer
	pop	DS			;restore DS
;
	mov	ES:[BX].active,off	;make us inactive
	display lptx_off		;redirection off message
no_close:
	jmp	nor_exit		;nothing to close so exit
file_op:				;open a file for output
;get the file name
	inc	SI			;to next chracter
	cmp	byte ptr DS:[SI],blank	;a blank?
	jne	no_b			;no
	inc	SI			;skip over blank
no_b:
; at this point, we have found a new file name. We close the old
; file if one was open
	cmp	ES:[BX].active,on	;are we active?
	jne	no_cl			;no
	mov	AL,1AH			;CTRL-Z
	mov	word ptr ES:stksav+2,AX	;do this so that prnt does
					;not bother to save the DOS stack
	push	DS
	push	ES
	pop	DS			;set DS to point to resident
					;data segment
	call	prnt			;print end of file mark
	call	flush			;flush out write buffer
	pop	DS			;restore DS
;
	mov	ES:[BX].active,off	;make us inactive
	display lptx_off		;redirection off message
no_cl:
	mov	DI,BX			;base of structure
	add	DI,offset filen		;add offset of destination
;
	push	SI			;save pointer to file name
; search for a drive letter
	inc	SI			;should point to a colon if
					;one is there
	cmp	byte ptr [SI],colon	;?
	je	got_drive		;yes
get_drive:
	mov	AL,drive		;get drive letter
	add	AL,'a'			;make it a letter
	mov	ES:[DI],AL		;put it in file name
	inc	DI
	mov	byte ptr ES:[DI],colon	;put in a colon
	inc	DI
	jmp	path_search
got_drive:
	pop	SI			;move pointer back to start
	mov	AL,[SI]			;get the given drive
	mov	ES:[DI],AL		;move it
	sub	AL,'a'			;make it a number
	mov	drive,AL		;save the drive number
	inc	SI
	inc	DI
	mov	byte ptr ES:[DI],colon
	inc	DI
	inc	SI
	push	SI			;save new start pointer
path_search:
; now search for a backslash which says that a pathname was given
bk_s_lp:cmp	byte ptr [SI],backslash
	je	got_path		;a path
	cmp	byte ptr [SI],cr	;end of the file name?
	je	get_path		;yes with no path
	inc	SI
	jmp	short bk_s_lp			;loop
get_path:
	mov	byte ptr ES:[DI],backslash	;create the path
	inc	DI
	mov	DL,drive		;the current drive
	inc	DL			;bump it for DOS
	push	DS
	push	ES
	pop	DS			;set up DS for DOS
	mov	SI,DI			;set up SI for pathname
	mov	AH,def_path		;get current directory
	int	dos_call		;path goes into DS:SI
	pop	DS			;restore DS
	cmp	byte ptr ES:[SI],null	;null path?
	je	null_path		;yes - root directory
path_lp:				;now find the end of the string
	cmp	byte ptr ES:[SI],null	;null byte marks end of pathname
	je	end_path		;now append the file name
	inc	SI
	jmp	short path_lp
end_path:
	mov	byte ptr ES:[SI],backslash
	inc	SI
null_path:
	mov	DI,SI			;DI is destination
got_path:
	pop	SI			;restore source of filename
; pick up everything to next blank
get_lp:
	mov	AL,DS:[SI]		;character
	mov	ES:[DI],AL		;put it away
	cmp	AL,cr			;was it a Carriage Return?
	je	end_line
	cmp	AL,blank		;was it a space?
	je	end_line
	inc	SI
	inc	DI
	jmp	short get_lp		;no so get next character
end_line:
	mov	byte ptr ES:[DI],null	;zero out the CR or blank
					;at the end of the filename
					;it becomes an ASCIIZ string
	sub	DI,BX			;now take out the base and
	cmp	DI,offset filen		; make sure that we got something
	jne	lptx_make		;file name was ok
	display lptx_creat		;could not understand the file name
	jmp	nor_exit		;don't stay resident
;
nor_ex:	jmp	nor_exit

lptx_make:
;
; default DTA used by Find File is set by DOS to an offset of
; 80h into this program's Program Segment Prefix
;
	push	DS
	push	ES
	pop	DS			;uses DS:DX
	mov	DX,BX
	add	DX,offset filen		;file name
	mov	AH,find_file
	mov	CX,0			;normal files only
	int	dos_call		;find first match
	pop	DS
	jnc	lptx_d			;file was found
	jmp	lptx_create		;not there - which is ok
;file already exists
lptx_d:	display lptx_over
	mov	DX,offset yn_max;input buffer
	mov	AH,0AH
	int	dos_call
	cmp	yn_act,0		;anything typed?
	display	lptx_cr
	je	lptx_x			;no - exit
	cmp	yn_in,'y'		;a yes?
	je	lptx_d_yes		;yes
	cmp	yn_in,'Y'		;a yes?
	je	lptx_d_yes		;yes
lptx_x:	display	lptx_nc
	jmp	nor_exit	;all done if we can't overwrite
				;see if we should abort the host
lptx_d_yes:
	display lptx_del
;
	push	DS
	push	ES
	pop	DS			;uses DS:DX
	mov	DX,BX
	add	DX,offset filen		;file name
	mov	AH,delete_file
	int	dos_call		;delete file
	pop	DS
	jnc	lptx_create		;ok its gone
	display lptx_err_3		;can't delete it
	jmp	nor_exit
;
;
lptx_create:
;
; create the file
	push	DS
	push	ES
	pop	DS			;uses DS:DX
	mov	DX,BX			;base of this LPT's structure
	add	DX,offset filen		;file name
	mov	AH,create_file
	mov	CX,0			;normal files only
	int	dos_call			;find first match
	pop	DS
	jnc	creat_ok
	display lptx_creat		;could not create the file
	jmp	nor_exit		;don't stay resident
;
creat_ok:				;now close the file
	push	BX
	mov	BX,AX			;AX was loaded by the create file
					;	call
	mov	AH,close_file		;close the file
	int	dos_call
	pop	BX
;
	display	lptx_on
; set the program up for writing
	mov	ES:[BX].sp_left,empty	;set buffer empty
	mov	ES:[BX].active,on	;set us on
;
	cmp	flag_27,on		;make this one resident?
	jne	nor_exit		;no
;
; Now set LPTX up as the new int 17h interrupt handler
;
	mov	AH,25h			;set interrupt vector
	mov	AL,17h			;BIOS printer
	mov	DX,offset prt_int
	int	dos_call
	display resident		;resident loaded message
	call	stat			;display status
	mov	DX,offset end_res
	int	27h			;terminate but stay resident
;
; HELP printer
;
help:	display	help_msg		;display the HELP screen
	jmp	short nor_exit
;
; Normal exit for transient copy of LPTX
;
nor_exit:
	call	stat			;display status
	mov	AH,0
	int	dos_call		;terminate
;------------------------------------------------------------------------
;
; displays the status of each of the three line printers
;
stat	proc	near
; display each LPTx with a message "not redirected"
;			or redirected to <filename>
	display	stat_stat
stat_1:
	mov	BX,offset lpt1		;first printer
	mov	stat_ptr,'1'
	display	stat_lp
	cmp	ES:[BX].active,on	;are we active?
	je	stat_1_a		;yes
	display stat_off
	jmp	short stat_2
stat_1_a:
	mov	SI,BX			;base
	add	SI,offset filen		;offset
	mov	DI,offset stat_fn
stat_1_lp:
	mov	AL,ES:[SI]
	mov	[DI],AL
	inc	SI
	inc	DI
	cmp	AL,null			;loop till a null byte is found
	jne	stat_1_lp
	mov	byte ptr [DI],cr
	inc	DI
	mov	byte ptr [DI],lf
	inc	DI
	mov	byte ptr [DI],dollar
	display stat_dir		;display file name
;
stat_2:
	mov	BX,offset lpt2		;second printer
	mov	stat_ptr,'2'
	display	stat_lp
	cmp	ES:[BX].active,on	;are we active?
	je	stat_2_a		;yes
	display stat_off
	jmp	short stat_3
stat_2_a:
	mov	SI,BX			;base
	add	SI,offset filen		;offset
	mov	DI,offset stat_fn
stat_2_lp:
	mov	AL,ES:[SI]
	mov	[DI],AL
	inc	SI
	inc	DI
	cmp	AL,null			;loop till a null byte is found
	jne	stat_2_lp
	mov	byte ptr [DI],cr
	inc	DI
	mov	byte ptr [DI],lf
	inc	DI
	mov	byte ptr [DI],dollar
	display stat_dir		;display file name
;
stat_3:
	mov	BX,offset lpt3		;third printer
	mov	stat_ptr,'3'
	display	stat_lp
	cmp	ES:[BX].active,on	;are we active?
	je	stat_3_a		;yes
	display stat_off
	jmp	short stat_done
stat_3_a:
	mov	SI,BX			;base
	add	SI,offset filen		;offset
	mov	DI,offset stat_fn
stat_3_lp:
	mov	AL,ES:[SI]
	mov	[DI],AL
	inc	SI
	inc	DI
	cmp	AL,null			;loop till a null byte is found
	jne	stat_3_lp
	mov	byte ptr [DI],cr
	inc	DI
	mov	byte ptr [DI],lf
	inc	DI
	mov	byte ptr [DI],dollar
	display stat_dir		;display file name
;
stat_done:
	ret
stat	endp
;
cseg	ends
%out EOF
	end	lptx
;
;
;
;
;
; Good Luck
;

