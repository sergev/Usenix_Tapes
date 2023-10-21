  100 print "     [CLR]"
  110 input "baud";b$
  120 if left$(b$,2)="12"then poke 2637,0
  130 if left$(b$,1)="3" then poke 2637,6
  140 sys(2688)
