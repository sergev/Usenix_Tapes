: plot uucp map on graphics gizmo
MAPDIR=/usr/lib/uucp/map
grep '^#L' $MAPDIR/u.usa.* | sed -f uplot.sed | sort | uplot
