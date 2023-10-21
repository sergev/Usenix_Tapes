; Definitions for the new ANSI driver.
; (C) 1986 Daniel Kegel
; May be distributed for educational and personal use only

takeBIOS	equ	0	; take over BIOS write_tty if true
is_8088		equ	1	; no fancy instructions if true
cls_homes_too	equ	1	; set true for ANSI.SYS compatibility

;Comment this out if running MASM 1.0

	if	is_8088
		.8086
	else
		.286c
	endif

