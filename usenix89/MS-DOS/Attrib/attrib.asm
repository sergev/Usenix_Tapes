	PAGE	64,132
	TITLE	ATTRIB - a program to alter file attributes
	NAME	ATTRIB
;
; This program was written by Bob Eager, Herne Bay, England.
; It is placed in the public domain. No part of this program
; may be copied and sold for gain.
;
; There isn't really much in the way of documentation. ATTRIB
; works like the DOS 3 command of the same name (except that it
; is a lot smaller and also DOES a lot MORE!)
;
; For those unfamiliar with that:
;    ATTRIB filename     -   reports file attributes as single letters
;                            (H, S, R)
;    ATTRIB +x filename  -   sets the x attribute (H, S or R)
;    ATTRIB -x filename  -   unsets the x attribute (H, S or R)
;
;  The filename may be a wildcard.
;
; Inverse attributes are provided as a friendly touch:
;
;      Hidden   <==>   Visible
;      System   <==>   User
;      Read     <==>   Write
;
; So +R is the same as -W, etc.
;
;   Enjoy!
;
	.SALL
;
; Values for exit status:
;    0 - Success
;    1 - Parameter too long
;    2 - Incorrect DOS version
;    3 - File does not exist
;    4 - Invalid attribute letter
;
; Constants
;
TAB	EQU	09H			; Tabulate
LF	EQU	0AH			; Linefeed
CR	EQU	0DH			; Carriage return
PTHSEP1	EQU	'\'			; Normal path separator
PTHSEP2	EQU	'/'			; Alternate path separator
;
STDOUT	EQU	1			; Standard output handle
STDERR	EQU	2			; Standard error handle
MAXFNAM	EQU	63			; Maximum length of filename
;
; Macro definitions
;
MSG	MACRO	X,Y			;; define a message
MES&X	DB	Y
LMES&X	EQU	$-MES&X
	ENDM
;
$MSG	MACRO	X			;; display a message on standard error
	MOV	BX,STDERR		;; file handle
	MOV	DX,OFFSET MES&X		;; location of message
	MOV	CX,LMES&X		;; length of message
	MOV	AH,40H			;; write function
	INT	21H
	ENDM
;
CSEG	SEGMENT
;
	ORG	100H
;
	ASSUME	CS:CSEG,DS:CSEG,ES:CSEG,SS:CSEG
;
MAIN	PROC	FAR
	JMP	MAIN10
;
	SUBTTL	Data areas
	PAGE+
;
; Storage
;
ATT	DW	?			; Mask and bits specified by parameter
FIRST	DB	?			; Used for 'find first' calls
PARPTR	DW	?			; Pointer to next parameter
PSIZE	DW	?			; Size of path part of filename
DISP	DB	?,?,?,'    '		; Attribute display string
LDISP	EQU	$-DISP			; Length of DISP
FNAME	DB	MAXFNAM+1 DUP (?)	; Extra byte for terminator
DTA	DB	43 DUP (?)		; Alternate disk transfer area
;
; Table of attribute settings
;
; The table ATTC is a list of valid attribute letters
; The table ATTON corresponds one-to-one with ATTC for setting an attribute
; The table ATTOFF corresponds one-to-one with ATTC for clearing an attribute
; ATTON and ATTOFF use the high byte of each entry as a mask to clear
; unwanted bits, and the low byte to indicate bits to be set.
;
ATTC	DB	'RWHVSU'
NATT	EQU	$-ATTC
ATTON	DW	0FF01H,0FE00H,0FF02H,0FD00H,0FF04H,0FB00H
ATTOFF	DW	0FE00H,0FF01H,0FD00H,0FF02H,0FB00H,0FF04H
;
CRBUF	DB	CR,LF
;
	DW	32 DUP (?)		; Stack
STACK	LABEL	WORD
;
; Messages
;
	MSG	1,<'Parameter too long',CR,LF>
	MSG	2,<'Filename too long - '>
	MSG	3,<'Incorrect DOS version',CR,LF>
	MSG	4,<'No matching file found for '>
	MSG	5,<'Invalid attribute letter',CR,LF>
;
	SUBTTL	Main code
	PAGE+
;
MAIN10:	MOV	SP,OFFSET STACK		; set up stack
;
	MOV	AH,30H
	INT	21H			; get DOS version
	XCHG	AH,AL			; make more useful form
	CMP	AX,0200H		; version 2 at least?
	JAE	MAIN20			; j if so - OK
	$MSG	3			; "Incorrect DOS version"
	MOV	AL,2			; exit status
	JMP	SHORT MAIN60		; join common exit code
;
MAIN20:	MOV	DX,OFFSET DTA		; set alternate DTA to avoid...
	MOV	AH,1AH			; ...overwriting parameters
	INT	21H
	MOV	PARPTR,81H		; start of parameter area
	CALL	GETFLAGS		; get any attribute flags
	JC	MAIN60			; j if error (unrecognised flag)
	MOV	ATT,AX			; save information for later on
MAIN30:	CALL	GETPAR			; read next parameter item
	JC	MAIN50			; j if no more parameters
	MOV	BYTE PTR FIRST,0	; clear 'find first' flag
MAIN40:	CALL	GETNAM			; get next filename
	JC	MAIN30			; j if no more names
	CALL	DOATTR			; handle attribute
	JC	MAIN60			; j if error (status in AL)
	JMP	MAIN40			; see if more to do
;
MAIN50:	XOR	AL,AL			; zero status
;
MAIN60:	MOV	AH,4CH			; exit
	INT	21H
MAIN	ENDP
;
	SUBTTL	Get attribute flag from command line
	PAGE
;
; This routine reads an attribute flag and its sense from the command line.
; Attribute flags start with '+' or '-' and continue with a letter.
; Possible letters are:  R  -  read only
;                        W  -  read and write (inverse of read only)
;                        H  -  hidden
;                        V  -  visible (inverse of hidden)
;                        S  -  system
;                        U  -  user (inverse of system)
;
; On entry:  PARPTR points to the first byte of the parameter string
;
;  On exit:  PARPTR has been updated past the attribute specification if
;            one was found
;
; Return is made with AH containing a mask to remove any unwanted bits,
; and with AL containing any additional bits to be set in the attributes.
; The value AX=0 is returned if no attribute flag was found, and carry is set
; if an unrecognised attribute letter was encountered.
;
GETFLAGS PROC	NEAR
	CLD				; autoincrement
	MOV	SI,PARPTR		; point to start of parameter list
	PUSH	SI			; save for possible restoration
GETF10:	LODSB				; skip leading spaces...
	CMP	AL,' '
	JE	GETF10
	CMP	AL,TAB			; ...and tabs
	JE	GETF10
	CMP	AL,'+'			; valid sense?
	JE	GETF20			; j if so
	CMP	AL,'-'			; other sense?
	JE	GETF20			; j if so
	POP	AX			; restore the parameter...
	MOV	PARPTR,AX		; ...pointer
	XOR	AX,AX			; indicate no attribute specified
	RET				; return with carry clear
;
GETF20:	POP	BX			; lose saved parameter pointer
	PUSH	AX			; save sense
	LODSB				; get attribute letter
	CALL	UPPER			; convert to upper case
	MOV	BX,OFFSET ATTC		; list of valid letters
	MOV	CX,NATT			; count of valid letters
	XOR	DI,DI			; pointer into letter table
GETF30:	CMP	AL,[BX+DI]		; found the letter?
	JE	GETF40			; j if so
	INC	DI			; point to next possibility
	LOOP	GETF30			; keep trying
	$MSG	5			; "Invalid attribute letter"
	POP	AX			; lose sense from stack
	MOV	AL,4			; exit status
	STC				; indicate error
	RET
;
GETF40:	MOV	BX,OFFSET ATTON		; assume sense 'on'
	POP	AX			; now check specified sense
	CMP	AL,'-'
	JNE	GETF50			; j if sense 'on' required
	MOV	BX,OFFSET ATTOFF	; else use different table
GETF50:	ADD	DI,DI			; form word offset
	MOV	AX,[BX+DI]		; get mask and attribute word
	MOV	PARPTR,SI		; update parameter line pointer
	CLC				; indicate success
	RET
GETFLAGS ENDP
;
	SUBTTL	Get next parameter
	PAGE
;
; This routine reads the next parameter from the command line, converting
; it to upper case and placing it into the area FNAME.
; Parameters are delimited by spaces, tabs, commas or the end of the line.
;
; On exit:
;    Carry is clear if a parameter was found
;    Carry is set if the parameter list is exhausted
;
GETPAR	PROC	NEAR
	CLD				; autoincrement
	MOV	SI,PARPTR		; point to start of possible parameter
GETP10:	LODSB				; skip leading spaces...
	CMP	AL,' '
	JE	GETP10
	CMP	AL,TAB			; ...and tabs
	JE	GETP10
	MOV	DI,OFFSET FNAME		; area to hold item
	MOV	CX,MAXFNAM		; maximum item size
GETP20:	CMP	AL,CR			; end of list?
	JNE	GETP30			; j if not
	DEC	SI			; must re-read next time
	JMP	SHORT GETP40
GETP30:	CMP	AL,' '			; other terminators...
	JE	GETP40
	CMP	AL,TAB
	JE	GETP40
	CMP	AL,','
	JE	GETP40			; ...are space, tab and comma
	CALL	UPPER			; convert to upper case
	STOSB				; store character away
	LODSB				; get next character
	LOOP	GETP20
;
GETP40:	JCXZ	GETP60			; j if too long
	XOR	AL,AL			; add terminator
	STOSB
	MOV	PARPTR,SI		; save new value
	CMP	CX,MAXFNAM		; null item?
	JNE	GETP50			; j if not
	STC				; indicate no more
	RET
;
GETP50:	CLC				; indicate item found
	RET
;
GETP60:	$MSG	1			; "Parameter too long"
	MOV	AL,1			; exit status
	JMP	MAIN60
GETPAR	ENDP
;
	SUBTTL	Get next filename
	PAGE
;
; This routine gets the next filename matching the parameter.
; The filename is left in FNAME.
;
; On exit:
;    Carry is clear if a match was found
;    Carry is set if no further matches exist
;
GETNAM	PROC	NEAR
	CLD				; autoincrement
	CMP	FIRST,0			; first time with this parameter item?
	JNE	GETN60			; j if not
	INC	FIRST			; mark not first any more
;
	MOV	SI,OFFSET FNAME		; find last separator
GETN10:	MOV	BX,SI			; initial value
GETN20:	LODSB				; get next byte of name
	OR	AL,AL			; end of name?
	JZ	GETN30			; j if so
	CMP	AL,PTHSEP1		; normal separator?
	JE	GETN10			; j if so, and update pointer
	CMP	AL,PTHSEP2		; alternate separator?
	JE	GETN10			; j if so, and update pointer
	CMP	AL,':'			; drive name?
	JE	GETN10			; j if so; this counts as separator
	JMP	GETN20
;
; BX now points at first character of last component of filename
;
GETN30:	SUB	BX,OFFSET FNAME		; derive length of path part
	MOV	PSIZE,BX
	MOV	DX,OFFSET FNAME
	MOV	CX,07H			; get read only, system and hidden files too
	MOV	AH,4EH			; find first matching file
	INT	21H
	JNC	GETN40			; j if match found
	$MSG	4			; "No matching file found for "
	MOV	BX,STDERR
	CALL	PRNAME
	MOV	BX,STDERR
	CALL	CRLF
	STC				; indicate error
	RET
;
GETN40:	MOV	SI,OFFSET DTA+30	; terminal filename
	MOV	DI,OFFSET FNAME
	ADD	DI,PSIZE		; point to filename part of path
	MOV	CX,MAXFNAM
GETN50:	LODSB
	STOSB
	OR	AL,AL
	LOOPNE	GETN50			; copy name and terminator
	JCXZ	GETN70			; j if name too long
	CLC
	RET				; return with carry clear
;
GETN60:	MOV	AH,4FH			; find next matching file
	INT	21H
	JNC	GETN40			; j if match found
	RET				; return with carry still set
;
GETN70:	$MSG	2			; "Filename too long - "
	MOV	BX,STDERR
	CALL	PRNAME
	MOV	BX,STDERR
	CALL	CRLF
	STC				; indicate error
	RET
GETNAM	ENDP
;
	SUBTTL	Actual attribute alteration and display code
	PAGE
;
DOATTR	PROC	NEAR
	CMP	ATT,0			; just display required?
	JZ	DOA100			; j if so
;
	MOV	AX,4300H		; first get existing attributes
	MOV	DX,OFFSET FNAME
	INT	21H			; attribute to CX (just CL really)
	JC	DOA200			; j if error (should not happen)
	AND	CL,BYTE PTR ATT+1	; remove unwanted bits
	OR	CL,BYTE PTR ATT		; add new bits
	MOV	AX,4301H		; set attributes now
	MOV	DX,OFFSET FNAME
	INT	21H
	JC	DOA200			; j if error (should not happen)
	RET
;
DOA100:	MOV	AX,4300H		; get attribute
	MOV	DX,OFFSET FNAME
	INT	21H			; returns attribute in CX
	JC	DOA200			; j if error (should not happen)
;
	MOV	DI,OFFSET DISP		; point to output area
	CLD				; autoincrement
	MOV	AH,' '
	MOV	AL,AH			; assume space initially
	TEST	CL,01H			; read only attribute?
	JZ	DOA110			; j if not
	MOV	AL,'R'			; else set character
DOA110:	STOSB				; set up for output
	MOV	AL,AH			; restore space
	TEST	CL,02H			; hidden attribute?
	JZ	DOA120			; j if not
	MOV	AL,'H'			; else set character
DOA120:	STOSB				; set up for output
	MOV	AL,AH			; restore space
	TEST	CL,04H			; system attribute?
	JZ	DOA130			; j if not
	MOV	AL,'S'			; else set character
DOA130:	STOSB				; set up for output
	MOV	BX,STDOUT		; handle
	MOV	CX,LDISP		; byte count for message
	MOV	DX,OFFSET DISP
	MOV	AH,40H			; write function
	INT	21H
;	JC	DOA200			; j if error
	MOV	BX,STDOUT		; handle
	CALL	PRNAME			; follow up with filename
	MOV	BX,STDOUT
	CALL	CRLF			; finish off line
DOA200:	RET
DOATTR	ENDP
;
	SUBTTL	Miscellaneous subroutines
	PAGE
;
; Routine to convert the letter in AL to upper case.
;
UPPER	PROC	NEAR
	CMP	AL,'a'			; check lower range
	JB	UPP10			; j if out of range
	CMP	AL,'z'			; check upper range
	JA	UPP10			; j if out of range
	SUB	AL,'a'-'A'		; do the conversion
UPP10:	RET
UPPER	ENDP
;
; Routine to write a carriage return and linefeed to the file specified
; by the handle in BX.
;
CRLF	PROC	NEAR
	MOV	CX,2			; byte count
	MOV	DX,OFFSET CRBUF
	MOV	AH,40H			; write function
	INT	21H
	RET
CRLF	ENDP
;
; Routine to output the filename in FNAME to the file specified by the
; handle in BX.
;
PRNAME	PROC	NEAR
	MOV	DI,OFFSET FNAME
	PUSH	DI			; save for later
	CLD				; autoincrement
	XOR	AL,AL			; search for zero byte
	MOV	CX,0FFFFH		; very long loop!
	REPNE	SCASB
	MOV	CX,DI
	POP	DX			; initial pointer
	SUB	CX,DX			; derive length
	MOV	AH,40H			; write function
	INT	21H
	RET
PRNAME	ENDP
;
CSEG	ENDS
;
	END	MAIN
----------CUT HERE-----------CUT HERE-----------CUT HERE-----------------
-- 
           Bob Eager

           rde@ukc.UUCP
           rde@ukc
           ...!mcvax!ukc!rde

           Phone: +44 227 66822 ext 7589
