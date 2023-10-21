/ some things not in v6 C compiler
.globl _dpdiv
_dpdiv:
	mov 2(sp), r0
	mov 4(sp), r1
	div 6(sp), r0
	rts pc
.globl _dpmul
_dpmul:
	mov 2(sp), r0
	clr r1
	mul 4(sp), r0
	rts pc
