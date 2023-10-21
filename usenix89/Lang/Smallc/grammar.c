
# FILE MAIN.C

parse  ::= "extern" dodclsEXTERN
        |  "static" dodclsSTATIC
        |  dodclsPUBLIC
        |  "#asm" doasm
        |  "#include" doinclude
        |  "#define" dodefine
        |  "#undef" doundef
        |  newfunc

dodcls ::= char declglb
        |  int  declglb
        |  declglb

# FILE FUNCTION.C

newfunc::= symname ( symname [, symname]* )
           _newfunc statementYES
_newfunc::= [register] char/int getarg ns

getarg  ::= [*] symname [ "[]" ] [, [*] symname [ "[]" ]]*

# FILE STMT.C

statementYES ::= { compound

statementNO  ::= stst
              |  { compound

stdecl     ::= "register" doldclsDEFAULT
            |  "auto"     doldclsDEFAUTO
            |  "static"   doldclsLSTATIC
            |  doldclsAUTO

stst       ::= "if" doif
            |  "while" dowhile
            |  "switch" doswitch
            |  "do" dodo
            |  "for" dofor
            |  "return" doreturn
            |  "break" dobreak
            |  "continue" docont
            |  "case" docase
            |  "default" dodefault
            |  "#asm" doasm
            |  expression

compound   ::= [ stdecl ns ]* [ stst ns ]* }


# FILE EXPR.C

expression ::= heir1 [, heir1]*

heir1  ::= heir1a
        |  heir1a = heir1
        |  heir1a -= heir1
        |  heir1a += heir1
        |  heir1a *= heir1
        |  heir1a /= heir1
        |  heir1a %= heir1
        |  heir1a >>= heir1
        |  heir1a <<= heir1
        |  heir1a ^= heir1
        |  heir1a |= heir1

heir1a ::= heir1b
        |  heir1b [? heir1b : heir1b]+

heir1b ::= heir1c
        |  heir1c [|| heir1c]+

heir1c ::= heir2
        |  heir2 [&& heir2]+

heir2  ::= heir3
        |  heir3 ["|" heir3]+

heir3  ::= heir4
        |  heir4 [^ heir4]+

heir4  ::= heir5
        |  heir5 [& heir5]+

heir5  ::= heir6
        |  heir6 [== heir6]
        |  heir6 [!= heir6]

heir6  ::= heir7
        |  heir7 [<= heir7]+
        |  heir7 [>= heir7]+
        |  heir7 [<  heir7]+
        |  heir7 [>  heir7]+

heir7  ::= heir8
        |  heir8 [>> heir8]+
        |  heir8 [<< heir8]+

heir8  ::= heir9
        |  heir9 [+ heir9]+
        |  heir9 [- heir9]+

heir9  ::= heir10
        |  heir10 [* heir10]+
        |  heir10 [/ heir10]+
        |  heir10 [% heir10]+

heir10 ::= ++ heir10
        |  -- heir10
        |  -  heir10
        |  ~  heir10
        |  !  heir10
        |  *  heir10
        |  &  heir10
        |  heir11 ++
        |  heir11 --
        |  heir11

heir11 ::= primary
        |  primary ( expression ) 
        |  primary [ expression ]

# FILE PRIMARY.C

primary::= ( heir1 )
        |  "sizeof" ( "int" )
        |  "sizeof" ( "char" )
        |  "sizeof" ( symname )
        |  symname[GLOBAL/LOCAL]
        |  constant

constant::= number
         |  pstr
         |  qstr

number ::= [+] _number | [-] _number
_number::= 0x DIGITS | 0X DIGITS | 0 DIGITS | DIGITS

pstr   ::= ' [char]* '

qstr   ::= " [char]* "

char   ::= [^\] | \\ | \ spechar

spechar::= n | t | r | f | b | 0
