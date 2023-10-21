!--> file DEF1.COM   ---------- ---------- ---------- --------->
b        :== show queue sys$batch
q        :== show queue sys$print
p        :== print/feed/header/noflag
rmjob    :== del sys$batch/entry=
rmpri    :== del sys$print/entry=
e        :== edit/edt/command=bin:edtini.edt
pwd      :== sho def
sd       :== set def
cpu      :== monitor processes/topcpu
l        :== dir
ls       :== dir
lr       :== dir [...]
ll       :== dir/date/size
llr      :== dir/date/size [...]
llp      :== dir/date/size/prot
sv       :== set verify
sn       :== set noverify
st       :== sho term
stt      :== set term/dev=vt100
who      :== sho u
cp       :== copy
mv       :== rename
dl       :== delete
sym      :== sho sym
log      :== sho log
k        :== sho time
pro      :== sho prot
sys      :== sho syst
chmod    :== set prot/prot=(system:wred, owner:wred, group:re, world:re)
chmid    :== set prot/prot=(system:re,   owner:re,   group:re, world:re)
nomod    :== set prot/prot=(system:wred, owner:wred, group:, world:)
!<-- end  ---------- ---------- ---------- ---------- <---------
