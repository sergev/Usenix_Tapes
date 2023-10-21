*** ifconfig.8c.orig	Sun Apr  6 14:43:39 1986
--- ifconfig.8c	Sun Apr  6 14:43:41 1986
***************
*** 101,106 ****
--- 101,119 ----
  .B \-arp
  Disable the use of the Address Resolution Protocol.
  .TP 15
+ .B subarp
+ Enable an interface on a subnet gateway to respond to ARP requests 
+ for hosts reachable via this subnet.  
+ Only useful on hardware which supports ARP.
+ This allows a gateway between two subnets to respond to ARP requests
+ received on an enabled interface to respond for hosts on another
+ enabled interface and route packets thru the gateway.
+ This allows a network configuration where only the gateway hosts need to
+ be aware of the existence of subnets.
+ .TP 15
+ .B \-subarp
+ Disable the ARP subnet code.
+ .TP 15
  .BI metric " n"
  Set the routing metric of the interface to
  .IR n ,
