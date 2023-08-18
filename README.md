# Sirius

v4.0

Minimal UCI Support

Inspired/helped by
- Stockfish
- Ethereal
- Crafty
- Zurichess
- [The Chess Programming Wiki](https://www.chessprogramming.org/)
- [TalkChess/The Computer Chess Club](https://www.talkchess.com/forum3/viewforum.php?f=2)
- [People in this OpenBench Instance](https://chess.swehosting.se/index/)
    - [@JW](https://github.com/jacquesRW/)
    - [@Ciekce](https://github.com/ciekce/)
- Many others

CLI Usage
- Type "uci" for the UCI protocol(not recommended for direct use, usually used by a chess GUI)
    - Protocol is explained [here](https://www.wbec-ridderkerk.nl/html/UCIProtocol.html)
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
- `"search" <depth>`
    - Performs an iterative deepening search up to depth
    - Prints out the evaluation and PV of each depth
    - Prints out search statistics
    - WARNING: Search time increases exponentially with depth
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
    - PV Collection(pv list on stack)
    - Iterative Deepening
    - Aspiration Windows
    - Move Ordering
        - TT Move Ordering
        - MVV LVA
        - Killer Moves Heuristic
        - History Heuristic
    - Quiescence Search
        - Captures Only
        - SEE Pruning
    - Transposition Table
        - 4 entries per bucket
        - Always replace least depth
    - Selectivity
        - Check Extension
        - Mate Distance Pruning
        - Principal Variation Search(PVS)
        - Reverse Futility Pruning
        - Null Move Pruning
        - Futility Pruning
        - Late Move Reductions
        - SEE pruning