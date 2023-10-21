masm abortc.asm /a /i..\INC, obj\abortc.obj; 
masm cio_ctrl.asm /a /i..\INC, obj\cio_ctrl.obj; 
masm p_aplint.asm /a /i..\INC, obj\p_aplint.obj; 
lc -i..\inc\ -oobj\ -n -v *.c
link @app.lnk,app,nul,\lib\lcs.lib; 
