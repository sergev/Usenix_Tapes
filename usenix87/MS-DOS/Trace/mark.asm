;_ mark.asm   Sun Jul 13 1986 */
;MARK.ASM - mark a position in memory,
;           above which TSRs will later be cleared by RELEASE.PAS/COM
;           MARK can be called multiple times, each RELEASE will clear
;           above the last MARK called
;
; written for CHASM (CHeap ASseMbler)
; by Kim Kokkonen, TurboPower Software
; telephone: 408-378-3672, Compuserve 72457,2131
;
cseg	segment
assume	cs:cseg,ds:cseg
org	100h
mark   proc    near
       jmp     install

idstr	db	"MARK PARAMETER BLOCK FOLLOWS" ;used to find this TSR
dummy	db      0		;puts vector table on an even paragraph boundary
vector	db	400H dup(0)	;holds vector table (0..FF)*4 at invocation

;store the interrupt vector table
install:
       push    ds
       cli                     ;interrupts of
       cld                     ;copy up
       mov     cx,200H         ;512 integers to store
       xor     ax,ax
       mov     ds,ax           ;source address segment 0
       xor     si,si           ;offset 0
       mov     di,offset vector	;destination offset, es=cs already
       rep movsw		;copy vectors to our table
       sti			;interrupts on

;print message and TSR
       pop     ds
       mov     dx,offset didit	;get end of code
       mov     ah,9
       int     21H             ;write success message
       mov     cx,4
       shr     dx,cl           ;convert to paragraphs
       inc     dx              ;round up
       mov     ax,3100H
       int     21H             ;terminate and stay resident

;used to mark end of this TSR
didit  db      13,10,'Marked current memory position',13,10,36
mark	endp
cseg	ends
	end	mark

