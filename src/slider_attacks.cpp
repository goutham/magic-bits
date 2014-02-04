#include "common.h"
#include "slider_attacks.h"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>

template <typename T>
void ReadFile(const std::string& filename, std::vector<T>* v) {
  std::ifstream ifs(filename.c_str(), std::ios::in);
  if (!ifs.is_open()) {
    std::cerr << "Unable to open file " << filename << std::endl;
    exit(-1);
  }
  std::string s;
  while (ifs >> s) {
    v->push_back(T(std::strtoull(s.c_str(), nullptr, 16)));
  }
  ifs.close();
}

void SliderAttacks::Initialize() {
  std::vector<U64> rook_masks;
  ReadFile("rook_masks.magic", &rook_masks);
  assert(rook_masks.size() == SQUARES);

  std::vector<U64> bishop_masks;
  ReadFile("bishop_masks.magic", &bishop_masks);
  assert(bishop_masks.size() == SQUARES);

  std::vector<U64> rook_magics;
  ReadFile("rook_magics.magic", &rook_magics);
  assert(rook_magics.size() == SQUARES);

  std::vector<U64> bishop_magics;
  ReadFile("bishop_magics.magic", &bishop_magics);
  assert(bishop_magics.size() == SQUARES);

  std::vector<int> rook_offsets;
  ReadFile("rook_offsets.magic", &rook_offsets);
  assert(rook_offsets.size() == SQUARES);

  std::vector<int> bishop_offsets;
  ReadFile("bishop_offsets.magic", &bishop_offsets);
  assert(bishop_offsets.size() == SQUARES);

  std::vector<int> rook_shifts;
  ReadFile("rook_shifts.magic", &rook_shifts);
  assert(rook_shifts.size() == SQUARES);

  std::vector<int> bishop_shifts;
  ReadFile("bishop_shifts.magic", &bishop_shifts);
  assert(bishop_shifts.size() == SQUARES);

  ReadFile("rook_attack_table.magic", &rook_attack_table_);
  ReadFile("bishop_attack_table.magic", &bishop_attack_table_);

  for (int i = 0; i < SQUARES; ++i) {
    rook_magics_[i] = { rook_masks.at(i), rook_magics.at(i),
                        rook_shifts.at(i), rook_offsets.at(i) };
    bishop_magics_[i] = { bishop_masks.at(i), bishop_magics.at(i),
                          bishop_shifts.at(i), bishop_offsets.at(i) };
  }
}
