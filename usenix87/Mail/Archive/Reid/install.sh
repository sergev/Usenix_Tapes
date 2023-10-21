#! /bin/sh
#
# installs the archive server files
#

(mkdir /udir/recipes; cd /udir/recipes; mkdir /udir/recipes/requestqueue; mkdir /udir/recipes/workqueue; mkdir /udir/recipes/bin)
cp tmpdir.tmp/* /udir/recipes/bin
cp date+ /udir/recipes/bin
mv /udir/recipes/bin/copyright /udir/recipes
touch /udir/recipes/requestqueue/in.log
touch /udir/recipes/requestqueue/out.log
/etc/chown daemon /udir/recipes/requestqueue/in.log /udir/recipes/requestqueue/out.log
chmod 664 /udir/recipes/requestqueue/in.log /udir/recipes/requestqueue/out.log
echo 0 > /udir/recipes/workqueue/seq
