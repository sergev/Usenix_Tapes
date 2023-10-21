|-----------------------------------------------------------
|
|  these macros give assembly language access to the mac
|  floating-point arithmetic routines.  the arithmetic has
|  just one entry point.  it is typically accessed through
|  the tooltrap _fp68k, although a custom version of the
|  package may be linked as an object file, in which case
|  the entry point is the label %fp68k.
|
|  all calls to the arithmetic take the form:
|       pea     <source address>
|       pea     <destination address>
|       movw    <opcode>,sp@-
|       .word	_fp68k
|
|  all operands are passed by address.  the <opcode> word
|  specifies the instruction analogously to a 68000 machine
|  instruction.  depending on the instruction, there may be
|  from one to three operand addresses passed.
|
|  this definition file specifies details of the <opcode>
|  word and the floating point state word, and defines
|  some handy macros.
|
|  modification history:
|       29aug82: written by jerome coonen
|       13oct82: fbword	____ constrants added (jtc)
|       28dec82: logb, scalb added, inf modes out (jtc).
|       29apr83: abs, neg, cpysgn, class added (jtc).
|       03may83: next, setxcp added (jtc).
|       28may83: elementary functions added (jtc).
|       04jul83: short branches, trig and rand added (jtc).
|       01nov83: precision control made a mode (jtc).
|
|-----------------------------------------------------------



|-----------------------------------------------------------
| operation masks: bits /001f of the operation word
| determine the operation.  there are two rough classes of
| operations:  even numbered opcodes are the usual
| arithmetic operations and odd numbered opcodes are non-
| arithmetic or utility operations.
|-----------------------------------------------------------
foadd           =       /0000
fosub           =       /0002
fomul           =       /0004
fodiv           =       /0006
focmp           =       /0008
focpx           =       /000a
forem           =       /000c
foz2x           =       /000e
fox2z           =       /0010
fosqrt          =       /0012
forti           =       /0014
fotti           =       /0016
foscalb         =       /0018
fologb          =       /001a
foclass         =       /001c
| undefined     =       /001e

fosetenv        =       /0001
fogetenv        =       /0003
fosettv         =       /0005
fogettv         =       /0007
fod2b           =       /0009
fob2d           =       /000b
foneg           =       /000d
foabs           =       /000f
focpysgnx       =       /0011
fonext          =       /0013
fosetxcp        =       /0015
foprocentry     =       /0017
foprocexit      =       /0019
fotestxcp       =       /001b
| undefined     =       /001d
| undefined     =       /001f


|-----------------------------------------------------------
| operand format masks: bits /3800 determine the format of
| any non-extended operand.
|-----------------------------------------------------------
ffext           =       /0000   | extended -- 80-bit float
ffdbl           =       /0800   | double   -- 64-bit float
ffsgl           =       /1000   | single   -- 32-bit float
ffint           =       /2000   | integer  -- 16-bit integer
fflng           =       /2800   | long int -- 32-bit integer
ffcomp          =       /3000   | accounting -- 64-bit int


|-----------------------------------------------------------
| bit indexes for error and halt bits and rounding modes in
| the state word.  the word is broken down as:
|
|       /8000 -- unused
|
|       /6000 -- rounding modes
|                /0000 -- to nearest
|                /2000 -- toward +infinity
|                /4000 -- toward -infinity
|                /6000 -- toward zero
|
|       /1f00 -- error flags
|                /1000 -- inexact
|                /0800 -- division by zero
|                /0400 -- overflow
|                /0200 -- underflow
|                /0100 -- invalid operation
|
|       /0080 -- result of last rounding
|                /0000 -- rounded down in magnitude
|                /0080 -- rounded up in magnitude
|
|       /0060 -- precision control
|                /0000 -- extended
|                /0020 -- double
|                /0040 -- single
|                /0060 -- illegal
|
|       /001f -- halt enables, corresponding to error flags
|
| the bit indexes are based on the byte halves of the state
| word.
|-----------------------------------------------------------
fbinvalid       =       0       | invalid operation
fbuflow         =       1       | underflow
fboflow         =       2       | overflow
fbdivzer        =       3       | division by zero
fbinexact       =       4       | inexact
fbrndlo         =       5       | low bit of rounding mode
fbrndhi         =       6       | high bit of rounding mode
fblstrnd        =       7       | last round result bit
fbdbl           =       5       | double precision control
fbsgl           =       6       | single precision control


|-----------------------------------------------------------
| floating conditional branches: floating point comparisons
| set the cpu condition code register (the ccr) as follows:
|       relation        x n z v c
|       -------------------------
|         equal         0 0 1 0 0
|       less than       1 1 0 0 1
|       greater than    0 0 0 0 0
|        unordered      0 0 0 1 0
| the macros below define a set of so-called floating
| branches to spare the programmer repeated refernces to the
| the table above.
|-----------------------------------------------------------
        .macro  fbeq,l1
        beq     l1
        .endm

        .macro  fblt,l1
        bcs     l1
        .endm

        .macro  fble,l1
        bls     l1
        .endm

        .macro  fbgt,l1
        bgt     l1
        .endm

        .macro  fbge,l1
        bge     l1
        .endm

        .macro  fbult,l1
        blt     l1
        .endm

        .macro  fbule,l1
        ble     l1
        .endm

        .macro  fbugt,l1
        bhi     l1
        .endm

        .macro  fbuge,l1
        bcc     l1
        .endm

        .macro  fbu,l1
        bvs     l1
        .endm

        .macro  fbo,l1
        bvc     l1
        .endm

        .macro  fbne,l1
        bne     l1
        .endm

        .macro  fbue,l1
        beq     l1
        bvs     l1
        .endm

        .macro  fblg,l1
        bne     l1
        bvc     l1
        .endm

|-----------------------------------------------------------
| operation macros: (changed to defines for UNIX package).
|       these macros require that the operands' addresses
|       first be pushed on the stack.  the macros cannot
|       themselves push the addresses since the addresses
|       may be sp-relative, in which case they require
|       programmer care.
| operation macros: operand addresses should already be on
| the stack, with the destination address on top.  the
| suffix x, d, s, or c determines the format of the source
| operand -- extended, double, single, or computational
| respectively; the destination operand is always extended.
|-----------------------------------------------------------


|-----------------------------------------------------------
| addition.
|-----------------------------------------------------------
faddx = ffext+foadd

faddd = ffdbl+foadd

fadds = ffsgl+foadd

faddc = ffcomp+foadd


|-----------------------------------------------------------
| subtraction.
|-----------------------------------------------------------
fsubx = ffext+fosub

fsubd = ffdbl+fosub

fsubs = ffsgl+fosub

fsubc = ffcomp+fosub


|-----------------------------------------------------------
| multiplication.
|-----------------------------------------------------------
fmulx = ffext+fomul

fmuld = ffdbl+fomul

fmuls = ffsgl+fomul

fmulc = ffcomp+fomul


|-----------------------------------------------------------
| division.
|-----------------------------------------------------------
fdivx = ffext+fodiv

fdivd = ffdbl+fodiv

fdivs = ffsgl+fodiv

fdivc = ffcomp+fodiv


|-----------------------------------------------------------
| compare, signaling no exceptions.
|-----------------------------------------------------------
fcmpx = ffext+focmp

fcmpd = ffdbl+focmp

fcmps = ffsgl+focmp

fcmpc = ffcomp+focmp


|-----------------------------------------------------------
| compare, signaling invalid operation if the two operands
| are unordered.
|-----------------------------------------------------------
fcpxx = ffext+focpx

fcpxd = ffdbl+focpx

fcpxs = ffsgl+focpx

fcpxc = ffcomp+focpx


|-----------------------------------------------------------
| remainder.  the remainder is placed in the destination,
| and the low bits of the integer quotient are placed in
| the low word of register d0.
|-----------------------------------------------------------
fremx = ffext+forem

fremd = ffdbl+forem

frems = ffsgl+forem

fremc = ffcomp+forem


|-----------------------------------------------------------
| compare the source operand to the extended format and
| place in the destination.
|-----------------------------------------------------------
fx2x = ffext+foz2x

fd2x = ffdbl+foz2x

fs2x = ffsgl+foz2x

fi2x = ffint+foz2x

fl2x = fflng+foz2x

fc2x = ffcomp+foz2x


|-----------------------------------------------------------
| convert the extended source operand to the specified
| format and place in the destination.
|-----------------------------------------------------------
fx2d = ffdbl+fox2z

fx2s = ffsgl+fox2z

fx2i = ffint+fox2z

fx2l = fflng+fox2z

fx2c = ffcomp+fox2z


|-----------------------------------------------------------
| miscellaneous operations applying only to extended
| operands.  the input operand is overwritten with the
| computed result.
|-----------------------------------------------------------

| square root.
fsqrtx = fosqrt

| round to integer, according to the current rounding mode.
frintx = forti

| round to integer, forcing rounding toward zero.
ftintx = fotti

| set the destination to the product:
|  (destination) * 2^(source)
| where the source operand is a 16-bit integer.
fscalbx = ffint+foscalb

| replace the destination with its exponent, converted to
| the extended format.
flogbx = fologb


|-----------------------------------------------------------
| non-arithmetic sign operations on extended operands.
|-----------------------------------------------------------

| negate.
fnegx = foneg

| absolute value.
fabsx = foabs

| copy the sign of the destination operand onto the sign of
| the source operand.  note that the source operand is
| modified.
fcpysgnx = focpysgnx


|-----------------------------------------------------------
| the nextafter operation replaces the source operand with
| its nearest representable neighbor in the direction of the
| destination operand.  note that both operands are of the
| the same format, as specified by the usual suffix.
|-----------------------------------------------------------
fnexts = ffsgl+fonext

fnextd = ffdbl+fonext

fnextx = ffext+fonext


|-----------------------------------------------------------
| the classify operation places an integer in the
| destination.  the sign of the integer is the sign of the
| source.  the magnitude is determined by the value of the
| source, as indicated by the equates.
|-----------------------------------------------------------
fcsnan           =      1       | signaling nan
fcqnan           =      2       | quiet nan
fcinf            =      3       | infinity
fczero           =      4       | zero
fcnorm           =      5       | normal number
fcdenorm         =      6       | denormal number

fclasss = ffsgl+foclass

fclassd = ffdbl+foclass

fclassx = ffext+foclass

fclassc = ffcomp+foclass


|-----------------------------------------------------------
| these four operations give access to the floating point
| state (or environment) word and the halt vector address.
| the sole input operand is a pointer to the word or address
| to be placed into the arithmetic state area or read from
| it.
|-----------------------------------------------------------
fgetenv = fogetenv

fsetenv = fosetenv

fgettv = fogettv

fsettv = fosettv

|-----------------------------------------------------------
| both fprocentry and fprocexit have one operand -- a
| pointer to a word.  the entry procedure saves the current
| floating point state in that word and resets the state
| to 0, that is all modes to default, flags and halts to
| off.  the exit procedure performs the sequence:
|       1. save current error flags in a temporary.
|       2. restore the state saved at the address given by
|               the parameter.
|       3. signal the exceptions flagged in the temporary,
|               halting if so specified by the newly
|               restored state word.
| these routines serve to handle the state word dynamically
| across subroutine calls.
|-----------------------------------------------------------
fprocentry = foprocentry

fprocexit = foprocexit


|-----------------------------------------------------------
| fsetxcp is a null arithmetic operation which stimulates
| the indicated exception.  it may be used by library
| routines intended to behave like elementary operations.
| the operand is a pointer to an integer taking any value
| between fbinvalid and fbinexact.
| ftestxcp tests the flag indicated by the integer pointed
| to by the input address.  the integer is replaced by a
| pascal boolean (word /0000=false, $0100=true)
|-----------------------------------------------------------
fsetxcp = fosetxcp

ftestxcp = fotestxcp


|-----------------------------------------------------------
| warning: pascal enumerated types, like those of the
| decimal record, are stored in the high-order byte of the
| allocated word, if possible.  thus the sign has the
| integer value 0 for plus and 256 (rather than 1)
| for minus.
| binary-decimal conversion:  the next routines convert
| between a canonical decimal format and the binary format
| specified.  the decimal format is defined in pascal as
|
|   const
|       sigdiglen = 20;
|
|   type
|       sigdig  = string [sigdiglen];
|       decimal = record
|                     sgn : 0..1;
|                     exp : integer;
|                     sig : sigdig
|                 end;
|
| note that lisa pascal stores the sgn in the high-order
| byte of the allotted word, so the two legal word values
| of sgn are 0 and 256.
|-----------------------------------------------------------


|-----------------------------------------------------------
| decimal to binary conversion is governed by a format
| record defined in pascal as:
|
|   type
|       decform = record
|                     style  : (floatdecimal, fixeddecimal);
|                     digits : integer
|                 end;
|
| note again that the style field is stored in the high-
| order byte of the allotted word.
|
| these are the only operations with three operands.  the
| pointer to the format record is deepest in the stack,
| then the source pointer, and finally the destination
| pointer.
|-----------------------------------------------------------
fdec2x = ffext+fod2b

fdec2d = ffdbl+fod2b

fdec2s = ffsgl+fod2b

fdec2c = ffcomp+fod2b


|-----------------------------------------------------------
| binary to decimal conversion.
|-----------------------------------------------------------
fx2dec = ffext+fob2d

fd2dec = ffdbl+fob2d

fs2dec = ffsgl+fob2d

fc2dec = ffcomp+fob2d


|-----------------------------------------------------------
| equates and macros for elementary functions.
|-----------------------------------------------------------
flnx            =      /0000
flog2x          =      /0002
fln1x           =      /0004
flog21x         =      /0006

fexpx           =      /0008
fexp2x          =      /000a
fexp1x          =      /000c
fexp21x         =      /000e

fxpwri          =      /8010
fxpwry          =      /8012
fcompoundx      =      /c014
fannuityx       =      /c016

fsinx           =      /0018
fcosx           =      /001a
ftanx           =      /001c
fatanx          =      /001e
frandomx        =      /0020

