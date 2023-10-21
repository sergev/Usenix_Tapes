	.set MEMSIZ,0x10000
	.text
	.globl _wordspace
	.globl _bytespace
# set asside pdp11 memory space in nonprotected text segment at loc 0
_wordspace:
_bytespace:
	.space MEMSIZ
# put the memory size in a global variable for other uses
	.globl _memsiz
_memsiz:.long _memsiz
