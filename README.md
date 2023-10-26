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
    - Protocol is explained [here](https://github.com/mcthouacbb/Sirius/CmdLine.md)

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
- [Stockfish](https://github.com/official-stockfish/Stockfish), the best chess engine
- [Ethereal](https://github.com/AndyGrant/Ethereal), one of the best references for chess engines
- Crafty
- Zurichess
- [The Chess Programming Wiki](https://www.chessprogramming.org/), a bit outdated but nonetheless an excellent resource
- [@JW](https://github.com/jw1912), developer of [Akimbo](https://github.com/jw1912/akimbo), helped me a ton with developing and testing the engine
- [@Ciekce](https://github.com/ciekce/), developer of [Stormphrax](https://github.com/ciekce/Stormphrax), who, along with JW, taught me many things about Chess Programming, and is an excellent c++ programmer
- [@Alex2262](https://github.com/Alex2262), developer of [Altaire](https://github.com/Alex2262/AltairChessEngine)
- Many others
