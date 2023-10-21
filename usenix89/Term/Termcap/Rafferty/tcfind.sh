:
#!/bin/sh
# print a requested termcap entry
# CSR 10 June 1986

# check for just one arg
if test $# -ne 1; then
    echo "usage: tcfind terminalname"
    exit 1
fi

#Look for lines with the first argument surrounded by 
#
#           |    |           -or-   |    $      -or-        ^    |
#
sed -n -e '/|'$1'|/,/[^\\]$/p' -e '/|'$1'$/,/[^\\]$/p' -e '/^'$1'|/,/[^\\]$/p' /etc/termcap
