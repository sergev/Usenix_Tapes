echo off
cls
echo ���������������������������������������������������������ͻ
echo �                                                         �
echo �   Sample file for starting DOSamatic when DOS system    �
echo �   disk is in drive A and DOSamatic Disk is in drive B   �
echo �                                                         �
echo ���������������������������������������������������������ͼ
echo .
echo ���������������������������������������������������������ͻ
echo �                                                         �
echo �   Assumes Compaq system with Monochrome Graphics Screen �
echo �                                                         �
echo ���������������������������������������������������������ͼ
echo .
pause
set comspec=A:\COMMAND.COM
set DSH=B:\DOSAMATC.HLP
set DSD=A:\DEBUG.COM
set DSE=A:\EDLIN.COM
set DSV=MONO
DOSAMATC
