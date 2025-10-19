#ifndef CHESSBOARDWIDGET_H
#define CHESSBOARDWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <map>
#include "src/ChessGame.h"

class ChessBoardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChessBoardWidget(QWidget *parent = nullptr);
    void setChessGame(ChessGame* cg) {chess_game = cg;}

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    ChessGame* chess_game = nullptr;
    std::vector<Move> moves;
    int selected_square = -1;
    QPixmap piece_pixmaps[2][6];

    void loadPieceSprites();
    void drawPieces(QPainter& painter, int square_size);
    int gui_to_bitboard_toggle(int index);
};

#endif // CHESSBOARDWIDGET_H
