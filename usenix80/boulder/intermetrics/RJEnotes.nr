          
          
          
                       Installing RJE on Unix: A Case History
          
                                     Jude Miller
          
                                    Intermetrics
          
          
          _I_n_t_r_o_d_u_c_t_i_o_n
          
          One of our main goals in acquiring Unix was to setup a convenient
          telephone link to IBM MVS via JES2, a descendent of HASP.  We
          allowed a month for this goal, but required closer to five months
          to achieve it.  This history is presented with the hope that
          someone may save time by not repeating all of our mistakes.
          
          _C_o_n_f_i_g_u_r_a_t_i_o_n
          
          Our first mistake was in ordering our hardware.  We ordered a
          PDP-11/70 with 128K words of memory, two RP06 disks, DH11 multi-
          plexer, terminals, printer, and mag tape.  We also ordered a
          DQS-11B, a Bell 208 modem, and an additional phone line.  How-
          ever, the DQS-11 synchronous line controller was quoted as an 8
          month delivery, so a DQ-11 was ordered instead.  Unfortunately, a
          DQ-11 is not a DQS-11.  Although they are designed to do the same
          job, they do it in different ways.  It may be possible to con-
          struct a DQ-11 device handler which makes it possible to simulate
          a DQS-11, but we were unable to do so.  So, we ordered a DQS-11
          after all.  Fortunately, it was shipped within 45 days.
          
          _H_a_r_d_w_a_r_e__I_n_s_t_a_l_l_a_t_i_o_n
          
          Our DQS11-BA (Bx -- EBCDIC, xA -- EIA signals) was quickly
          installed and checked out by DEC personnel.  Unfortunately, func-
          tional test of the DQS-11 requires two DQS-11's.  Because the
          DQS-11 is a DEC Special Systems Group product, the local DEC
          office was not equipped to completely verify its operation.  A
          diagnostic (DEC-supplied) indicated that the device was okay
          internally, but did not check the actual modem signals.  We
          decided to assume these were okay, and proceeded to install Unix
          'hasp' according to "I.1  Setting Up PWB/Unix."
          
               After building the hasp system via the supplied 'make' file,
          We tried the command 'haspinit'.  The message 'phone connect not
          made' was printed.  After a few days of experiments and code
          reading, we determined that the file 'rje/lines' was not
          correctly initialized.  In our case, it needed to end with our
          remote number followed by any non-delimiting character followed
          by plus.  The plus indicates that no autodialing should be
          attempted.
          
               We confidently modified our 'lines' file and retried
          haspinit.  Many 'stray interrupt' messages began appearing on the
          system tty.  DEC was called , and a different CE opined that the
          device address for the DQS-11 had been inappropriately chosen.
          The address was modified to 765050, the system description file
          
          
          
                                        - 1 -
          
          
          
          
          
          
                                        - 2 -
          
          
          was updated, and the system remade.  Fresh trials of haspinit
          produced only the message 'phone connect not made.' After a few
          hours of this, we inserted a patch cable between the modem and
          the DQS-11 and monitored traffic with a logic analyzer.  'ENQ'
          messages were being transmitted to the modem, but no 'ACK's or
          anything else were being received.  Eventually, someone noted
          that the modem status lights indicated transmission when the
          patch cable was connected, but not when the normal cable was.  We
          followed this clue and discovered that that the cable terminated
          at the DQS-11 end at a circuit board holding several wire
          jumpers, none of them cut.  We had the prints handy for the DQS-
          11, and found comments for installing this cable on a DQ-11, but
          none for a DQS-11.  The DQ-11 instructions required several
          jumpers to be cut, which we severed with some glee.
          
          _S_o_f_t_w_a_r_e__I_n_s_t_a_l_l_a_t_i_o_n
          
          At this point, we were able to run haspinit to completion.  We
          immediately used send to transmit a few jobs to MVS and waited
          eagerly for the results.  When it became apparent that none were
          forthcoming, we signed onto MVS through another RJE system and
          found the results of our transmissions: a few incomplete JOB
          cards and JCL error messages.  We consulted another installation
          with functioning RJE.  They said that they had had trouble with
          their MVS host's remote line definitions.  Our remote unit was
          indeed incorrectly defined. The correct definitions included:
          
          
          
               RMTxx 2770 ABUFEX CONSOLE DISCINTV=0 FIXED LINE=yy
                       NUMPR=1 NUMPU=1 NUMRD=1 ROUTE TRANSP BLOCKED
               LINEyy CODEA HDUPLEX HISPED IFACEA NOADIS TRANSP EBCDIC
          
          
          
          _N_i_t_s
          
          The long-awaited time was at hand; jobs began to flow from Unix
          to MVS and back again.  Our users soon discovered two problems,
          though.  The first was that our MVS host used non-standard EBCDIC
          codes for the characters '[', ']', '{', and '}'.  The translate
          tables in hdisp, hrecv, rjestat, and send were modified and this
          problem was solved.  Also, jobs periodically disappeared.  This
          happened whenever the 'usr=( ... )' specification was wrong or
          missing, as for example on print jobs routed to our remote unit.
          This was fixed by recompiling haspdisp with the SFILE flag
          defined as '1'.
          
               We now use our RJE facility heavily and are very pleased
          with its performance, although the response to 'rjestat' commands
          is very slow when large files are being sent or received.  We
          currently use an autodialer run by a separate command before we
          do haspinit, but we hope to automate dialup and shutdown
          
          
          
          
          
          
          
          
          
          
                                        - 3 -
          
          
          completely soon.  We have not made the timeout modification men-
          tioned in "I.2 Administrative Advice", but have not noticed any
          problems with timeouts.  To any potential Unix RJE installers, we
          say "Bon courage!"
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
          
