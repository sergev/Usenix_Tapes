SRCS = :heading :monsters
OBJS = monsters.alpha monsters.exper monsters.treas

default: $(OBJS)

monsters.alpha:
	( cat :heading ; sort -f :monsters ) > $@
monsters.exper:
	( cat :heading ; sort -n -r -t\	 +5 -6 :monsters ) > $@
monsters.treas:
	( cat :heading ; sort -n -t\	 +6 -7 :monsters ) > $@
