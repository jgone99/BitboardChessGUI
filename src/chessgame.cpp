#include "chessgame.h"
#include "moveexec.h"
#include "movegen.h"
#include <string>

ChessGame::ChessGame(Position position)
{
    current_position = position;

    position_index = 0;
    position_history.push_back(current_position);
    position_history_size = 1;

    position_history_3fold[pos_stringid(position)] = 1;
	init_rays();
	update_occupancies(position);
}

ChessGame::ChessGame(std::string fen) : ChessGame(fen_to_pos(fen)) {}

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

Bitboard ChessGame::legal_moves(Square square, std::vector<Move> *legal_moves_list)
{
    return ::legal_moves(current_position, square, legal_moves_list);
}

void ChessGame::make_move(const Move &move)
{
    ::make_move(move, current_position);
    position_history_size = ++position_index;
    if (position_history.size() > position_history_size)
        position_history[position_history_size] = current_position;
    else
        position_history.push_back(current_position);
}

bool ChessGame::is_friendly_square(Square square)
{
    return ::is_friendly_square(current_position, square);
}

void ChessGame::previous_position()
{
    position_index = std::max(0, position_index - 1);
    current_position = position_history[position_index];
}

void ChessGame::next_position()
{
    position_index = std::min(position_history_size, position_index + 1);
    current_position = position_history[position_index];
}
