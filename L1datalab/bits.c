/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  // XOR == (not x and y) or (x and not y)
  // Use DeMorgans to turn or into and
  // XOR == not (not(not x and y) and not (x and not y))
  int xor = ~ (~(~ x & y) & ~(x & ~y));
  return xor;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  // tmin is 1 in most significant bit 0 elsewhere
  return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  // tmax = 0x7FFFFFFF
  // tmax + 1 = tmin
  // tmax + tmin = -1  -> + 1 or ~ to make 0
  // but x = -1 so check
  return (!((x+1)+x+1)) & (!!(x+1));
  
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  // make 32 bit mask with 1 in odd bits.
  // (mask & x) to make all even bits 0
  // (mask & x) ^ mask = 0 if all odd bits are set to 1
  // logical not to get 1 when all odd bits are set to 1 and 0 otherwise
  int odd = 0xAA;
  int mask = (odd << 8) + odd;
  mask = (mask << 8) + odd;
  mask = (mask << 8) + odd;
  return !((x & mask) ^ mask);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  // not x + 1 in twos complement (tmin edge case)
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  // cond1 -> bits 4 to 31 set to 0x3 
  // cond2 -> bit 3 not set (0x30-0x37) OR bits 1 and 2 not set bit 3 set (0x38, 0x39)
  int cond1 = !((x >> 4) ^ 0x3);
  int a = x & 0x0F;
  int lt8 = (a >> 3) ^ 1;
  int _89 = !((a >> 1) ^ 0x4);
  int cond2 = lt8 | _89;
  return cond1 & cond2;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  // false case -> 0x00000000 mask
  // true case -> !! to get 1 from any true int
  // shift left to get 0x80000000 then shift right (arithmetic) to get 0xFFFFFFFF
  // if mask is 0xFFFFFFFF and with any value y gives y
  int mask = ((!!x) << 31) >> 31;
  return (mask & y) | (~mask & z); 
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  // two cases same sign and different
  // same -> x + ~y < 0 -> x < y
  // as we want to include equals do not + 1, deals with tMin edge case as well
  // different -> check sign of x i.e neg x and pos y = true, pos x neg y = false
  int diffSign = ((x^y) >> 31) & 1;
  int less = ((x+(~y)) >> 31) & 1;
  return (diffSign & ((x>>31) & 1)) | ((!diffSign) & less);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  // make last bit represent 1 present in any bit
  // | first and second halves, for bit x in bits 0-15, bit x = 1 if bit x+ or bit x+16 were 1
  // continue until last bit
  x |= (x >> 16);
  x |= (x >> 8);
  x |= (x >> 4);
  x |= (x >> 2);
  x |= (x >> 1);
  return (~x) & 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  // find leftmost 1 bit
  // use !! to check for any 1
  // check upper half -> if 1 then add len(half) to res -> drop bottom half wtih >>
  // else check bottom half 
  // halving each step until 0; 16->8->4->2->1 + 1(minimum bit)
  // edge case negative numbers
  // check how many bits for ~x 
  // as -1 only need 1 -2 only 2...
  int flag;
  int cnt_16, cnt_8, cnt_4, cnt_2, cnt_1, cnt_0;
  int sign = x >> 31;
  
  x = (sign&(~x)) | ((~sign)&x);

  flag = !!(x >> 16);
  cnt_16 = flag << 4;
  x = x >> cnt_16;

  flag = !!(x >> 8);
  cnt_8 = flag << 3;
  x = x >> cnt_8;

  flag = !!(x >> 4);
  cnt_4 = flag << 2;
  x = x >> cnt_4;

  flag = !!(x >> 2);
  cnt_2 = flag << 1;
  x = x >> cnt_2;

  flag = !!(x >> 1);
  cnt_1 = flag;
  x = x >> cnt_1;

  cnt_0 = x;
  return cnt_16 + cnt_8 + cnt_4 + cnt_2 + cnt_1 + cnt_0 + 1;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  // sign always unchanged
  // exp == 0x7FF00000 for NaN
  // denorm > shift left 1 
  // norm -> frac same, exp + 1
  unsigned frac = uf & 0x7FFFFF;
  unsigned sign = uf & 0x80000000;
  unsigned exp = uf & 0x7F800000;
  if (exp >= 0x7F800000 ) {
    return uf;
  } else if (exp == 0) {
    return sign | ((exp+frac) << 1);
  } else {
    return sign | (exp + 0x800000) | frac; 
  }

}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  // denorm -> 0 exp < bias
  // norm -> 32 > exp - bias (127) = exp < 159
  //      -> need to add back implied 1 bit infront of frac -> frac is 24 bits
  //      -> frac >> (23 - (exp-127)) = 150 -exp
  // NaN and inf when exp > 158
  unsigned frac = uf & 0x7FFFFF;
  unsigned sign = uf & 0x80000000;
  unsigned exp = (uf >> 23) & 0xFF;
  
  if (exp < 127) {
    return 0;
  } else if (exp > 158) {
    return 0x80000000u;
  }

  frac |= 0x800000;
  frac = frac >> (150 - exp);

  return sign ? -frac : frac;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  // sign positive
  // frac = 0
  // exp = x + 127
  // valid range x [-127, 127]
  // exp == 0 denorm
  int exp = x + 127;
  if (exp < 0) {
    return 0;
  } else if (exp > 254) {
    exp = 255 << 23;
  } else {
    exp = exp << 23;
  }
  if (exp == 0) {
    return exp | (1 << 22);
  } else {
    return exp;
  }

}
