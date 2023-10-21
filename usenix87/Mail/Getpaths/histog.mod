#! /bin/sh
awk -F! '{ count[NF] += 1; if (NF > mx) mx = NF; sum += NF; }
END {
    printf "Path length statistics\nAverage length: %f\nHistogram:\n", sum / NR;
    for (i = 1; i <= mx; i++) printf ("%d %d\n",i,count[i]);
}'
