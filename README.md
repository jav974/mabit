mabit
=====

mabit is intended to hold a representation of an arbitrary number of bits as a signed integer number.

As such it can act like a primitive integer and supports the following operators :

Arithmetic Operators :

  +
  +=
  -
  -=
  *
  *=
  /
  /=
  %
  %=
  
Bitwise operators :

  ~
  &
  &=
  |
  |=
  ^
  ^=
  <<
  <<=
  >>
  >>=
  
Comparison operators :

  ==
  !=
  <
  <=
  >
  >=
  
Since mabit simply holds an array of arbitrary type of unsigned integer, you can simply have access to each cell :

  []
  

mabit has to be templated on an unsigned integer (char, short, int) except for those holding >= 64bits !

e.g : 
  // mabit < unsigned char >  xxx(-42);

Further description is coming.

jav974
