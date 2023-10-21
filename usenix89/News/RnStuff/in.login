set today=`date`
set todays_newsrc=.newsrc.`echo $today | sed "s/^\(...\).*/\1/"`
ln `echo $todays_newsrc` .newsrc
alias rn ~/.my.rn
