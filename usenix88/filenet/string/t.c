#include "string.h"
#include <stream.h>

char		nl[] = "\n";


main ()
{
    string	s = "Longish";
    string	c;
    string	a = "Hello ";
    string	b = "there\n";

    /*
     * Test concatenation, assignment, output, and construction from string&.
     */
    c = a + b;
    cout << c;
    cout << a + "in " + b;
    
    /*
     * Test substring selection.
     */
    cout << s(1, 3) << nl;
    c = s (2, 5);
    cout << "c = " << c << nl;
    
    /*
     * Test substrin assignment.
     */
    s (1, 3, string ("xyzABC"));
    cout << s << nl;

    /*
     * Test the logical operations
     */
    a = "xyz";
    b = "abc";
    cout << nl << "a = " << a << "\t\tb = " << b << nl;
    cout << "a == b: " << (a == b) << nl;
    cout << "a != b: " << (a != b) << nl;
    cout << "a > b: " << (a > b) << nl;
    cout << "a < b: " << (a < b) << nl;
    cout << "a >= b: " << (a >= b) << nl;
    cout << "a <= b: " << (a <= b) << nl;
    cout << "a != \"xyz\": " << (a != "abc") << nl;
    cout << "a == \"def\"  ||  a == b: " << (a == "abc"  ||  a == b) << nl;
    cout << "a == \"xyz\"  ||  a == b: " << (a == "xyz"  ||  a == b) << nl;
    
    cout << "\ntest left() and right():\n";
    cout << "a.left(10) =  `" << a.left(10) << "'\n";
    cout << "a.right(10) = `" << a.right(10) << "'\n";
    cout << "a.left(3) =   `" << a.left(3) << "'\n";

    cout << "\ntest left() of a constructed string and concatenation:\n";
    cout << "string(\"Hello\").left(10) + \"bingo\": `";
    cout << string("Hello").left(10) + "bingo";
    cout << "'\n";

    cout << "Test output of a floating point number: " << 3.14 << "\n";
    cout << string (3.14) << "\n";
}
