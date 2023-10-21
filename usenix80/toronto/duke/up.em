: : syntax:  x up <string> <command>
: : executes   ?<string>?<command>
: : however, it works correctly even if <string> contains a slash
> ~Htmp~K
ms.~1
>
!e - ~Htmp~K
gs,?,\\?,g
gs,^,?
gs,$,?u
wq
< ~Htmp~K
sh
.~A
!rm ~Htmp~K
