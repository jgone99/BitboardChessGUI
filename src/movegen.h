#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "position.h"
#include <vector>

enum Direction {
    NORTH		= 0,
    NORTHEAST	= 1,
    EAST		= 2,
    SOUTHEAST	= 3,
    SOUTH		= 4,
    SOUTHWEST	= 5,
    WEST		= 6,
    NORTHWEST	= 7
};

// For initial pre-computation of ray_attacks
extern const int DIRECTION_OFFSETS[8][2];

// Pre-computed ray attacks for sliding pieces in all 8 directions
extern Bitboard ray_attacks[8][64];

// Optimizatin Option: pre-compute with magic bitboards
// Pre-computes the attacking rays for the sliding pieces. A middle-ground approach between a per-move calculation and a magic bitboard solution
void init_rays();

inline Bitboard north_one(Bitboard bitboard) { return north_shift(bitboard, 1); }
inline Bitboard north_east_one(Bitboard bitboard) { return north_east_shift(bitboard, 1) & ~A_FILE; }
inline Bitboard east_one(Bitboard bitboard) { return east_shift(bitboard, 1) & ~A_FILE; }
inline Bitboard south_east_one(Bitboard bitboard) { return south_east_shift(bitboard, 1) & ~A_FILE; }
inline Bitboard south_one(Bitboard bitboard) { return south_shift(bitboard, 1); }
inline Bitboard south_west_one(Bitboard bitboard) { return south_west_shift(bitboard, 1) & ~H_FILE; }
inline Bitboard west_one(Bitboard bitboard) { return west_shift(bitboard, 1) & ~H_FILE; }
inline Bitboard north_west_one(Bitboard bitboard) { return north_west_shift(bitboard, 1) & ~H_FILE; }

Bitboard get_positive_ray_attacks(Bitboard occupied, Direction dir, Square square);
Bitboard get_negative_ray_attacks(Bitboard occupied, Direction dir, Square square);
Bitboard diagonal_attacks(Bitboard occupied, Square square);
Bitboard anti_diagonal_attacks(Bitboard occupancy, Square square);
Bitboard single_push(Bitboard pawns, Bitboard empty, int color);
Bitboard double_push(Bitboard pawns, Bitboard empty, int color);

Bitboard file_attacks(Bitboard occupancy, Square square);
Bitboard rank_attacks(Bitboard occupancy, Square square);

// Piece attacks
Bitboard rook_attacks(Bitboard occupancy, Square square);
Bitboard bishop_attacks(Bitboard occupancy, Square square);
Bitboard queen_attacks(Bitboard occupancy, Square square);
Bitboard knight_attacks(Bitboard knights);
Bitboard king_attacks(Bitboard king);
Bitboard pawn_attacks(Bitboard square, int color);

// Piece moves
Bitboard pawn_moves(const Position& position, Square square, int color);
Bitboard rook_moves(const Position& position, Square square, int color);
Bitboard bishop_moves(const Position& position, Square square, int color);
Bitboard knight_moves(const Position& position, Square square, int color);
Bitboard queen_moves(const Position& position, Square square, int color);
Bitboard king_moves(const Position& position, Square square, int color);

Bitboard castling_moves(const Position& position, Color color);
Bitboard moves(Square square, const Position& position);

// Generate legal moves of a piece given a square
Bitboard legal_moves(const Position& position, Square square, std::vector<Move>* legal_moves_list = nullptr);

bool is_valid_square(const Position& position, Square square);
bool is_friendly_square(const Position& position, Square square);

bool is_attacked(const Position& position, Square square, int attacking_color);
Bitboard attacked_by(const Position& position, Square square, int attacking_color);
bool is_king_in_check(const Position& position, int attacking_color);

// Magic Bitnboard implementation (WIP)

//void init_magics(bool is_rook, Bitboard piece_table[], Magic magics[]);
//struct Magic
//{
//	Bitboard* attacks;
//	Bitboard mask;
//	Bitboard magic;
//	int shift;

//	unsigned index(Bitboard occ)
//	{
//		return unsigned(((occ & mask) * magic) >> shift);
//	}
//};

//Bitboard rook_table[0x19000];
//Bitboard bishop_table[0x1480];

//Magic rook_magics[64];
//Magic bishop_magics[64];

#endif // MOVEGEN_H
