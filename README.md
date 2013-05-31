mabit
=====

mabit is intended to hold a representation of an arbitrary number of bits as a signed integer number.

As such it can act like a primitive integer and supports the following operators :

Arithmetic Operators :

   +  +=  -   -=  *   *=  /   /=  %   %=
  
Bitwise operators :

   ~  &   &=   |  |=  ^   ^=   <<  <<=   >>   >>=
  
Comparison operators :

   ==   !=  <   <=   >   >=
  
Since mabit simply holds an array of arbitrary type of unsigned integer, you can simply have access to each cell :

  []
  

mabit has to be templated on an unsigned integer (char, short, int long) except unsigned long long !

e.g : 
  // mabit < unsigned char >  xxx(-42);
  
The main goal behind that was to manipulate strings with more ease

Further description is coming.

jav974
