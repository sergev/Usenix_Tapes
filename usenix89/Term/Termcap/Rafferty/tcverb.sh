:
#! /bin/sh
# add commentary to termcap entry provided on stdin
# CSR 10 June 1986

# add linefeeds to colons
sed 's/:/:\
/g' |

# blow away lines with just space, colon or \
#             \t
sed '/^[\\ 	:]*$/d' |

# make shell commands
#                                                      \t     \t
sed 's!^\(..\)\(.*\)!echo '\''\1\2'\'' `sed -n "s/^\1.*	\\([^	]*\\)$/    \\1/p" /usr/man/man5/termcap.5`!' |

# execute 'em
sh
