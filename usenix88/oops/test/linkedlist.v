
Test class LinkedList
b = LinkedList[
LinkOb((1 @ 1))
LinkOb((1 @ 2))
LinkOb((1 @ 3))
LinkOb((1 @ 3))]

b.first(): LinkOb((1 @ 1))
b.last(): LinkOb((1 @ 3))
b.at(3): LinkOb((1 @ 3))
b.includes(C): 1
c == b: 1
b = LinkedList[
LinkOb((1 @ 0))
LinkOb((1 @ 1))
LinkOb((1 @ 2))
LinkOb((1 @ 3))
LinkOb((1 @ 3))
LinkOb((1 @ 19))]

b = LinkedList[
LinkOb((1 @ 0))
LinkOb((1 @ 1))
LinkOb((1 @ 2))
LinkOb((1 @ 3))
LinkOb((1 @ 3))
LinkOb((1 @ 19))
LinkOb((1 @ 1))
LinkOb((1 @ 2))
LinkOb((1 @ 3))
LinkOb((1 @ 3))]

b.includes(A): 1
b = LinkedList[
LinkOb((1 @ 0))
LinkOb((1 @ 2))
LinkOb((1 @ 3))
LinkOb((1 @ 3))
LinkOb((1 @ 19))
LinkOb((1 @ 1))
LinkOb((1 @ 2))
LinkOb((1 @ 3))
LinkOb((1 @ 3))]

c == b: 0
b.indexOf(D): 2
c = LinkedList[
LinkOb((1 @ 1))
LinkOb((1 @ 2))
LinkOb((1 @ 3))
LinkOb((1 @ 3))]

LinkOb((1 @ 1))LinkOb((1 @ 2))LinkOb((1 @ 3))LinkOb((1 @ 3))
c.size() = 4
c = LinkedList[
LinkOb((1 @ 2))
LinkOb((1 @ 3))
LinkOb((1 @ 3))]

c = LinkedList[
LinkOb((1 @ 3))
LinkOb((1 @ 3))]

c = LinkedList[
LinkOb((1 @ 3))]

c = LinkedList[
]

b.asSet(): Set[
LinkOb((1 @ 1))
LinkOb((1 @ 19))
LinkOb((1 @ 3))
LinkOb((1 @ 0))
LinkOb((1 @ 2))]

