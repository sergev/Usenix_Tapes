$set noon
$define lnk$library sys$library:vaxccurse
$define lnk$library_1 sys$library:vaxcrtl
$cc DOORCOU.C
$cc HIT.C
$cc INIT.C
$cc INVENTOR.C
$cc IO.C
$cc LEVEL.C
$cc MESSAGE.C
$cc MONSTER.C
$cc MOVE.C
$cc OBJECT.C
$cc PACK.C
$cc PLAY.C
$cc RANDOM.C
$cc ROOM.C
$cc SCORE.C
$cc SPECIAL.C
$cc THROW.C
$cc USE.C
$cc ZAP.C
$library/create   HIT.C
$library/replace INIT.C
$library/replace INVENTOR.C
$library/replace IO.C
$library/replace LEVEL.C
$library/replace MESSAGE.C
$library/replace MONSTER.C
$library/replace MOVE.C
$library/replace OBJECT.C
$library/replace PACK.C
$library/replace PLAY.C
$library/replace RANDOM.C
$library/replace ROOM.C
$library/replace SCORE.C
$library/replace SPECIAL.C
$library/replace THROW.C
$library/replace USE.C
$library/replace ZAP.C
$cc      MAIN.C
$link main,rogue/lib
