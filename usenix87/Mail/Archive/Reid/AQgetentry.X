#! /bin/sh
#
# This script is invoked to process one request to the archive server
# 

PATH=#PATH#
export PATH

awk '
$1 == "PATH" {Path=$2; for(i=3;i<=NF;i++) Path = Path " " $i; next}
$1 == "ARGS" {Args=$2; for(i=3;i<=NF;i++) Args = Args " " $i; next}
$1 == "FROM" {From=$2; for(i=3;i<=NF;i++) From = From " " $i; next}
$1 == "VERB" {
   if ($2 == "help") {
       print | "#BIN#/AQhelp " "'\''" Path "'\'' " "'\''" From "'\''"
       next;
    }
   if ($2 == "reject") {
       print | "#BIN#/AQreject " "'\''" Path "'\'' " "'\''" From "'\''"
       next;
    }
   if ($2 == "index") {
       print | "#BIN#/AQindex " "'\''" Path "'\'' " "'\''" From "'\'' " Args
   }
}'
