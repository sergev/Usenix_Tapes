BEGIN   { STARTED = "FALSE"}

$1=="AFM" &&  $2=="FILE" \
        { AFM=$3; STARTED = "TRUE"; next;}

$1=="StartFontMetrics", $1=="EndFontMetrics" \
        { print $0 >AFM }

/END OF FONT METRICS/ && STARTED == "TRUE" \
        { exit }
