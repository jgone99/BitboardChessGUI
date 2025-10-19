#include "chessgame.h"
#include "moveexec.h"
#include "movegen.h"
#include <string>

ChessGame::ChessGame(Position position)
{
	current_position = position;
	position_history_3fold[pos_stringid(position)] = 1;
	init_rays();
	update_occupancies(position);
}

ChessGame::ChessGame(std::string fen) : ChessGame(fen_to_pos(fen))
{
}

PieceType ChessGame::get_type(int square, int color)
{
    Bitboard square_bb = (1ULL << square);
	int type;
    for (type = 0; type < KING && !(square_bb & current_position.pieces[color][type]); ++type);
	return (PieceType)type;
}

bool ChessGame::is_valid_square(int square)
{
    return ::is_valid_square(current_position, Square(square));
}

void ChessGame::legal_moves(Square square, std::vector<Move> *legal_moves_list)
{
    ::legal_moves(current_position, square, legal_moves_list);
}

void ChessGame::make_move(const Move &move)
{
    ::make_move(move, current_position);
}

bool ChessGame::is_friendly_square(Square square)
{
    return ::is_friendly_square(current_position, square);
}

//void ChessGame::update_game_status()
//{
//	std::cout << pos_stringid(current_position) << std::endl;
//	if ((position_history_3fold[pos_stringid(current_position)] += 1) > 2)
//	{
//		current_position.state = REPETITION;
//		return;
//	}
//
//	bool is_black = current_position.color_to_move;
//	int king_square = bit_scan_forward(current_position.pieces[is_black][KING]);
//	U64 king_check_mask = queen_moves_mask(king_square, current_position, is_black);
//
//	U64 rook_check;
//	U64 bishop_check;
//	U64 knight_check;
//
//	U64 checks = (rook_check = king_check_mask & rook_mask(king_square) & (current_position.pieces[!is_black][ROOK] | current_position.occupancy[QUEEN]))
//		| (bishop_check = king_check_mask & bishop_mask(king_square) & current_position.occupancy[!is_black] & (current_position.pieces[0][BISHOP] | current_position.pieces[1][BISHOP] | current_position.pieces[0][QUEEN] | current_position.pieces[1][QUEEN]))
//		| (knight_check = knight_moves_mask(king_square, current_position, is_black) & current_position.pieces[is_black][KNIGHT]);
//
//	bool king_can_move = king_moves(current_position, is_black);
//
//	if (checks)
//	{
//		bool double_check = checks & (checks - 1);
//		if (king_can_move)
//		{
//			current_position.state = CHECK;
//		}
//		else
//		{
//			if (double_check)
//			{
//				current_position.state = CHECKMATE;
//			}
//			else
//			{
//				U64 pin_path = 0ULL;
//				U64 moves_bb = 0ULL;
//
//				if (knight_check) pin_path = knight_check;
//				if (bishop_check) pin_path = king_check_mask & bishop_mask(king_square) & bishop_moves_mask(bit_scan_forward(bishop_check), current_position, !is_black) | bishop_check;
//				if (rook_check) pin_path = king_check_mask & rook_mask(king_square) & rook_moves_mask(bit_scan_forward(rook_check), current_position, !is_black) | rook_check;
//
//				for (int type = PAWN; type <= QUEEN; type++)
//				{
//					U64 type_bb = current_position.pieces[is_black][type];
//					while (type_bb)
//					{
//						int square = bit_scan_forward(type_bb);
//						moves_bb |= moves(square, current_position, is_black) & pin_path;
//						type_bb &= type_bb - 1;
//					}
//				}
//
//				current_position.state = moves_bb ? CHECK : CHECKMATE;
//				current_position.checking_path_bb = pin_path;
//			}
//		}
//	}
//	else if (!king_can_move)
//	{
//		U64 bb = 0ULL;
//
//		for (int type = PAWN; type <= QUEEN; type++)
//		{
//			U64 type_bb = current_position.pieces[is_black][type];
//			while (type_bb)
//			{
//				int square = bit_scan_forward(type_bb);
//
//				bb |= moves(square, current_position, is_black);
//
//				type_bb &= type_bb - 1;
//			}
//		}
//
//		current_position.state = bb ? NORMAL : STALEMATE;
//	}
//	else
//	{
//		current_position.state = NORMAL;
//	}
//}



/*
	Magic Bitboard implementation (WIP)
*/

/*
void ChessGame::init_magics(bool is_rook, U64 piece_table[], Magic magics[])
{
	int size = 0;
	U64 b = 0;
	U64 occ[4096];
	U64 ref[4096];
	for (int sq = a1; sq <= h8; sq++)
	{
		U64 edges_ex = (((fifth_rank | eighth_rank) & ~rank_mask(sq)) | ((a_file | h_file) & ~file_mask(sq)));

		Magic& m = magics[sq];
		m.mask = (is_rook ? rook_mask_ex(sq) : bishop_mask_ex(sq)) & ~edges_ex;
		m.shift = 64 - pop_count(m.mask);
		m.attacks = sq == a1 ? piece_table : magics[sq - 1].attacks + size;

		b = size = 0;

		do
		{
			occ[size] = b;
			ref[size] = is_rook ? rook_attack(sq, b) : bishop_attack(sq, b);
			size++;
			b = (b - m.mask) & m.mask;
		} while (b);
	}
}
*/
