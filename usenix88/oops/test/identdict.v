
Test class IdentDict
d = IdentDict[
AssocInt((1 @ 1)=1)
Assoc((1 @ 2)=(2 @ 2))
AssocInt((1 @ 3)=3)]

d.atKey(a): 1
d.includesKey(Point(1,1)): 0
d = IdentDict[
AssocInt((1 @ 1)=0)
Assoc((1 @ 2)=(2 @ 2))
AssocInt((1 @ 3)=3)]

d.includesAssoc(asc): 1
d.includesKey((1 @ 3)): 1
d.keyAtValue(Integer(0)) = (1 @ 1)
d.includesAssoc(asc): 0
d.includesKey((1 @ 3)): 0
d.asBag: Bag[
AssocInt(0=1)
AssocInt((2 @ 2)=1)]

d.addKeysTo(keys): OrderedCltn[
(1 @ 1)
(1 @ 2)]

d.addValuesTo(vals): OrderedCltn[
0
(2 @ 2)]

