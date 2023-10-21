      subroutine cntr(iarin,iarout,n)
      character*1 iarin(80),iarout(80),itemp(80)
      do 1 i=1,80
   1  itemp(i)=iarin(i)
      ilst=80
      do 2 i=1,80
      if(itemp(i)  .ne.  1h   )  go to 3
    2 continue
      i=80
    3 ifst=i
      do 4 i=1,80
      if(itemp(81-i)  .ne.  1h  )  go to 5
    4 continue
      i=80
    5 ilst=81-i
      n=ilst-ifst+1
      if(n  .lt.  0)  n=0
      if(n  .eq.  0)  return
      do 6 i=ifst,ilst
      j=i-ifst+1
   6  iarout(j)=itemp(i)
      return
      end
