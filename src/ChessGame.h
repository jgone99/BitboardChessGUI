#pragma once
#include "position.h"
#include <map>
#include <vector>

class ChessGame
{
public:
    std::vector<Position> position_history;

    std::map<std::string, int> position_history_3fold;
    Position current_position;

	ChessGame(Position position = starting_position);
	ChessGame(std::string fen);

    PieceType get_type(int square, int color);
    bool is_valid_square(int square);
    void legal_moves(Square square, std::vector<Move>* legal_moves_list = nullptr);
    void make_move(const Move& move);
    bool is_friendly_square(Square square);

	// Magic Bitnboard implementation (WIP)
	
	//static void init_magics(bool is_rook, U64 piece_table[], Magic magics[]);
	//struct Magic
	//{
	//	U64* attacks;
	//	U64 mask;
	//	U64 magic;
	//	int shift;

	//	unsigned index(U64 occ)
	//	{
	//		return unsigned(((occ & mask) * magic) >> shift);
	//	}
	//};

	//U64 rook_table[0x19000];
	//U64 bishop_table[0x1480];

	//Magic rook_magics[64];
	//Magic bishop_magics[64];

};

