
rem mktalk.bat: build talkcon.dev, talking device driver
masm reading;
masm events;
masm synth;
masm talkcon;
link talkcon+events+synth+reading;
exe2bin talkcon.exe talkcon.dev
del *.obj
del *.exe

