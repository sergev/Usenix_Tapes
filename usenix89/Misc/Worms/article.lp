(C) 1986 Jan W. Steinman and M&T Publishing, Inc.  This article may be
copied for PERSONAL USE by any means, as long as credit is given to
the author and this copyright notice is included with the material.
M&T Publishing retains worldwide publication rights to this material.





                    TTTThhhheeee WWWWoooorrrrmmmm MMMMeeeemmmmoooorrrryyyy TTTTeeeesssstttt


                      Jan W. Steinman

                    2002 Parkside Court
                    West Linn, OR 97068
                   _t_e_k_t_r_o_n_i_x!_t_e_k_e_c_s!_j_a_n_s
                      503/657-7703 (h)
                      503/685-2956 (w)



     No, this is not a method for quantifying the mental

retentive powers of certain long, cylindrical invertebrates,

but a test that could help to diagnose certain types of

computer memory errors.  The Worm memory test uses a

dynamically executing program as the actual test data.

Unlike previous memory test programs of this type, this Worm

has a special twist -- it is able to overlay itself _w_h_i_l_e _i_t

_i_s _e_x_e_c_u_t_i_n_g, thanks to the MC68000 _p_r_e_f_e_t_c_h register.


SSSSoooommmmeeee FFFFeeeettttcccchhhhiiiinnnngggg FFFFaaaaccccttttssss


     Never heard of the prefetch register?  To understand

how the Worm works, it might help to review the way that the

MC68000 fetches and executes instructions.  The MC68000 uses

instruction pipelining in order to speed execution.  There

is, in effect, a sixteen-bit register between the data bus

and the instruction decoding logic.  (The MC68010 has 32

bits of prefetch, and the 68020 has a 64 entry instruction

cache, but the results should be similar.) When an

instruction is executed, the opcode for that instruction is

first loaded into the prefetch register (often while the

previously fetched instruction is being executed), then the









                           - 2 -


instruction is moved into the instruction decoding register,

where it is executed.  The net effect is that the processor

_u_s_u_a_l_l_y has a handle on the next thing it is supposed to do.


     Prefetch works fine most of the time, but does slow

things down during certain operations.  If the instruction

being executed causes a non-sequential instruction to be

executed, execution may be either faster or slower.  In the

case of a conditional branch instruction, a branch _t_a_k_e_n is

quite fast, because the prefetch register already holds the

displacement that must be added to the program counter in

order to fetch the next non-sequential instruction.  A

branch _n_o_t _t_a_k_e_n, however, will be a bit faster if it is a

short branch, because the next instruction is already in the

prefetch register, and the two clocks needed to add a

displacement to the program counter can be saved.  The worst

case happens when a branch is _n_o_t _t_a_k_e_n and the branch

displacement is sixteen bits.  In this case, the processor

has useless information in the prefetch register, and must

flush that information before it can fetch the next

instruction.


     Other non-sequential instructions cause an immediate

flush of the prefetch register, and use an extra four clocks

simply to restart the pipeline.  One exception is the

decrement-and-branch instruction which, like the taken

branches, benefits from having the branch displacement

handy.  (The MC68010, with its 32 bit prefetch register,










                           - 3 -


actually executes many 16 bit instructions out of the

prefetch register if they precede a decrement-and-branch

instruction.)


HHHHoooowwww TTTThhhheeee WWWWoooorrrrmmmm CCCCrrrraaaawwwwllllssss


     The Worm depends on these characteristics of pipelining

in order to overlay itself _w_h_i_l_e _i_t _i_s _r_u_n_n_i_n_g, but it needs

some management and control in order to be useful -- a Worm

on the loose would quickly destroy all of memory!  Besides

the Worm, a complete memory test requires two additional

parts: an initialization sequence, and a routine for

controlling the Worm and reporting its results.


     The initialization routine has some special

characteristics, and includes most of the system

dependencies.  It is only executed once, at the beginning,

and is therefore throw-away code.  This is why it is placed

last -- the Worm actually crawls right over its

initialization code in this implementation.  The registers

are set up to the specifications of the Worm, and several

important system functions are performed.  In particular, it

is important that page faulting does not occur in systems

that support virtual memory, and if special hocus-pocus is

needed to turn off interrupts, it should be done here.


     The Worm Manager exercises control over the Worm, and

is responsible for communicating errors it discovers, and

displaying progress messages, if desired.  When the Manager










                           - 4 -


is entered upon completion of a Worm pass, it must decide if

it has been entered because of an error, or simply as a

point of control.  If there has been an error, the Worm is

no longer runnable, so the Manager will have to report the

error and terminate.  If no error is detected, the Manager

must check the progress of the Worm to keep it from

consuming all of memory.  At this point, the Manager can

decide enough memory has been checked to warrant a progress

report of some kind.


     The real heart of the whole thing is, after all, the

Worm.  The Worm simply replicates itself, one long-word

lower in memory, while comparing the new copy of itself

against the original, which never executes.  The Worm may be

the heart of the Worm test, but the three instructions

starting at Crawl are where the magic happens.  This loop

starts at the beginning of Worm, and copies the first long

word down to Worm-4.  It continues with each additional long

word, until it gets to the long word at Crawl+4, which is a

DBNE instruction with its 16 bit displacement.  The

preceding MOVE.L and CMP.L have already been copied down.


     At this point, it becomes a little difficult to keep

track of what is data and what is code.  When the MOVE.L is

in the instruction decode register, ready to be executed,

the following CMP.L is in the prefetch register, waiting its

turn to be executed.  When the MOVE.L at Crawl executes, it

moves the DBNE instruction into the location it and the










                           - 5 -


following CMP.L, are currently occupying.  The processor has

no way of knowing it has just invalidated its prefetch

register, so it continues, moving the CMP.L instruction into

the instruction decode register, and moving the following

DBNE into the prefetch register.  The CMP.L executes,

comparing the DBNE just moved against the original, while

moving the branch displacement for the DBNE into the

prefetch register.


     Assuming the compare was successful, the DBNE executes,

decrementing D0 and branching backward four bytes to where

the MOVE.L used to be.  The prefetch register is flushed

because of the branch, so the value at that location is

loaded into the prefetch register, and immediately into the

instruction decode register.  But what is loaded?  A _c_o_p_y of

the DBNE, complete with the same negative displacement

value.  The condition codes have not changed, and the count

register D0 should not be anywhere near zero, so the copy of

the DBNE gets executed identically to its predecessor, which

still resides in the next long word.  The DBNE copy branches

to the MOVE.L copy, and the loop continues moving the code

down four bytes.


     When the count register D0 underflows, the DBNE copy

drops through, interrupts are enabled, the Worm's dynamic

image pointer A5 is adjusted to point to the new Worm copy,

and the Worm reports back to its Manager.  _N_o_t_e _t_h_a_t _n_o_n_e _o_f

_t_h_e _W_o_r_m _c_o_d_e _i_s _e_v_e_r _e_x_e_c_u_t_e_d _b_e_f_o_r_e _i_t _h_a_s _b_e_e_n _c_o_m_p_a_r_e_d










                           - 6 -


_a_n_d _v_e_r_i_f_i_e_d.


____________________________________________________________
                   Before                  After

    Crawl-4       ...               move.l   (a0)+,(a1)
    Crawl-2       ...               cmp.l    (a1)+,(a2)+
    Crawl   move.l   (a0)+,(a1)     dbne     d0,-6 <--+
    Crawl+2 cmp.l    (a1)+,(a2)+                      |
    Crawl+4 dbne     d0,-6 ---------------------------+
____________________________________________________________


     It is vitally important to disable interrupts when the

MOVE.L overlays itself and the following CMP.L.  An

interrupt at this point causes the prefetch to be flushed

when the interrupt is serviced.  Upon return from the

interrupt, the displacement part of the DBNE (Hex FFFA) will

be fetched as an instruction.  This will cause a "line 1111

emulator exception" unless your system has a coprocessor

with an id code of 7, but either way, _t_h_e _W_o_r_m _w_i_l_l _b_e

_b_r_o_k_e_n _a_n_d _t_h_e _m_e_m_o_r_y _t_e_s_t _w_i_l_l _f_a_i_l.  And of course, it is

important that the Worm length remain a multiple of four if

you decide to modify it!


BBBBuuuutttt WWWWhhhhaaaatttt GGGGoooooooodddd IIIIssss IIIItttt????


     I originally developed the MC68000 Worm test for an

embedded processor application which was having dynamic RAM

refresh problems. It was discovered that conventional RAM

tests, which move smoothly up through consecutive addresses,

were masking the problem by unintentionally providing

software refresh.  The Worm is not long enough to cause a

complete cycle of all of a dynamic RAMs _r_o_w-_a_d_d_r_e_s_s-_s_t_r_o_b_e










                           - 7 -


(RAS) lines, and was able to help diagnose the problem.


     In the form presented, this implementation of the Worm

is primarily useful as an illustrative example of position-

independent coding, modular design, and of course, a unique

use of the prefetch register.  It could be put to practical

use in a number of ways.


     The best use of the Worm might be to have it running

continuously, as a very low priority task.  The Manager

would have to take some of the responsibility of Init by

allocating test memory and restarting the Worm when it

finishes testing a buffer.  The interrupt disabling code may

be simpler on systems without virtual memory -- on the Amiga

it is a simple memory store.


     Virtual memory systems would also need to add code to

branch around the interrupt disabling code on the copy of

the first long word only, which would allow the Worm to

generate page faults whenever it first crosses a page

boundary.  To make it practical in such systems, the Manager

would have to access the memory management hardware in order

to map faulty virtual locations to broken chips.


     The Worm routine itself can hold much more code if

desired.  I originally had much of the Manager's decision

code in the Worm, which did speed it up at the expense of

simplicity.  In a message-based system, such as the Amiga,

the Manager could be totally deleted.  The Worm could










                           - 8 -


contain all the task code, merrily crawling through any

available RAM it could find and sending error reports

through inter-task messages, all with minimal impact on the

user.





















































