/ default filename for fsboot2.s
/ will get assembly error here if boot is too big.
/ this must be the last file assembled.
. = 760^.
bootf:
	<unix\0\0\0\0\0\0\0\0\0\0\0\0>	/ 16 char zero fill
