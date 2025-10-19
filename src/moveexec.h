#ifndef MOVEEXEC_H
#define MOVEEXEC_H

#include "position.h"

void update_game_state(Position& position);
void test_move(Move& move, Position& position);
void make_move(const Move& move, Position& position);

#endif // MOVEEXEC_H
