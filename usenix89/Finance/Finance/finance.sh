
trap '' 2
FINFILE=/tmp/FIN$$
FINISHED="FINISHED......
Do you wish to :
1. View output then remove
2. View output & send to mailbox
3. Send output to mailbox to view later
select: \c"
while :
do
echo " * * * PERSONAL FINANCE * * *
1. Savings Account
2. Amoritized Loan
3. Installment Loan
Q. Quit

Select: \c"
read select
case $select in
     1) while :
        do
        echo "SAVINGS....\nContinue (Y/N)? \c"
        read yorn
        case $yorn in
            [Yy]*) ;;
            *) exit;;
        esac
        echo "Starting Balance: \c"
        read amnt
        echo "Yearly Interest Rate (6.5% = 6.5): \c"
        read rate
        echo "Number of days to calculate: \c"
        read num
        echo "Print (F)ull Table or (S)ummery: \c"
        read how
        case $how in
            [Ff]*) how="F";;
            *) how="S";;
        esac
        echo "Calculating....."
        echo "$amnt $num $rate $how" |
        awk '{
            P = $1
            M = $2
            R = $3
            OR = $3
            TABLE = $4
        }
        END{
            R *= (.01/360)
            
            print "         * * * SAVINGS - INTEREST TABLE COMPOUNDED DAILY * * *"
            printf"\nBalance $%9.2f\nCompounded at %2.2f%%\nFor %d Days\n\n",P,OR,M
        
            if(TABLE == "F"){
            print "DAY        BALANCE        INTEREST"
            print "--------------------------------------------------"
            }
            TI = 0
            for(J = 1; J <= M; J++){
                I1 = P * R
                TI += I1
                P += I1
                if(TABLE == "F")
                    printf"%3d:     %9.2f     %3.6f\n",J,P,I1
            }
            print ""
            print "           BALANCE     TOTAL INTEREST"
            print "----------------------------------------------------"
            printf"        $%9.2f     %3.6f\n", P,TI
        }' > $FINFILE
        echo "$FINISHED"
        read select
        case $select in
            1) more $FINFILE;;
            2) more $FINFILE
               cat $FINFILE | mail $NAME;;
            3) cat $FINFILE | mail $NAME;;
            *) break;;
        esac
        rm $FINFILE
        echo "Again (Y/N) \c"
        read again
        case $again in
            [Yy]*) continue;;
            *) break;;
        esac
        done
        continue;;
    
    2) while :
        do
        echo "AMORTIZATION....\nContinue (Y/N)? \c"
        read yorn
        case $yorn in
            [Yy]*) ;;
            *) exit;;
        esac
        echo "Amount Borrowed (principal): \c"
        read amnt
        echo "Interest Rate (6.5% = 6.5): \c"
        read rate
        echo "Number of months to pay: \c"
        read num
        echo "Calculating....."
        echo "$amnt $num $rate" |
        awk '{
            P = $1
            M = $2
            R = $3
            OR = $3
        }
        END{
            R *= (.01/12)
            RR = R + 1
            PR = RR
            for(i = 1; i <= M - 1 ; i++){
                PR = PR * RR
            }
            E = (P * R * (PR))/((PR) - 1)
            print "         * * * AMORTIZATION TABLE * * *"
            printf"\nPrincipal: $%9.2f\nInterest Rate: %2.2f%%\nMonths: %d\n\n",P,OR,M
            print "Month   Principal   Interest + Principal = Monthly"
            print "Number  Owed        Payment    Payment     Payment"
            print "--------------------------------------------------"
            for(J = 1; J <= M; J++){
                I1 = P * R
                P1 = E - I1
                if(J == M){
                    P1 = P
                    I1 = E - P1
                }
                printf"%3d:  %9.2f  %9.2f  %9.2f  %9.2f\n",J,P,I1,P1,E
                T1 = T1 + I1
                TP = TP + P1 + I1
                SP = SP + P1
                P = P - P1
            }
            print ""
            print "                 Total       Total       Total"
            print "                 Interest    Principal   Payments"
            print "----------------------------------------------------"
            printf"                 %9.2f  %9.2f  %9.2f\n",T1, SP, TP
        }' > $FINFILE
        echo "$FINISHED"
        read select
        case $select in
            1) more $FINFILE;;
            2) more $FINFILE
               cat $FINFILE | mail $NAME;;
            3) cat $FINFILE | mail $NAME;;
            *) break;;
        esac
        rm $FINFILE
        echo "Again (Y/N) \c"
        read again
        case $again in
            [Yy]*) continue;;
            *) break;;
        esac
        done
        continue;;
    

    3) while :
        do
        echo "INSTALLMENT LOAN....\nContinue (Y/N)? \c"
        read yorn
        case $yorn in
            [Yy]*) ;;
            *) exit;;
        esac
        echo "Amount Borrowed (principal): \c"
        read amnt
        echo "Interest Rate (6.5% = 6.5): \c"
        read rate
        echo "Number of months to pay: \c"
        read num
        echo "Print (F)ull table or (S)ummery: \c"
        read how
        case $how in
            [Ff]*) how="F";;
            *) how="S";;
        esac
        echo "Calculating....."
        echo "$amnt $num $rate $how" |
        awk '{
            P = $1
            M = $2
            R = $3
            OR = $3
            TABLE=$4
        }
        END{
            R *= .01
            P1 = P/M
            print "         * * * AMORTIZATION TABLE * * *"
            printf"\nPrincipal: $%9.2f\nInterest Rate: %2.2f%%\nMonths: %d\n\n",P,OR,M
            if(TABLE == "F"){
            print "Month   Principal   Interest + Principal = Monthly"
            print "Number  Owed        Payment    Payment     Payment"
            print "--------------------------------------------------"
            }
            for(J = 1; J <= M; J++){
                I1 = (P * R)/12
                if(TABLE == "F")
                    printf"%3d:  %9.2f  %9.2f  %9.2f  %9.2f\n",J,P,I1,P1,P1 + I1
                T1 = T1 + I1
                TP = TP + P1 + I1
                SP = SP + P1
                P = P - P1
            }
            print ""
            print "                 Total       Total       Total"
            print "                 Interest    Principal   Payments"
            print "----------------------------------------------------"
            printf"                 %9.2f  %9.2f  %9.2f\n",T1, SP, TP
        }' > $FINFILE
        echo "$FINISHED"
        read select
        case $select in
            1) more $FINFILE;;
            2) more $FINFILE
               cat $FINFILE | mail $NAME;;
            3) cat $FINFILE | mail $NAME;;
            *) break;;
        esac
        rm $FINFILE
        echo "Again (Y/N) \c"
        read again
        case $again in
            [Yy]*) continue;;
            *) break;;
        esac
        done
        continue;;

    [Qq]) exit;;
    *) continue;;
esac
done
