# Sirius

v4.0

CLI Usage
- Type "uci" for the UCI protocol(not recommended for direct use, usually used by a chess GUI)
    - Protocol is explained [here](https://backscattering.de/chess/uci)
- Type "cmdline" for the CLI protocol(Mainly for convenience in certain situations)
    - Protocol is explained below

Command Line Protocol(for debugging/convenience)
- `"position" {"fen" | "startpos"} [fenString]`
    - Set the board position to the starting position or the fenString
- `"print"`
    - Print the current state of the board
          - Piece positions
        - Number of plies since the start of the game(starts at 0)
        - Half Move Clock
            - Used to detect 50 move rule draws
            - Draw at 100 half moves
            - Castling Rights
          - Side to move
        - Square of en passant, if available
        - Zobrist hash
- `"move" <move>`
    - makes a move
    - Standard Algebraic Notation(FIDE notation)
    - Square is a file (a-h) and rank(1-8)
    - Promotion piece is either, q(queen), r(rook), b(bishop), or n(knight)
- `"undo"`
    - Undo the last move that was made
- `"eval"`
    - Prints the static evaluation of the position
- `"qeval"`
    - Prints the quiescence evaluation of the position
    - Currently not working
- `"search" "depth" <depth>`
- `"search" "time" <time>`
- `"search" "infinite"`
    - Performs an iterative deepening search up to depth, until time time, or until interrupted
    - Prints out the evaluation, node count, and PV of each depth
- `"stop"`
    - Stops the search
- `"tests"`
    - Runs test suite
    - Currently, only perft tests are run
- `"perft" <depth>`
    - Performs are perft up to depth
    - A perft(performance test) searches all moves up to depth and returns the number of positions reached
    - WARNING: time usage increases exponentially with depth
- `"book"`
    - Returns all the moves in the opening book
    - Opening book is currently hardcoded to "Sirius/res/gaviota_trim.pgn"
    - Prints "No moves in book found" if position is not in book

Non-standard UCI commands
- `"d"`
    - Prints a string representation of the board from white's perspective
- `"bench" <depth>`
    - Runs an <depth> depth search on a set of internal benchmark positions and prints out the number of nodes and the time token.

Features
- Board representation
    - BitBoards
    - Mailbox 0x88
    - Zobrist hashing
    - Packed 16 bit Move Representation
    - Static Exchange Evaluation
- Move Generation
    - Magic Bitboards for sliding pieces
    - Legal move generation
- Evaluation
    - Tapered Evaluation
    - Material
    - Piece Square Tables
    - Tuning via Texel's Tuning Method
- Search
    - Fail-soft Alpha-Beta Pruning
    - Iterative Deepening
    - Aspiration Windows
    - Mate Distance Pruning
    - Move Ordering
        - TT Move Ordering
        - MVV LVA
        - SEE Move Ordering
        - Killer Moves Heuristic
        - History Heuristic
    - Quiescence Search
        - SEE Pruning
    - Transposition Table
        - Stockfish/Ethereal replacement scheme
    - Selectivity
        - Check Extension
        - Principal Variation Search(PVS)
            - 2 Fold LMR
        - Reverse Futility Pruning
        - Null Move Pruning
        - Futility Pruning
        - Late Move Pruning
        - SEE pruning
        - Late Move Reductions

Inspired/helped by
- [Stockfish](https://github.com/official-stockfish/Stockfish)
- [Ethereal](https://github.com/AndyGrant/Ethereal), one of the best references for chess engines
- Crafty
- Zurichess
- [The Chess Programming Wiki](https://www.chessprogramming.org/), a bit outdated but nonetheless an excellent resource
- [@JW](https://github.com/jw1912), developer of [Akimbo](https://github.com/jw1912/akimbo), helped me a ton with developing and testing the engine
- [@Ciekce](https://github.com/ciekce/), developer of [Stormphrax](https://github.com/ciekce/Stormphrax), who, along with JW, taught me many things about Chess Programming, and an excellent c++ programmer
- [@Alex2262][https://github.com/Alex2262], developer of [Altaire](https://github.com/Alex2262/AltairChessEngine)
- Many others
