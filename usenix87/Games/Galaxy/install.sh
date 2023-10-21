
#       This is the galaxy maker.
# It should be run: /bin/sh install.sh
# Be sure to set the following shell-variables
# before you run this command file.
# Since this file has to use super-user privileges,
# make sure you read it BEFORE using it.
# Good luck.

SDIR=/usr/src/Games/Galaxy
#SDIR=/users/guest/galaxy/distr
# Set this to reflect the home directory of galaxy.
# All the rest resides in subdirectories.
SOURCES=$SDIR/src
SOURCES1=$SDIR/src1
MAN=$SDIR/man
DOC=$SDIR/doc
ONLINE=$SDIR/online
# This are system variables.
# The first should be carefully matched with variables set in
# $SDIR/src/constants.h!!!
LIBDIR=/usr/games/lib/galaxy
SYSMAN6=/usr/man/local/man6
SYSDOC=/usr/doc

cat << Funny
Warning: Read this file before running it!

Warning: This file must be run with super_user priviliges.
         Before running it , create a user named galaxy,
         group - games.

Shall I proceed ?
Funny
read answer
if [ "$answer" != y ]; then
        echo GoodBye
        exit 1
fi

if [ ! -d $LIBDIR ]; then
        mkdir $LIBDIR
        chown galaxy $LIBDIR
fi

echo 'Making the galaxy game'
cd $SOURCES ; make galaxy                       # install the main game
echo 'Installing the galaxy game'
cd $SOURCES ; make install
echo 'Making the utility files'
cd $SOURCES1 ; make all                         # install utility files
echo 'Installing the utility files'
cd $SOURCES1 ; make install

echo 'Checking/creating online directory'
if [ ! -d $LIBDIR/online ]; then
        mkdir $LIBDIR/online
fi

chown galaxy $LIBDIR/online
echo 'Copying online files'
cp $ONLINE/* $LIBDIR/online
chown galaxy $LIBDIR/online/*                   # install online documents
cd $LIBDIR
echo 'Creating the "score" and "wizards" files'
echo > galaxy.scor                              # score file.
chown galaxy galaxy.scor
chmod 600 galaxy.scor
echo mrdch > wizards                            # wizard file.
chmod 644 wizards                               # I'm always...
chown galaxy wizards
echo 'Placing the manual page'
cp $MAN/galaxy.6 $SYSMAN6/galaxy.6              # manual
echo 'Placing the document'
cp $DOC/Galaxy.doc $SYSDOC/galaxy               # document
