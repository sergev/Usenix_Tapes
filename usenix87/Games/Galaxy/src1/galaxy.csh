#! /bin/csh -f
#
# %W% (mrdch&amnnon) %E%
#

set motd = "/usr/games/lib/galaxy/galaxy.motd"

if ( -e $motd && $#argv == 0) cat $motd

/usr/games/lib/galaxy/galaxy.out $argv
