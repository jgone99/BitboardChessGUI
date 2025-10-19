#include "mainwindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* layout = new QVBoxLayout(central);

    setFixedSize(820, 820);

    chess_board_widget = new ChessBoardWidget(this);
    chess_game = new ChessGame();
    chess_board_widget->setChessGame(chess_game);

    layout->addWidget(chess_board_widget);
}

MainWindow::~MainWindow()
{
    delete chess_board_widget;
    delete chess_game;
}
