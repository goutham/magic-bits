#include "../include/magic_bits.hpp"

#include <cassert>
#include <iostream>

void PrintBitBoard(const uint64_t bitboard) {
  for (int row = 7; row >= 0; --row) {
    for (int col = 0; col <= 7; ++col) {
      if (bitboard & (1ULL << ((row * 8) + col))) {
        std::cout << "1 ";
      } else {
        std::cout << "0 ";
      }
    }
    std::cout << std::endl;
  }
}

int main() {
  magic_bits::Attacks attacks;
  const uint64_t bitboard = 0xF7F6F3748CA5B610ULL;
  PrintBitBoard(bitboard);
  std::cout << std::endl;
  uint64_t q_attacks = attacks.Queen(bitboard, 36);
  PrintBitBoard(q_attacks);
  std::cout << std::endl;
  assert(q_attacks == 0x4382c38509000ULL);
  q_attacks = attacks.Queen(bitboard, 29);
  PrintBitBoard(q_attacks);
  assert(q_attacks == 0x70d8708804ULL);
  // TODO: Add more tests.
  return 0;
}
