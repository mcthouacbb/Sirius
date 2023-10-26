# Sirius

v4.0

## Features
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
            - History Malus
            - History Gravity
    - Quiescence Search
        - SEE Pruning
    - Transposition Table
        - Stockfish/Ethereal replacement scheme
        - Cutoffs
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

## CLI Usage
- Type "uci" for the UCI protocol(not recommended for direct use, usually used by a chess GUI)
    - Protocol is explained [here](https://backscattering.de/chess/uci)
- Type "cmdline" for the CLI protocol(Mainly for convenience in certain situations)
    - Protocol is explained [below](#command-line-protocol)

## Non-standard UCI commands
- `"d"`
    - Prints a string representation of the board from white's perspective
- `"bench" <depth>`
    - Runs an <depth> depth search on a set of internal benchmark positions and prints out the number of nodes and the time token.

## UCI options
| Name             |  Type   | Default value |       Valid values        | Description                                                                          |
|:-----------------|:-------:|:-------------:|:-------------------------:|:-------------------------------------------------------------------------------------|
| Hash             | integer |      64       |        [1, 2048]          | Size of the transposition table in Megabytes.                                        |
| Threads          | integer |       1       |         [1, 1]            | Number of threads used to search (currently does nothing).                           |

## Building
- It's just CMake lol
- C++20 required

## Credits/Thanks
- [The Chess Programming Wiki](https://www.chessprogramming.org/), a bit outdated but nonetheless an excellent resource
- [Stockfish](https://github.com/official-stockfish/Stockfish)
- [Ethereal](https://github.com/AndyGrant/Ethereal), one of the best references for chess programming
- Crafty
- Zurichess
- The Engine Programming Discord Server, and the people in it
    - [@JW](https://github.com/jw1912), developer of [Akimbo](https://github.com/jw1912/akimbo), helped me a ton with developing and testing the engine
    - [@Ciekce](https://github.com/ciekce/), developer of [Stormphrax](https://github.com/ciekce/Stormphrax), who, along with JW, taught me many things about Chess Programming, and is an excellent c++ programmer
    - [@Alex2262](https://github.com/Alex2262), developer of [Altaire](https://github.com/Alex2262/AltairChessEngine)
- The Stockfish Discord Server
- The [Sebastian Lague Chess-Challenge](https://github.com/seblague/Chess-challenge) Discord Server
	- Not a direct resource, but led to me finding many of the above mentioned resources
- [Rustic Chess Blog](https://rustic-chess.org/), currently a WIP but it has excellent explanations for the techniques it does explain.
- Many others


<div id="command-line-protocol"></div>
## Command Line Protocol
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
    - Prints the quiescence search evaluation of the position
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
