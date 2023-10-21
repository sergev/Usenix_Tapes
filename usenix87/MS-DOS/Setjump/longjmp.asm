PAGE	60,132
		
	TITLE	ROUTINES for use by the MSDOS Lisp interpreter  
	SUBTTL	Peter Ashwood-Smith for TANDY  2000 1985.
	NAME	LONGJMP

PAGE
	INCLUDE LM8086.MAC
PAGE

;	TEMPORARY STORAGE AND EQUATES
;	=============================


POFFS	 EQU	 6		  ; offset for lattice C L model.         



PAGE

	 PSEG	
;
;	 SYNOPSIS:   longjmp(longlabel,value)
;		     jmp_buf longlabel; 
;		     int value;
;
;		     longjmp will restore the SP and SS segments from
;		     the longlabel buffer and return. This has the 
;		     effect of skipping a whole pile of returns. Have
;		     a look at the Unix manuals if you do not know what
;		     these functions do.
;		     
;
; 				
	 PUBLIC   LONGJMP		    
LONGJMP  PROC	  FAR 						    
	 PUSH	  BP
	 MOV      BP,SP					
	 MOV	  AX,[BP+POFFS+2]		; AX = high bits of ptr
	 MOV	  ES,AX				; ES = high bits
	 MOV      DI,[BP+POFFS]       		; DI  = low bits of ptr
	 MOV	  SP,ES:[DI] 			; SP = *ptr
	 MOV	  AX,ES:[DI+2]
	 MOV	  SS,AX				
	 MOV	  BP,SP	
	 MOV	  AX,ES:[DI+4]			; AX = SETJMP return addr
	 MOV	  [BP+2],AX          		; put return addr in STACK
	 MOV	  AX,ES:[DI+6]
	 MOV	  [BP+4],AX
	 MOV	  AX,ES:[DI+8]			; SETJMP's BP value       
	 MOV	  [BP],AX			; put into stack for restore
	 MOV	  AX,ES:[DI+10]			; restore the DS value
	 MOV	  DS,AX				;   
	 MOV	  AX,ES:[DI+12]			; restore the ES value
	 MOV      ES,AX
	 MOV	  AX,1H          		; load AX with user return
	 POP	  BP  			   	; pop the SETJMP's BP push
	 RET					; do  the SETJMP's ret   
LONGJMP  ENDP

PAGE
;	 SYNOPSIS:   setjmp(longlabel)      
;		     jmp_buf longlabel; 
;		     int value;
;
;		     Setjmp will save the stack pointer, and other critical
;		     registers in the buffer provided so that longjmp
;		     can restore them to perform the long jump operation.
;		     the jmp_buf must be more than 10 integers wide.
;
; 				
	 PUBLIC   SETJMP		    
SETJMP   PROC	  FAR 						    
	 PUSH	  BP
	 MOV      BP,SP					
	 MOV      AX,ES				; save ES for later.
	 MOV	  CX,AX				; in CX
	 MOV	  AX,[BP+POFFS+2]		; load the pointer to buff
	 MOV	  ES,AX				; es high part of pointer 
	 MOV      DI,[BP+POFFS]       		; di low part of pointer 
	 MOV	  ES:[DI],SP			; put sp in buf[0]
	 MOV	  AX,SS				; get stack segment 'ss' 
	 MOV	  ES:[DI+2],AX			; put ss in buf[1]
	 MOV	  AX,[BP+2]			; get setjmp return addr 
	 MOV	  ES:[DI+4],AX  		; put return addr in buf[2] 
	 MOV	  AX,[BP+4]			; and buf[3] for long model
	 MOV	  ES:[DI+6],AX
	 MOV	  AX,[BP]			; get setjmp pop bp value
	 MOV	  ES:[DI+8],AX			; put in buf[3]              
 	 MOV	  AX,DS
 	 MOV	  ES:[DI+10],AX			; put away DS
	 MOV	  ES:[DI+12],CX			; put away ES
	 XOR	  AX,AX				; return 0 always
 	 POP	  BP			   
	 RET
SETJMP   ENDP

PAGE
;
;	 SYNOPSIS:   char _envget(i)  					    
;		     int  i;    					    
;
;	 EFFECT  :   Get the ith byte from the environment area.            
;		     used by the function 'getenv'.
;							
	 PUBLIC   _ENVGET		    ;  _ENVGET(I:INTEGER)  	    
         EXTRN	  _PSP:DWORD
_ENVGET  PROC	  FAR 						    
	 PUSH	  BP
	 MOV      BP,SP			
         PUSH     DS
	 MOV	  DI,[BP+POFFS]		    ;  load offset parameter 'i'
	 MOV      AX,_PSP+2  		    ;  AX = segment address of PSP 
 	 MOV      DS,AX			    ;  DS = &PSP
         MOV      AX,DS:[2CH]		    ;  AX = PSP[2CH] (env seg ptr)
 	 MOV      DS,AX			    ;  DS = env seg ptr
 	 MOV      AL,DS:[DI]	     	    ;  AL = ENV[i]
         XOR      AH,AH			    
         POP      DS
	 POP	  BP			   
	 RET
_ENVGET  ENDP

         ENDPS
	 END
