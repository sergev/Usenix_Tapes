ckwart %1.w %1.c
cp68 %1.c %1.i
rm %1.c
c068 %1.i %1.1 %1.2 %1.3 -f
rm %1.i
c168 %1.1 %1.2 %1.s
rm %1.1
rm %1.2
as68 -l -u %1.s
rm %1.s
link68 [co[%1.inp]]
relmod %1
rm %1.68K
wait.prg
