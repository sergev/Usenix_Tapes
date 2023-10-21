	PAGE	60, 132
TITLE	XonXoff	30-Sep-85	XON/XOFF Printer Interface		|

;-----------------------------------------------------------------------|
;									|
;	XON/XOFF Printer Interface					|
;		Installed as "terminate and stay resident" program	|
;									|
;-----------------------------------------------------------------------|
;	REVISION HISTORY						|
;									|
; Number    DD-MMM-YY             WHO                     WHY		|
;-------|---------------|-----------------------|-----------------------|
; 0.0	|   30-Sep-85	| Frank Whaley		| Initial Release	|
;-----------------------------------------------------------------------|

IERport	EQU	03F9H			; Interrupt Enable Register
LSRport	EQU	03FDH			; Line Status Register
MCRport	EQU	03FCH			; Modem Control Register
MSRport	EQU	03FEH			; Modem Status Register
RBRport	EQU	03F8H			; Receiver Buffer Register
THRport EQU	RBRport			; Transmit Holding Register
Imask	EQU	0EFH			; interrupt enable mask
IIRport	EQU	03FAH			; Interrupt Identification Register

BrkInt	EQU	6			; break interrupt
EOI	EQU	20H			; End Of Interrupt
InpInt	EQU	4			; input interrupt
INTA00	EQU	20H
INTA01	EQU	21H
THRE	EQU	20H			; Transmit Holding Register Empty

XOFF	EQU	'S' - 40H
XON	EQU	'Q' - 40H

;-----------------------------------------------------------------------|
;	The Usual Stuff							|
;-----------------------------------------------------------------------|

cGroup	Group	Code

Code	Segment Public 'Code'

	Assume	CS:Code, DS:Code, ES:Code, SS:Code

	Org	100H

XonXoff:
	JMP	Install			; install traps

Ready	DB	80H			; ready flag

	PAGE
;-----------------------------------------------------------------------|
;	Printer interrupt handler					|
;									|
;	ENTRY :	as for printer interrupt (INT 17H)			|
;									|
;	EXIT :	ditto							|
;									|
;-----------------------------------------------------------------------|

Handler	Proc	Far

	TEST	AH,AH			; output request ??
	JNZ	Hand3			; if NZ: return status

	STI

Hand1:
	TEST	CS:Ready,0FFH		; ready ??
	JZ	Hand1			; wait until ready

	CLI
	PUSH	BX			; (+1) save
	PUSH	DX			; (+2)
	MOV	BL,AL			; save char
	MOV	DX,LSRport

Hand2:
	IN	AL,DX			; get line status
	TEST	AL,THRE			; transmit holding register empty ??
	JZ	Hand2			; if Z: uart not ready yet, loop

	MOV	AL,BL
	MOV	DX,THRport		; transmit port
	OUT	DX,AL			; transmit

	POP	DX			; (+1) restore
	POP	BX			; (+0)

Hand3:
	MOV	AH,CS:Ready
	OR	AH,10H			; always selected
	IRET

Handler	EndP

	PAGE
;-----------------------------------------------------------------------|
;	Serial interrupt server						|
;									|
;	ENTRY :	as from serial interrupt				|
;									|
;	EXIT :	all registers preserved					|
;									|
;-----------------------------------------------------------------------|

Server	Proc	Far

	PUSH	DX			; (+1) all what i use
	PUSH	AX			; (+2)

	MOV	DX,IIRport
	IN	AL,DX
	AND	AL,0FEH			; skip interrupt pending
	TEST	AL,AL			; modem status ??
	JZ	Serv2			; if Z: read MSR

	CMP	AL,BrkInt		; break ??
	JE	Serv3			; if E: just clear interrupt

	CMP	AL,InpInt		; input data received ??
	JE	Serv4			; if E: get and queue input

Serv1:
	MOV	AL,EOI			; signal end-of-interrupt
	OUT	INTA00,AL

	POP	AX			; (+1) restore
	POP	DX			; (+0)
	IRET

Serv2:					; read MSR
	MOV	DX,MSRport		; just read port to clear interrupt
	IN	AL,DX
	JMP	SHORT Serv1

Serv3:					; break received
	MOV	DX,LSRport
	IN	AL,DX
	JMP	SHORT Serv1

Serv4:					; look for XON/XOFF
	MOV	DX,RBRport
	IN	AL,DX			; AL = input
	AND	AL,7FH
	XOR	AH,AH			; assume not ready
	CMP	AL,XOFF
	JE	Serv5			; if E: not ready

	MOV	AH,80H			; assume ready
	CMP	AL,XON
	JE	Serv5			; if E: is ready

	JMP	SHORT Serv1		; otherwise no change

Serv5:
	MOV	CS:Ready,AH		; set flag
	JMP	SHORT Serv1

Server	EndP

	PAGE
;-----------------------------------------------------------------------|
;	XON/XOFF Printer Interface					|
;									|
;	ENTRY :	normal COM program entry				|
;									|
;	EXIT :	Terminate / Stay Resident				|
;									|
;-----------------------------------------------------------------------|

Install	Proc	Near

	MOV	DX,Offset Handler	; first take over INT 17 vector
	MOV	AX,2517H
	INT	21H

	MOV	DX,Offset Server	; now COM1 vector
	MOV	AX,250CH
	INT	21H

	CLI
	MOV	DX,MCRport		; get status of MCR
	IN	AL,DX
	OR	AL,0FH			; set DTR, RTS, OUT1, OUT2
	OUT	DX,AL			; init MCR

	MOV	DX,LSRport		; clear pending status
	IN	AL,DX
	MOV	DX,RBRport
	IN	AL,DX
	MOV	DX,IIRport
	IN	AL,DX
	MOV	DX,MSRport
	IN	AL,DX

	IN	AL,INTA01
	AND	AL,Imask
	OUT	INTA01,AL

	MOV	AL,1			; select Data Available interrupt
	MOV	DX,IERport
	OUT	DX,AL
	STI

	MOV	DX,Offset Install	; DS:DX -> end of "keep" area
	INT	27H			; terminate / stay resident

Install	EndP

Code	EndS

	END	XonXoff			; of XonXoff.asm
