/*
 a test file for Integer class
 */

#include <Integer.h>

Integer factorial(Integer n)
{
  Integer f = 1;
  while (n > 0)
  {
    f *= n;
    --n;
  }
  return f;
}

Integer fibonacci(Integer n)
{
  if (n <= 0)
    return 0;
  else
  {
    Integer f = 1;
    Integer prev = 0;
    while (n > 1)
    {
      Integer tmp = f;
      f += prev;
      prev = tmp;
      --n;
    }
    return f;
  }
}


main()
{
  Integer one = 1;
  cout << "one = " << one << "\n";

  Integer two = 2;
  cout << "two = " << two << "\n";

  Integer fact30 = factorial(30);
  cout << "fact30 = factorial(30) = " << fact30 << "\n";

  Integer fact28 = factorial(28);
  cout << "fact28 = factorial(28) = " << fact28 << "\n";

  cout << "fact30 + fact28 = " << fact30 + fact28 << "\n";
  cout << "fact30 - fact28 = " << fact30 - fact28 << "\n";
  cout << "fact30 * fact28 = " << fact30 * fact28 << "\n";
  cout << "fact30 / fact28 = " << fact30 / fact28 << "\n";
  cout << "fact30 % fact28 = " << fact30 % fact28 << "\n";
  cout << "-fact30 = " << -fact30 << "\n";
  cout << "abs(-fact30) = " << abs(-fact30) << "\n";
  cout << "lg(fact30) = " << lg(fact30) << "\n";
  cout << "gcd(fact30, fact28) = " << gcd(fact30, fact28) << "\n";
  cout << "sqrt(fact30) = " << sqrt(fact30) << "\n";

  Integer negfact31 = fact30;
  negfact31 *= -31;
  cout << "negfact31 = " << fact30 << "\n";
  cout << "fact30 + negfact31 = " << fact30 + negfact31 << "\n";
  cout << "fact30 - negfact31 = " << fact30 - negfact31 << "\n";
  cout << "fact30 * negfact31 = " << fact30 * negfact31 << "\n";
  cout << "fact30 / negfact31 = " << fact30 / negfact31 << "\n";
  cout << "fact30 % negfact31 = " << fact30 % negfact31 << "\n";
  cout << "gcd(fact30, negfact31) = " << gcd(fact30, negfact31) << "\n";

  Integer fib50 = fibonacci(50);
  cout << "fib50 = fibonacci(50) = " << fib50 << "\n";
  Integer fib48 = fibonacci(48);
  cout << "fib48 = fibonacci(48) = " << fib48 << "\n";

  cout << "fib48 + fib50 = " << fib48 + fib50 << "\n";
  cout << "fib48 - fib50 = " << fib48 - fib50 << "\n";
  cout << "fib48 * fib50 = " << fib48 * fib50 << "\n";
  cout << "fib48 / fib50 = " << fib48 / fib50 << "\n";
  cout << "fib48 % fib50 = " << fib48 % fib50 << "\n";
  cout << "gcd(fib50, fib48) = " << gcd(fib50, fib48) << "\n";
  cout << "sqrt(fib50) = " << sqrt(fib50) << "\n";
  
  Integer pow64 = pow(two, 64);
  cout << "pow64 = pow(two, 64) = " << pow64 << "\n";
  cout << "lg(pow64) = " << lg(pow64) << "\n";

  Integer s64 = one << 64;
  cout << "s64 = one << 64 = " << s64 << "\n";
  cout << "pow64 == s64 = " << (pow64 == s64) << "\n";
  cout << "pow64 != s64 = " << (pow64 != s64) << "\n";
  cout << "pow64 <  s64 = " << (pow64 <  s64) << "\n";
  cout << "pow64 <= s64 = " << (pow64 <= s64) << "\n";
  cout << "pow64 >  s64 = " << (pow64 >  s64) << "\n";
  cout << "pow64 >= s64 = " << (pow64 >= s64) << "\n";

  Integer s32 = s64 >> 32;
  cout << "s32 = s64 >> 32 = " << s32 << "\n";
  cout << "pow64 == s32 = " << (pow64 == s32) << "\n";
  cout << "pow64 != s32 = " << (pow64 != s32) << "\n";
  cout << "pow64 <  s32 = " << (pow64 <  s32) << "\n";
  cout << "pow64 <= s32 = " << (pow64 <= s32) << "\n";
  cout << "pow64 >  s32 = " << (pow64 >  s32) << "\n";
  cout << "pow64 >= s32 = " << (pow64 >= s32) << "\n";

  Integer comps64 = ~s64;
  cout << "comp64 = ~s64 = " << comps64 << "\n";
  cout << "comps64 & s32 = " << (comps64 & s32) << "\n";
  cout << "comps64 | s32 = " << (comps64 | s32) << "\n";
  cout << "comps64 ^ s32 = " << (comps64 ^ s32) << "\n";

/*
  cout << "The following should abort execution:\n";
  cout << "sqrt(negfact31) = " << sqrt(negfact31) << "\n";
*/
  cout << "\nEnd of test\n";
}
