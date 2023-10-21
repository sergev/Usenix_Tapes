
    TinyTcp Public Domain Release

The files in this release contain a simple implementation of TCP & FTP,
suitable for burning into ROM.  It is, in effect, a big hack put together
in two or three days.  It works for us, though, and you might like it,
too.  We use it to boot our image processor by retrieving a load file
using the standard FTP server.

The code is intended to use the buffers from the Ethernet interface,
although shadow buffers could be hidden in the driver with no problem.
On one home-brew board here and didn't support byte-mode access to it.
Hence, the code takes pains to fool our local compiler into never
generating byte- or bit-mode instructions.

Since we have already burned the TCP into ROM, it is unlikely that
any further development will take place at IMAGEN.  I would be willing
to act as a clearinghouse for future improvements to the code.

Warning: the code was intended for a 68000, and doesn't have
any byte swapping support in it.  Shouldn't be too hard to add
if you want to run it on a little-endian machine.

Please note (and honor) the copyright on each file:

  Copyright (C) 1986, IMAGEN Corporation
  "This code may be duplicated in whole or in part provided that [1] there
   is no commercial gain involved in the duplication, and [2] that this
   copyright notice is preserved on all copies.  Any other duplication
   requires written notice of the author (Geoffrey H. Cooper)."

...in other words, do what you want with the code, but don't sell it and
give IMAGEN and me credit on whatever you produce.  If you develop a product
based on this code and want to sell it, give me a call and I'll be reasonable.

This code is distributed "as is." Neither the author nor IMAGEN Corporation
guarantees that it works or meets any particular specifications.  The act
of distributing this software does not represent a commitment by either the
author nor IMAGEN Corporation to provide maintenance or consulting services
related to this software.

But feel free to ask me any questions that you can't answer for yourself.

    - Geof Cooper
      Imagen Corporation
      (408)986 9400
      [imagen!geof@decwrl.dec.com, {decwrl,saber}!imagen!geof]
      April 16, 1986

The package requires some system support:

    clock_ValueRough() - should be a procedure that returns the current
        value of a millisecond clock.  The procedure is called frequently,
        so that interrupts are not needed to service the clock.  Our
        implementation polls the real time timer and assumes that it is
        called frequently enough so that it doesn't miss clock ticks (Since
        the timer is only used for network timeouts, it doesn't really matter
        if it does miss clock ticks, of course).  Systems without a clock
        could probably get by with a procedure that increments a static
        variable and returns it, by adjusting the timeout constants in the
        program.

    Network driver - some network interface  driver is needed.  A driver for a
        3Com multibus (ethernet) board is included; this board isn't made
        anymore (at least not by 3Com, there are still compatible boards
        being made, but I don't know of any being popularly distributed),
        so you'll probably need to write a driver for the board in your system.


Guide to source files:

    sed.c - Simple Ethernet Driver - Driver for 3Com multibus card.  If you
            have another type of Ethernet board, you can use this driver as
            a template.

    sed.h - header file for the above.

    arp.c - Implementation of Address Resolution Protocol.  Note that there
            is no arp "mapping" per se.  The higher level code (tcp, in this
            case) is required to keep track of internet and ethernet addresses.

    tinytcp.c - Implementation of TCP.

    tinytcp.h - Header file for above, and for everything else.

    tinyftp.c - Implementation of FTP, only allows files to be retrieved,
                not sent.
