echo off
if exist make$$$$.bat del make$$$$.bat
xmake %1 %2 %3 %4 %5 %6 %7 %8 %9
if exist make$$$$.bat make$$$$.bat
