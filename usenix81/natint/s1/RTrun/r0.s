/ RT simulator, low core module 0
/       two modules req'd to circumvent assembler bug
RMON= 0134000
PCLOC=040
SPLOC=042

	.globl _main, start
.= 0 ^ .
	jmp     start

.= .+060000-4
