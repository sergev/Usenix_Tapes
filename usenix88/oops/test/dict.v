
Test class Dictionary
d = Dictionary[
Assoc((1 @ 1)=(2 @ 1))
Assoc((1 @ 3)=(2 @ 3))
Assoc((1 @ 2)=(2 @ 2))]

d.atKey(Point(1,1)): (2 @ 1)
d = Dictionary[
Assoc((1 @ 1)=(3 @ 1))
Assoc((1 @ 3)=(2 @ 3))
Assoc((1 @ 2)=(2 @ 2))]

d.includesAssoc(c): 1
d.includesKey(*c.key()): 1
d.keyAtValue(Point(3,1)) = (1 @ 1)
d.includesAssoc(c): 0
d.includesKey(*c.key()): 0
d.asBag: Bag[
AssocInt((2 @ 2)=1)
AssocInt((3 @ 1)=1)]

d.addKeysTo(keys): OrderedCltn[
(1 @ 1)
(1 @ 2)]

d.addValuesTo(vals): OrderedCltn[
(3 @ 1)
(2 @ 2)]

