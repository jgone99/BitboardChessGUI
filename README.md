<h1 align="center">♟️ Bitboard Chess GUI</h1>
<p align="center">
  A modern C++ & Qt-based chess game using an efficient bitboard implementation.<br>
  Built for speed, clarity, and an elegant interactive interface.
</p>

---

<p align="center">
  <img width="802" height="832" alt="Screenshot 2025-10-15 170645" src="https://github.com/user-attachments/assets/b1f61f6c-42ea-4296-88a9-e9edf4a96848" />
</p>

---

## 🧩 Overview

This project is a **fully functional chess game** built with **C++17** and **Qt 6**, implementing the game logic using **bitboards** for high performance and clean move generation.  
It includes a **graphical user interface (GUI)** that supports:

- Piece selection and highlighting of legal moves  
- Move execution with real-time board updates  
- Game state management (turns, captures, etc.)  
- Optional move history navigation (undo/redo or click to restore past states)

---

## 🧠 Core Features

| Category | Description |
|-----------|--------------|
| **Bitboard Engine** | Fast internal board representation using 64-bit integers |
| **Move Generation** | Pseudo-legal and legal move computation with checks |
| **Qt GUI** | Smooth and interactive board rendering with scaling and highlights |
| **Game Logic** | Handles turn switching, captures, check detection, and special moves |
| **Scalable Design** | Modular classes for easy AI integration later |

---

## 🔮 Future Features

The following features are planned for future development:

| Feature | Description |
|---------|-------------|
| **En Passant** | Full support for en passant captures, including proper move generation and legality checks. |
| **Castling** | King-side and queen-side castling with move legality enforcement (including checks for squares under attack). |
| **Promotion Options** | Visual and logical handling of pawn promotion with choice of piece. |
| **Move History Navigation** | Undo/redo moves and click-to-restore previous positions in the GUI. |
| **AI Opponent** | Optional AI opponent using bitboard move evaluation for single-player mode. |
