/* all: Load all the current bits of the standard Prolog system

                                                 Fernando Pereira
                                                 Updated: 30 December 82
*/

:-([
	 'pl/arith.pl',		   % Arithmetic compilation
         'pl/grammar.pl',             % DCG grammar rule translation
	 'pl/setof.pl',		   % Setof and sorting
         'pl/tracing.pl',             % Debugging evaluable predicates
         'pl/listing.pl',             % Listing the database
	 'pl/graphics.pl' 		   % Graphics
   ]).


:-([     'pl/protect.pl'    ]).       % Lock things up

