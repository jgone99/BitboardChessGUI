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
    Bitboard legal_moves(Square square, std::vector<Move>* legal_moves_list = nullptr);
    void make_move(const Move& move);
    bool is_friendly_square(Square square);
    void previous_position();
    void next_position();

private:
    int position_index;
    int position_history_size;
};

