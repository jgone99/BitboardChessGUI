#ifndef MOVEEXEC_H
#define MOVEEXEC_H

#include "position.h"

void update_game_state(Position& position);
void test_move(Move& move, Position& position);
void make_move(const Move& move, Position& position);

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


#endif // MOVEEXEC_H
