
Test class OrderedCltn
b = OrderedCltn[
(1 @ 1)
(1 @ 2)
(1 @ 3)
(1 @ 3)]

b.first(): (1 @ 1)
b.last(): (1 @ 3)
b = OrderedCltn[
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 3)]

b.after(Point(1,2)): (1 @ 21)
b.before(Point(1,2)): (1 @ 19)
b.includes(C): 1
c == b: 1
b = OrderedCltn[
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 3)
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 3)]

b.includes(C): 1
b = OrderedCltn[
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 3)]

c == b: 0
c&b:OrderedCltn[
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 3)
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 3)]

b.indexOfSubCollection(c): -1
c = OrderedCltn[
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 3)]

b.replaceFrom(1,3,c): OrderedCltn[
(1 @ 1)
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 3)
(1 @ 1)
(1 @ 19)
(1 @ 2)
(1 @ 21)
(1 @ 3)
(1 @ 3)]

b.sort(): OrderedCltn[
(1 @ 1)
(1 @ 1)
(1 @ 1)
(1 @ 2)
(1 @ 2)
(1 @ 3)
(1 @ 3)
(1 @ 3)
(1 @ 19)
(1 @ 19)
(1 @ 21)]

b.asSet(): Set[
(1 @ 1)
(1 @ 19)
(1 @ 3)
(1 @ 21)
(1 @ 2)]

