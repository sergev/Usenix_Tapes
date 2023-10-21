#! /bin/sh

echo Doing library
X="uu.cim_change.o uu.cim_size.o uu.csr_change.o uu.csr_init.o"
X="$X uu.ttysw_main.o uu.ttyvt100.o uu.vt100fonts.o uu.vt100keys.o"
cd libdir
for i in $X ; do
    uudecode $i
    rm $i
done

X="uu.gacha.b.8 uu.gacha.bg.8 uu.gacha.g.8 uu.gacha.r.8 uu.testfont.sh"
X="$X uu.thin.b.5 uu.thin.b.6 uu.thin.bg.5 uu.thin.bg.6 uu.thin.g.5"
X="$X uu.thin.g.6 uu.thin.r.5 uu.thin.r.6 uu.thinbot.b.10 uu.thinbot.bg.10"
X="$X uu.thinbot.g.10 uu.thinbot.r.10 uu.thintop.b.10 uu.thintop.bg.10"
X="$X uu.thintop.g.10 uu.thintop.r.10 uu.thinwide.b.10 uu.thinwide.bg.10"
X="$X uu.thinwide.g.10 uu.thinwide.r.10 uu.wide.b.16 uu.wide.bg.16"
X="$X uu.wide.g.16 uu.wide.r.16 uu.widebot.b.16 uu.widebot.bg.16"
X="$X uu.widebot.g.16 uu.widebot.r.16 uu.widetop.b.16 uu.widetop.bg.16"
X="$X uu.widetop.g.16 uu.widetop.r.16"

echo Doing fonts
cd fontdir
for i in $X ; do
    uudecode $i
    rm $i
done

exit 0
