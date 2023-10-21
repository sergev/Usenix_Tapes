
MAKEMAKE.PRG	: MakeLibr.LNK MakeToke.LNK MakePars.LNK MakeFore.LNK  \
  CommandL.LNK MAKEMAKE.LNK 
	C:\BIN\LNK.PRG MAKEMAKE.LNK
MAKEMAKE.LNK	: MAKEMAKE.MOD MakePars.SYM MakeFore.SYM CommandL.SYM
  #libs:   Strings.SYM Streams.SYM ASCII.SYM InOut.SYM
	C:\BIN\MOD.PRG MAKEMAKE.MOD
MakeLibr.LNK	: MakeLibr.MOD MakeLibr.SYM
  #libs:   Strings.SYM
	C:\BIN\MOD.PRG MakeLibr.MOD
MakeLibr.SYM	: MakeLibr.DEF
	C:\BIN\MOD.PRG MakeLibr.DEF
MakeToke.LNK	: MakeToke.MOD MakeToke.SYM
  #libs:   Streams.SYM
	C:\BIN\MOD.PRG MakeToke.MOD
MakeToke.SYM	: MakeToke.DEF
	C:\BIN\MOD.PRG MakeToke.DEF
MakePars.LNK	: MakePars.MOD MakePars.SYM MakeLibr.SYM MakeFore.SYM \
   MakeToke.SYM
  #libs:   Strings.SYM
	C:\BIN\MOD.PRG MakePars.MOD
MakePars.SYM	: MakePars.DEF
	C:\BIN\MOD.PRG MakePars.DEF
MakeFore.LNK	: MakeFore.MOD MakeFore.SYM
  #libs:   Storage.SYM Strings.SYM
	C:\BIN\MOD.PRG MakeFore.MOD
MakeFore.SYM	: MakeFore.DEF
	C:\BIN\MOD.PRG MakeFore.DEF
