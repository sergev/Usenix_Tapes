dgroup	group	data
data	segment byte public 'DATA'
data	ends

_prog	segment byte
	assume cs:_prog,ds:dgroup

; long time_bin()
;
; Function: return the current time as a long in binary format:
; top byte is hours (0-23), next is minutes (0-59), seconds (0-59),
; and hundreths (0-99).
;
	public time_bin
time_bin proc far
	mov	ah,2Ch		; get-time command
	int	21h		; BIOS request
	mov	bx,dx		; ss,hu
	mov	ax,cx		; hh,mm
	ret
time_bin endp

; long date_bin()
;
; Function:  return the current date as a long: top two bytes are year,
; followed by month and day in last two bytes.
;
	public date_bin
date_bin proc far
	mov	ah,2Ah		; get-date command
	int	21h		; BIOS request
	mov	bx,dx		; year
	mov	ax,cx		; month and day
	ret
date_bin endp

_prog	ends
	end
