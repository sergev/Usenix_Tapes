lc -i..\inc\ -oobj\ -n -v pkt\*.c
lc -i..\inc\ -oobj\ -n -v frm\*.c
lc -i..\inc\ -oobj\ -n -v pad\*.c
lc -i..\inc\ -oobj\ -n -v xys\*.c
masm /a /i..\inc XYS\P_ASMUTL,OBJ\P_ASMUTL;
masm /a /i..\inc FRM\P_COMINT,OBJ\P_COMINT;
masm /a /i..\inc FRM\P_COMUTL,OBJ\P_COMUTL;
masm /a /i..\inc XYS\P_DRVINT,OBJ\P_DRVINT;
masm /a /i..\inc XYS\P_INTRUP,OBJ\P_INTRUP;
masm /a /i..\inc FRM\P_LNKMDM,OBJ\P_LNKMDM;
masm /a /i..\inc XYS\P_TIMINT,OBJ\P_TIMINT;
masm /a /i..\inc XYS\P_XPCDRV,OBJ\P_XPCDRV;
link @xpc.lnk,xpc,nul;
exe2bin xpc.exe xpc.com
del xpc.exe
