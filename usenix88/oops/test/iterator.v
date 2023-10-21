
Test class Iterator
OrderedCltn[
(0 @ 0)
(1 @ 1)
(2 @ 2)
(3 @ 3)
(4 @ 4)
(5 @ 5)]
Set[
(0 @ 0)
(5 @ 5)
(4 @ 4)
(3 @ 3)
(2 @ 2)
(1 @ 1)]
LinkedList[
LinkOb((0 @ 0))
LinkOb((5 @ 5))
LinkOb((4 @ 4))
LinkOb((3 @ 3))
LinkOb((2 @ 2))
LinkOb((1 @ 1))]

Test Iterator(OrderedCltn).storeOn()
Iterator(OrderedCltn[0]#0): (0 @ 0)(1 @ 1)(2 @ 2) Iterator(OrderedCltn[3]#0)
Test Iterator(OrderedCltn) readFrom()
Iterator(OrderedCltn[3]#0): (3 @ 3)(4 @ 4)(5 @ 5)
Test Iterator(OrderedCltn).deepCopy()
Iterator(OrderedCltn[3]#0): (3 @ 3)(4 @ 4)(5 @ 5)
Test Iterator(OrderedCltn).reset()
Iterator(OrderedCltn[0]#0): (0 @ 0)(1 @ 1)(2 @ 2)(3 @ 3)(4 @ 4)(5 @ 5)

Test Iterator(LinkedList).storeOn()
Iterator(LinkedList[0]#0): LinkOb((0 @ 0))LinkOb((5 @ 5))LinkOb((4 @ 4)) Iterator(LinkedList[3]#0)
Test Iterator(LinkedList) readFrom()
Iterator(LinkedList[3]#0): LinkOb((3 @ 3))LinkOb((2 @ 2))LinkOb((1 @ 1))
Test Iterator(LinkedList).deepCopy()
Iterator(LinkedList[3]#0): LinkOb((3 @ 3))LinkOb((2 @ 2))LinkOb((1 @ 1))
Test Iterator(LinkedList).reset()
Iterator(LinkedList[0]#0): LinkOb((0 @ 0))LinkOb((5 @ 5))LinkOb((4 @ 4))LinkOb((3 @ 3))LinkOb((2 @ 2))LinkOb((1 @ 1))
