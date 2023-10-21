
Test class Set
s = Set[
(1 @ 1)
(1 @ 3)
(1 @ 2)]

s.includes(C): 1
t == s: 1
s.includes(C): 0
t == s: 0
s = Set[
(1 @ 1)
(1 @ 4)
(1 @ 2)]

t = Set[
(1 @ 1)
(1 @ 3)
(1 @ 2)]

s&t = Set[
(1 @ 1)
(1 @ 2)]

s|t = Set[
(1 @ 1)
(1 @ 4)
(1 @ 3)
(1 @ 2)]

s-t = Set[
(1 @ 4)]

s.asOrderedCltn: OrderedCltn[
(1 @ 1)
(1 @ 4)
(1 @ 2)]

