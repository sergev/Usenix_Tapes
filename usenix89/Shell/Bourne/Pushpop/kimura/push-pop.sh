
DIRSTAK=""

dirs() {
   echo `pwd` $DIRSTAK
}

pushdir() {
   if test $# -eq 0
   then
      if test "$DIRSTAK" = ""
      then
         echo "directory stack empty."
      else
	 OLDPWD=`pwd`
         popdir
         DIRSTAK="$OLDPWD $DIRSTAK"
      fi
   else
      while test "$*" != ""
      do
	 OLDPWD=`pwd`
         if cd "$1"
         then
            DIRSTAK="$OLDPWD $DIRSTAK"
         fi
         shift
      done
   fi
}

listhead() {
   echo $1
}

listtail() {
   shift
   echo $*
}

popdir() {
   if test $# -ne 0
   then
      echo "No parameters allowed with popdir"
   else
      if test "$DIRSTAK" = ""
      then
         echo "directory stack empty."
      else
         TMP1=`listhead $DIRSTAK`
         DIRSTAK=`listtail $DIRSTAK`
         if test "$TMP1" != ""
         then
            cd "$TMP1"
         fi
      fi
   fi
}

pushd() {
   if pushdir $*
   then
      dirs
   fi
}

popd() {
   if popdir $*
   then
      dirs
   fi

}
