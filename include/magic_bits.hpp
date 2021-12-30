#ifndef __MAGIC_BITS_HPP__
#define __MAGIC_BITS_HPP__

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <vector>

namespace magic_bits {

// A class for generating attack bitboards for sliding pieces (queen, rook, bishop) using magic
// bitboards technique (https://www.chessprogramming.org/Magic_Bitboards).
class Attacks {
public:
  Attacks() {

// We use pre-computed magics by default but the algorithm to generate magics from first principles
// is retained for illustrative purposes, and can be enabled by option MAGIC_BITS_REGENERATE_MAGICS.
#ifdef MAGIC_BITS_REGENERATE_MAGICS
    std::function<uint64_t()> rook_rand_gen = ZeroBitBiasedRandom;
    std::function<uint64_t()> bishop_rand_gen = ZeroBitBiasedRandom;
#else
    std::function<uint64_t()> rook_rand_gen = PreComputedRookMagics();
    std::function<uint64_t()> bishop_rand_gen = PreComputedBishopMagics();
#endif

    Generate(rook_rand_gen,
             {Direction(Direction::NORTH), Direction(Direction::SOUTH), Direction(Direction::EAST),
              Direction(Direction::WEST)},
             rook_shifts_, rook_magics_, &rook_attack_table_, rook_offsets_, rook_masks_);
    Generate(bishop_rand_gen,
             {Direction(Direction::NORTH_EAST), Direction(Direction::NORTH_WEST),
              Direction(Direction::SOUTH_EAST), Direction(Direction::SOUTH_WEST)},
             bishop_shifts_, bishop_magics_, &bishop_attack_table_, bishop_offsets_, bishop_masks_);
  }

  // Generates the rook attack bitboard given a board occupancy bitboard and the board index where
  // the attacking rook is placed. Bounds are not checked but it is expected that 0 <= index <= 63.
  uint64_t Rook(const uint64_t occupancy_bitboard, const int index) const {
    return rook_attack_table_[AttackTableIndex(occupancy_bitboard, rook_masks_[index],
                                               rook_magics_[index], rook_shifts_[index],
                                               rook_offsets_[index])];
  }

  // Generates the bishop attack bitboard given a board occupancy bitboard and the board index where
  // the attacking bishop is placed. Bounds are not checked but it is expected that 0 <= index<= 63.
  uint64_t Bishop(const uint64_t occupancy_bitboard, const int index) const {
    return bishop_attack_table_[AttackTableIndex(occupancy_bitboard, bishop_masks_[index],
                                                 bishop_magics_[index], bishop_shifts_[index],
                                                 bishop_offsets_[index])];
  }

  // Generates the queen attack bitboard given a board occupancy bitboard and the board index where
  // the attacking queen is placed. Bounds are not checked but it is expected that 0 <= index <= 63.
  uint64_t Queen(const uint64_t occupancy_bitboard, const int index) const {
    return Rook(occupancy_bitboard, index) | Bishop(occupancy_bitboard, index);
  }

private:
  class Direction {
  public:
    enum D { NORTH, SOUTH, EAST, WEST, NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST };

    Direction(D direction) : direction_(direction) {}

    // Index of the next square along this direction. Returns -1 if next index is outside the board.
    int NextIndex(int index) const {
      int row = Row(index);
      int col = Col(index);

      // clang-format off
      switch (direction_) {
        case NORTH:      ++row;        break;
        case SOUTH:      --row;        break;
        case EAST:       ++col;        break;
        case WEST:       --col;        break;
        case NORTH_EAST: ++row; ++col; break;
        case NORTH_WEST: ++row; --col; break;
        case SOUTH_EAST: --row; ++col; break;
        case SOUTH_WEST: --row; --col; break;
      }
      // clang-format on
      return (row > 7 || col > 7 || row < 0 || col < 0) ? -1 : Indx(row, col);
    }

    // Number of squares from given square to the edge of the board along this direction.
    int EdgeDistance(int index) const {
      using std::min;
      int row = Row(index);
      int col = Col(index);

      auto inv = [](int x) -> int { return 7 - x; };

      int d = -1;
      // clang-format off
      switch (direction_) {
        case NORTH:      d = inv(row);                break;
        case SOUTH:      d = row;                     break;
        case EAST:       d = inv(col);                break;
        case WEST:       d = col;                     break;
        case NORTH_EAST: d = min(inv(row), inv(col)); break;
        case NORTH_WEST: d = min(inv(row), col);      break;
        case SOUTH_EAST: d = min(row, inv(col));      break;
        case SOUTH_WEST: d = min(row, col);           break;
      }
      // clang-format on
      assert(d >= 0 && d <= 7);
      return d;
    }

  private:
    static unsigned Indx(unsigned row, unsigned col) { return row * 8 + col; }
    static unsigned Row(unsigned index) { return index / 8; }
    static unsigned Col(unsigned index) { return index % 8; }

    D direction_;
  };

  class OccupancyCombiner {
  public:
    OccupancyCombiner(int index) : index_(index) {}

    // Combines occupancy bitboards by bitwise ORing each stored bitboard with bitboards generated
    // by GenerateOccupancies along given direction.
    void Combine(const Direction& direction) {
      std::vector<uint64_t> bbv;
      GenerateOccupancies(direction, index_, &bbv);
      if (bbv.empty()) {
        return;
      }
      if (occupancies_.empty()) {
        occupancies_.insert(occupancies_.end(), bbv.begin(), bbv.end());
        return;
      }
      std::vector<uint64_t> tmp;
      for (const uint64_t bb : bbv) {
        for (const uint64_t occupancy : occupancies_) {
          tmp.push_back(bb | occupancy);
        }
      }
      occupancies_.swap(tmp);
    }

    const std::vector<uint64_t>& Occupancies() const { return occupancies_; }

  private:
    // Generate all piece occupancies along a rank, file or diagonal, in the given direction, with
    // index as the reference point. The square given by the index and the edge of the board in the
    // given direction are not covered. For example, direction = NORTH_WEST, index = 29 (marked by
    // X) will generate all combinations of occupancies for squares marked by # (there are 8
    // possible occupancies):
    // 8 | 0 0 0 0 0 0 0 0
    // 7 | 0 0 # 0 0 0 0 0
    // 6 | 0 0 0 # 0 0 0 0
    // 5 | 0 0 0 0 # 0 0 0
    // 4 | 0 0 0 0 0 X 0 0
    // 3 | 0 0 0 0 0 0 0 0
    // 2 | 0 0 0 0 0 0 0 0
    // 1 | 0 0 0 0 0 0 0 0
    // -------------------
    //   | A B C D E F G H
    static void GenerateOccupancies(const Direction& direction, const int index,
                                    std::vector<uint64_t>* bbv) {
      // Number of squares in this direction excluding current square and edge of the board.
      const int num_squares = direction.EdgeDistance(index) - 1;
      if (num_squares <= 0) {
        return;
      }

      // Number of possible piece occupancies in these squares along the given direction.
      const unsigned num_occupancies = (1U << num_squares);

      // Create bitboard for each occupancy with the index next to given index as starting point,
      // along the given direction.
      for (unsigned occupancy = 0U; occupancy < num_occupancies; ++occupancy) {
        uint64_t bitboard = 0ULL;
        int next_index = index;
        for (unsigned bit_mask = 1U; bit_mask <= occupancy; bit_mask <<= 1) {
          next_index = direction.NextIndex(next_index);
          assert(next_index != -1);
          bitboard |= (uint64_t(!!(occupancy & bit_mask)) << next_index);
        }
        bbv->push_back(bitboard);
      }
    }

    const int index_;
    std::vector<uint64_t> occupancies_;
  };

  static size_t AttackTableIndex(const uint64_t bitboard, uint64_t mask, uint64_t magic, int shift,
                                 int offset) {
    uint64_t occupancy = bitboard & mask;
    return ((occupancy * magic) >> (kSquares - shift)) + offset;
  }

  // Masks all the bits from the given index, and along the given direction to 1, excluding the
  // square given by the index and the edge of the board along given direction.
  static uint64_t MaskBits(const Direction& direction, const int index) {
    uint64_t bitboard = 0ULL;
    int next_index = index;
    while ((next_index = direction.NextIndex(next_index)) >= 0 &&
           direction.NextIndex(next_index) >= 0) {
      bitboard |= (1ULL << next_index);
    }
    return bitboard;
  }

  // Generate an attack bitboard from a given square in the given direction for a specific occupancy
  // of pieces.
  static uint64_t GenerateAttack(const Direction& direction, const int index,
                                 const uint64_t occupancy) {
    uint64_t attack_bb = 0ULL;
    for (int i = index; (i = direction.NextIndex(i)) != -1;) {
      attack_bb |= (1ULL << i);
      if (occupancy & (1ULL << i)) {
        break;
      }
    }
    return attack_bb;
  }

  static void GenerateMagic(const std::function<uint64_t()>& rand_gen,
                            const std::vector<Direction>& directions, const int index,
                            const int shift_bits, uint64_t* magic,
                            std::vector<uint64_t>* attack_table) {
    // Generate occupancies.
    OccupancyCombiner combiner(index);
    for (const Direction& direction : directions) {
      combiner.Combine(direction);
    }
    std::vector<uint64_t> occupancies = combiner.Occupancies();

    // Generate attacks.
    std::vector<uint64_t> attacks;
    for (const uint64_t occupancy : occupancies) {
      uint64_t attack = 0ULL;
      for (const Direction& direction : directions) {
        attack |= GenerateAttack(direction, index, occupancy);
      }
      attacks.push_back(attack);
    }

    // No bishop or rook attack can cover all squares of the board.
    static const uint64_t kInvalidAttack = ~0ULL;

    // Trial and error approach to generate magics.
    while (true) {
      std::vector<uint64_t> table(1U << shift_bits, kInvalidAttack);
      uint64_t candidate_magic = rand_gen();
      bool collision = false;
      for (size_t k = 0; k < occupancies.size(); ++k) {
        const uint64_t occupancy = occupancies.at(k);
        const uint64_t attack = attacks.at(k);
        const int offset = (occupancy * candidate_magic) >> (64 - shift_bits);
        if (table.at(offset) == kInvalidAttack || table.at(offset) == attack) {
          table.at(offset) = attack;
        } else {
          collision = true;
          break;
        }
      }
      if (!collision) {
        *magic = candidate_magic;
        attack_table->swap(table);
        break;
      }
    }
  }

  static void Generate(const std::function<uint64_t()>& rand_gen,
                       const std::vector<Direction>& directions, const int shifts[],
                       uint64_t magics[], std::vector<uint64_t>* attack_table, int offsets[],
                       uint64_t masks[]) {
    for (int i = 0; i < kSquares; ++i) {
      masks[i] = 0ULL;
      for (const Direction& d : directions) {
        masks[i] |= MaskBits(d, i);
      }
      std::vector<uint64_t> tmp_attack_table;
      GenerateMagic(rand_gen, directions, i, shifts[i], &magics[i], &tmp_attack_table);
      offsets[i] = attack_table->size();
      attack_table->insert(attack_table->end(), tmp_attack_table.begin(), tmp_attack_table.end());
    }
  }

  // Returns a unsigned 64 bit random number.
  static uint64_t U64Rand() {
    return (uint64_t(0xFFFF & rand()) << 48) | (uint64_t(0xFFFF & rand()) << 32) |
           (uint64_t(0xFFFF & rand()) << 16) | uint64_t(0xFFFF & rand());
  }

  // Bias the random number to contain more 0 bits.
  static uint64_t ZeroBitBiasedRandom() { return U64Rand() & U64Rand() & U64Rand(); }

  class PreComputedRookMagics {
  public:
    uint64_t operator()() {
      static constexpr std::array<uint64_t, kSquares> magics = {
          612498416294952992ULL,  2377936612260610304ULL,  36037730568766080ULL,
          72075188908654856ULL,   144119655536003584ULL,   5836666216720237568ULL,
          9403535813175676288ULL, 1765412295174865024ULL,  3476919663777054752ULL,
          288300746238222339ULL,  9288811671472386ULL,     146648600474026240ULL,
          3799946587537536ULL,    704237264700928ULL,      10133167915730964ULL,
          2305983769267405952ULL, 9223634270415749248ULL,  10344480540467205ULL,
          9376496898355021824ULL, 2323998695235782656ULL,  9241527722809755650ULL,
          189159985010188292ULL,  2310421375767019786ULL,  4647717014536733827ULL,
          5585659813035147264ULL, 1442911135872321664ULL,  140814801969667ULL,
          1188959108457300100ULL, 288815318485696640ULL,   758869733499076736ULL,
          234750139167147013ULL,  2305924931420225604ULL,  9403727128727390345ULL,
          9223970239903959360ULL, 309094713112139074ULL,   38290492990967808ULL,
          3461016597114651648ULL, 181289678366835712ULL,   4927518981226496513ULL,
          1155212901905072225ULL, 36099167912755202ULL,    9024792514543648ULL,
          4611826894462124048ULL, 291045264466247688ULL,   83880127713378308ULL,
          1688867174481936ULL,    563516973121544ULL,      9227888831703941123ULL,
          703691741225216ULL,     45203259517829248ULL,    693563138976596032ULL,
          4038638777286134272ULL, 865817582546978176ULL,   13835621555058516608ULL,
          11541041685463296ULL,   288511853443695360ULL,   283749161902275ULL,
          176489098445378ULL,     2306124759338845321ULL,  720584805193941061ULL,
          4977040710267061250ULL, 10097633331715778562ULL, 325666550235288577ULL,
          1100057149646ULL,
      };
      return magics.at(i_++);
    }

  private:
    int i_ = 0;
  };

  class PreComputedBishopMagics {
  public:
    uint64_t operator()() {
      static constexpr std::array<uint64_t, kSquares> magics = {
          9368648609924554880ULL, 9009475591934976ULL,     4504776450605056ULL,
          1130334595844096ULL,    1725202480235520ULL,     288516396277699584ULL,
          613618303369805920ULL,  10168455467108368ULL,    9046920051966080ULL,
          36031066926022914ULL,   1152925941509587232ULL,  9301886096196101ULL,
          290536121828773904ULL,  5260205533369993472ULL,  7512287909098426400ULL,
          153141218749450240ULL,  9241386469758076456ULL,  5352528174448640064ULL,
          2310346668982272096ULL, 1154049638051909890ULL,  282645627930625ULL,
          2306405976892514304ULL, 11534281888680707074ULL, 72339630111982113ULL,
          8149474640617539202ULL, 2459884588819024896ULL,  11675583734899409218ULL,
          1196543596102144ULL,    5774635144585216ULL,     145242600416216065ULL,
          2522607328671633440ULL, 145278609400071184ULL,   5101802674455216ULL,
          650979603259904ULL,     9511646410653040801ULL,  1153493285013424640ULL,
          18016048314974752ULL,   4688397299729694976ULL,  9226754220791842050ULL,
          4611969694574863363ULL, 145532532652773378ULL,   5265289125480634376ULL,
          288239448330604544ULL,  2395019802642432ULL,     14555704381721968898ULL,
          2324459974457168384ULL, 23652833739932677ULL,    282583111844497ULL,
          4629880776036450560ULL, 5188716322066279440ULL,  146367151686549765ULL,
          1153170821083299856ULL, 2315697107408912522ULL,  2342448293961403408ULL,
          2309255902098161920ULL, 469501395595331584ULL,   4615626809856761874ULL,
          576601773662552642ULL,  621501155230386208ULL,   13835058055890469376ULL,
          3748138521932726784ULL, 9223517207018883457ULL,  9237736128969216257ULL,
          1127068154855556ULL,
      };
      return magics.at(i_++);
    }

  private:
    int i_ = 0;
  };

  static constexpr int kSquares = 64;
  // clang-format off
  static constexpr int rook_shifts_[kSquares] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12,
  };
  static constexpr int bishop_shifts_[kSquares] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
  };
  // clang-format on
  uint64_t rook_masks_[kSquares];
  uint64_t bishop_masks_[kSquares];
  uint64_t rook_magics_[kSquares];
  uint64_t bishop_magics_[kSquares];
  int rook_offsets_[kSquares];
  int bishop_offsets_[kSquares];
  std::vector<uint64_t> rook_attack_table_;
  std::vector<uint64_t> bishop_attack_table_;
};

} // namespace magic_bits

#endif
