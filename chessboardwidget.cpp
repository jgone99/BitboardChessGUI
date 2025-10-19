#include "chessboardwidget.h"
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>

ChessBoardWidget::ChessBoardWidget(QWidget *parent)
    : QWidget{parent}
{
    loadPieceSprites();
}

void ChessBoardWidget::loadPieceSprites()
{
    piece_pixmaps[WHITE][PAWN]   = QPixmap(":/assets/white_pawn.png");
    piece_pixmaps[BLACK][PAWN]   = QPixmap(":/assets/black_pawn.png");
    piece_pixmaps[WHITE][ROOK]   = QPixmap(":/assets/white_rook.png");
    piece_pixmaps[BLACK][ROOK]   = QPixmap(":/assets/black_rook.png");
    piece_pixmaps[WHITE][KNIGHT] = QPixmap(":/assets/white_knight.png");
    piece_pixmaps[BLACK][KNIGHT] = QPixmap(":/assets/black_knight.png");
    piece_pixmaps[WHITE][BISHOP] = QPixmap(":/assets/white_bishop.png");
    piece_pixmaps[BLACK][BISHOP] = QPixmap(":/assets/black_bishop.png");
    piece_pixmaps[WHITE][QUEEN]  = QPixmap(":/assets/white_queen.png");
    piece_pixmaps[BLACK][QUEEN]  = QPixmap(":/assets/black_queen.png");
    piece_pixmaps[WHITE][KING]   = QPixmap(":/assets/white_king.png");
    piece_pixmaps[BLACK][KING]   = QPixmap(":/assets/black_king.png");
}

void ChessBoardWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    int square_size = std::min(width(), height()) / 8;
    int padding = square_size * 0.1;
    int piece_size = square_size - 2 * padding;
    int offset = (square_size - piece_size) / 2;

    // 1. Draw board squares
    for (int rank = 0; rank < 8; ++rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            bool light = (rank + file) % 2 == 0;
            QColor color = light ? QColor(240, 217, 181) : QColor(181, 136, 99);
            painter.fillRect(file * square_size, rank * square_size, square_size, square_size, color);
        }
    }

    // 2. Draw pieces using bitboards
    for (int color = 0; color < 2; ++color)
    {
        for (int type = 0; type < 6; ++type)
        {
            Bitboard bb = chess_game->current_position.pieces[color][type];

            while (bb)
            {
                int sq = __builtin_ctzll(bb);      // bitscan forward
                bb &= bb - 1;                      // clear LSB

                int rank = sq / 8;
                int file = sq % 8;

                QPixmap pieceImage = piece_pixmaps[color][type].scaled(piece_size, piece_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QRect targetRect(file * square_size + padding, (7 - rank) * square_size + padding, piece_size, piece_size);
                painter.drawPixmap(targetRect, pieceImage);
            }
        }
    }

    // // Draw squares
    // for (int row = 0; row < 8; ++row) {
    //     for (int col = 0; col < 8; ++col) {
    //         int idx = row * 8 + col;
    //         bool light = (row + col) % 2 == 0;
    //         painter.setPen(Qt::NoPen);
    //         painter.setBrush(light ? QColor(240, 217, 181) : QColor(181, 136, 99));
    //         painter.drawRect(col * square_size, row * square_size, square_size, square_size);

    //         // Get piece from your board logic
    //         const Piece* piece = board->getSquare(row * 8 + col)->piece;
    //         QString type;

    //         if (piece) {
    //             switch (piece->type) {
    //                 case Piece::PieceType::KING: type = "K"; break;
    //                 case Piece::PieceType::QUEEN: type = "Q"; break;
    //                 case Piece::PieceType::ROOK: type = "R"; break;
    //                 case Piece::PieceType::BISHOP: type = "B"; break;
    //                 case Piece::PieceType::KNIGHT: type = "N"; break;
    //                 case Piece::PieceType::PAWN: type = "P"; break;
    //                 default: type = ""; break;
    //             }

    //             QString key = QString("%1_%2")
    //             .arg(type).arg(piece->isWhite ? "white" : "black");
    //             painter.drawPixmap(col * square_size + offset, row * square_size + offset, piece_size, piece_size, pieceSprites[key.toStdString()]);
    //         }
    //     }
    // }

    if (selected_square != -1) {
        int selRow = selected_square / 8;
        int selCol = selected_square % 8;
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::yellow, 3));
        painter.drawRect(selCol*square_size, selRow*square_size, square_size, square_size);

        painter.setPen(QPen(Qt::blue, 3));
        for (Move move: moves)
        {
            int gui_index = gui_to_bitboard_toggle(move.to);
            int col = gui_index % 8;
            int row = gui_index / 8;

            painter.drawRect(col * square_size, row * square_size, square_size, square_size);
        }
    }
}

void ChessBoardWidget::mousePressEvent(QMouseEvent* event)
{
    int square_size = std::min(width(), height()) / 8;
    int col = event->position().x() / square_size;
    int row = event->position().y() / square_size;
    int index = row * 8 + col;
    int bb_index = gui_to_bitboard_toggle(index);

    if (selected_square == -1) {
        // First click: select a square if it has a piece of the current turn
        if (chess_game->is_valid_square(Square(bb_index)))
        {
            selected_square = index;
            chess_game->legal_moves(Square(bb_index), &moves);
        }
    } else {
        // Second click: attempt move
        int from = gui_to_bitboard_toggle(selected_square);
        int to = bb_index;

        if (from != to) {
            std::vector<Move>::iterator it = find_if(moves.begin(), moves.end(), [from, to](Move m) { return m.from == from && m.to == to; });
            if (it != moves.end())
            {
                chess_game->make_move(*it);
                selected_square = -1;
            }
            else if (chess_game->is_friendly_square(Square(to))) {
                selected_square = index;
                chess_game->legal_moves(Square(to), &moves);
            }
            else
            {
                selected_square = -1;
            }
        }
    }

    update();
}

inline int ChessBoardWidget::gui_to_bitboard_toggle(int index)
{
    int row = index / 8;     // GUI row 0 = rank 8
    int col = index % 8;
    return (7 - row) * 8 + col;  // flip vertically
}
