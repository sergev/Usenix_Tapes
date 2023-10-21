>xref
ed xref <<EOF
r xrefhead.sh
/???usrlocal/p
s;;$1;p
\$r mergelines.awk
g/###/s///p
\$s/.*/&'/p
w
q
EOF
