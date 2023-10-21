
Test class Bag
b = Bag[
AssocInt((1 @ 1)=1)
AssocInt((1 @ 3)=2)
AssocInt((1 @ 2)=1)]

b.asArrayOb: ArrayOb[
(1 @ 1)
(1 @ 3)
(1 @ 3)
(1 @ 2)]

b.includes(C): 1
c == b: 1
b.includes(C): 1
b = Bag[
AssocInt((1 @ 1)=1)
AssocInt((1 @ 3)=1)
AssocInt((1 @ 2)=1)]

b.includes(C): 0
b = Bag[
AssocInt((1 @ 1)=1)
AssocInt((1 @ 2)=1)]

c == b: 0
