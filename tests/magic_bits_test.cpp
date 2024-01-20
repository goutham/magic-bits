/*
g++ magic_bits_test.cpp -o run_tests && ./run_tests
g++ magic_bits_test.cpp -DMAGIC_BITS_REGENERATE_MAGICS -o run_tests && ./run_tests
*/

#include "../include/magic_bits.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>

/*
Given a board layout string, where '1' denotes an occupied square and '.' denotes
an empty square, this function returns its corresponding bitboard representation.

Mapping from board layout to indices:

    8| 56, 57, 58, 59, 60, 61, 62, 63,
    7| 48, 49, 50, 51, 52, 53, 54, 55,
    6| 40, 41, 42, 43, 44, 45, 46, 47,
    5| 32, 33, 34, 35, 36, 37, 38, 39,
    4| 24, 25, 26, 27, 28, 29, 30, 31,
    3| 16, 17, 18, 19, 20, 21, 22, 23,
    2|  8,  9, 10, 11, 12, 13, 14, 15,
    1|  0,  1,  2,  3,  4,  5,  6,  7,
        --  --  --  --  --  --  --  --
        A   B   C   D   E   F   G   H
*/
uint64_t FromLayout(const char* layout_str) {
  uint64_t bitboard = 0ULL;
  int sq = 0;
  for (int i = 0; layout_str[i] != '\0'; ++i) {
    if (layout_str[i] != '.' && layout_str[i] != '1') {
      continue;
    }
    const int index = (7 - (sq / 8)) * 8 + (sq % 8);
    sq++;
    if (sq > 64) {
      throw std::invalid_argument("Too many squares");
    }
    if (layout_str[i] == '.') {
      continue;
    }
    bitboard |= (1ULL << index);
  }
  if (sq < 64) {
    throw std::invalid_argument("Too few squares");
  }
  return bitboard;
}

int main() {
  magic_bits::Attacks attacks;
  const char* layout_str = R"""(
    1.11..1.
    .1...1.1
    1....11.
    ........
    ....1...
    ...1.1.1
    111..11.
    1....11.
  )""";
  assert(FromLayout(R"""(
    ........
    .1......
    .1...1..
    .1..1...
    .1.1....
    111.....
    1.1.....
    111.....
  )""") == attacks.Queen(FromLayout(layout_str), 9));
  assert(FromLayout(R"""(
    ...1...1
    ....1.1.
    ........
    ....1.1.
    ...1...1
    ..1.....
    .1......
    ........
  )""") == attacks.Bishop(FromLayout(layout_str), 45));
  assert(FromLayout(R"""(
    ..1.111.
    ...1....
    ...1....
    ...1....
    ...1....
    ...1....
    ........
    ........
  )""") == attacks.Rook(FromLayout(layout_str), 59));
  std::cout << "TESTS PASSED" << std::endl;
  return 0;
}
