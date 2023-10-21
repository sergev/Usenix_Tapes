echo off
cls
echo ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
echo บ                                                         บ
echo บ   Sample file for starting DOSamatic when DOS system    บ
echo บ   disk is in drive A and DOSamatic Disk is in drive B   บ
echo บ                                                         บ
echo ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
echo .
echo ษอออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
echo บ                                                         บ
echo บ   Assumes Compaq system with Monochrome Graphics Screen บ
echo บ                                                         บ
echo ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
echo .
pause
set comspec=A:\COMMAND.COM
set DSH=B:\DOSAMATC.HLP
set DSD=A:\DEBUG.COM
set DSE=A:\EDLIN.COM
set DSV=MONO
DOSAMATC
