$set noon
$define lnk$library sys$library:vaxccurse
$define lnk$library_1 sys$library:vaxcrtl
$cc DOORCOU.C
$cc help.c
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
$library/create  rogue  HIT
$library/replace rogue  help
$library/replace rogue  INIT
$library/replace rogue  INVENTOR
$library/replace rogue  doorcou
$library/replace rogue  IO
$library/replace rogue  LEVEL
$library/replace rogue  MESSAGE
$library/replace rogue  MONSTER
$library/replace rogue  MOVE
$library/replace rogue  OBJECT
$library/replace rogue  PACK
$library/replace rogue  PLAY
$library/replace rogue  RANDOM
$library/replace rogue  ROOM
$library/replace rogue  SCORE
$library/replace rogue  SPECIAL
$library/replace rogue  THROW
$library/replace rogue  USE
$library/replace rogue  ZAP
$cc      MAIN
$link main,rogue/lib
