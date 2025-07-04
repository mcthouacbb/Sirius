# Sirius

A UCI chess engine written in c++.
Sirius does not come with a gui. To play against it or use it, you should download a chess GUI or tournament runner that supports the Universal Chess Interface(UCI) protocol

## Strength
See [Releases](https://github.com/mcthouacbb/Sirius/releases)

| Version | Release Date | [CCRL Blitz](https://www.computerchess.org.uk/ccrl/404/) | [CCRL 40/15](https://www.computerchess.org.uk/ccrl/4040/) |
| --- | --- | --- | --- |
| 5.0 | 2023-10-27 | N/A | 2678 |
| 6.0 | 2024-02-17 | N/A | 2965 |
| 7.0 | 2024-07-09 | N/A | 3221 |
| 8.0 | 2024-10-05 | 3435 | 3357 |
| 9.0 | 2025-07-03 | N/A | N/A |

## Usage
Sirius can be used with any UCI Chess GUI or matchrunner including Arena, Cutechess, Cutechess-cli, Fastchess, Banksia, and more.
You can also play it on [Lichess](https://lichess.org/@/Sirius_Bot)(Though it's not online very often)

## Features
- Board representation
    - BitBoards
    - 8x8 Mailbox
    - Zobrist hashing
- Move Generation
    - Magic Bitboards for sliding pieces
    - Hybrid pseudo-legal/legal move generation
- Evaluation
    - Tapered Evaluation
    - Incrementally updated evaluation
    - Tempo bonus
    - Material
    - Piece Square Tables
      - Horizontally mirrored
    - Mobility
    - Threats
    - Knight outposts
    - Rook on semi-open/open file
    - Minors behind pawns
    - Bishop pair
    - Bishop same color pawns
    - Long diagonal bishop
    - Pawn Structure
        - Passed pawns
        - Isolated pawns
        - Backwards pawns
        - Doubled pawns
        - Defended pawns
        - Pawn phalanxes
        - Passed pawn Distance to Kings
        - Candidate passed pawns
    - King Safety
        - King-pawn storm/shield
        - Safe and unsafe checks from enemy pieces
        - King ring attacks
        - Weak squares in the king ring
        - Attacks and defenses to the king flank
        - Existence of attacking queen
        - Quadratic safety adjustment formula
    - Complexity eval
    - Endgame scaling
    - Specialized evaluation and scaling for known endgames
    - Tuning via Texel's Tuning Method
        - [https://github.com/mcthouacbb/Sirius-Tune-2](https://github.com/mcthouacbb/Sirius-Tune-2)
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
        - Continuation History
        - Capture History
    - Quiescence Search
        - SEE Pruning
        - Quiescence Search Futility Pruning
        - Quiescence Search Late Move Pruning
        - Quiet Check Evasions
    - Transposition Table
    - Selectivity
        - Check Extensions
        - Singular Extensions
            - Multicut
            - Double Extensions
            - Negative Extensions
        - Principal Variation Search(PVS)
            - 3 Fold LMR
        - Reverse Futility Pruning
        - Null Move Pruning
        - Futility Pruning
        - Noisy/Capture Futility Pruning
        - Late Move Pruning
        - History Pruning
        - SEE Pruning
        - Late Move Reductions
        - Internal Iterative Reductions
        - Probcut
    - Miscellaneous
        - Improving Heuristic
        - Node count time management
        - Best move stability time management
        - tt score adjustment
        - Various static evaluation correction histories
        - TTPV
        - Staged move generation
        - Lazy SMP

## Non-standard UCI commands
- `"d"`
    - Prints a string representation of the board from white's perspective
    - Prints out various statistics about the board
- `"perft <depth>"`
    - Runs a perft, which counts the number of leaf nodes of a brute force depth <depth> search
- `"eval"`
    - Prints the static evaluation of the current position
- `"bench"`
    - Runs an depth 15 search on a set of internal benchmark positions and prints out the number of nodes and number of nodes searched per second.

## UCI options
| Name             |  Type   | Default value |       Valid values        | Description                                                                          |
|:-----------------|:-------:|:-------------:|:-------------------------:|:------------------------------------------------------------------------------------:|
| UCI_Chess960     | boolean |   false       |        true, false        | Whether to enable chess960(usually set by the GUI/match runner)                      |
| UCI_ShowWDL      | boolean |    true       |        true, false        | Whether to show estimated win, draw, and loss probabilities when printing info       |
| Hash             | integer |      64       |       [1, 33554432]       | Size of the transposition table in Megabytes.                                        |
| Threads          | integer |       1       |        [1, 2048]          | Number of threads used to search.                                                    |
| MoveOverhead     | integer |      10       |         [1, 100]          | Amount of time subtracted to account for overhead between engine and gui.            |
| PrettyPrint      | boolean |   false       |        true, false        | Whether to pretty print uci output. Defaults to true if UCI is not first command     |

## Building
Do not use the Makefile, it is intended for building with OpenBench only

- C++20, CMake, and a decent C++ compiler required
- If you have ninja and clang, you can build the release builds by running the following commands
  ```
  cmake --preset ninja-clang-x86-64-v<version>
  cmake --build build/x86-64-<version>
  ```
  where `<version>` is 1-4.
  You can also use the other presets, though they are mainly a convenience feature.
- If you would like to build with your own settings, feel free to do so.
    - On their own, the CMake files only define what is absolutely necessary to build Sirius(With the exception of a flag that links msvc std lib statically), so you don't have to change the build files to build Sirius yourself

## Credits/Thanks
- [Sebastian Lague](https://www.youtube.com/@SebastianLague), for getting me into chess programming
- [The Chess Programming Wiki](https://www.chessprogramming.org/), very outdated but has some good ideas, especially for move generation/board representation
- [OpenBench](https://github.com/AndyGrant/OpenBench), an very well made distributed SPRT testing framework for chess engines
- [Stockfish](https://github.com/official-stockfish/Stockfish)
- [Ethereal](https://github.com/AndyGrant/Ethereal), one of the best references for chess programming
- [Berserk](https://github.com/jhonnold/berserk), another good reference engine
- [Weiss](https://github.com/TerjeKir/Weiss)
- [Stash](https://github.com/mhouppin/stash-bot)
- [Perseus](https://github.com/TheRealGioviok/Perseus-Engine)
- Crafty
- Zurichess
- The Engine Programming Discord Server, and the people in it
    - [@JW](https://github.com/jw1912), developer of [Akimbo](https://github.com/jw1912/akimbo), [Bullet](https://github.com/jw1912/bullet), and [Monty](https://github.com/official-monty/Monty), helped me a ton with developing and testing the engine
    - [@Ciekce](https://github.com/ciekce/), developer of [Stormphrax](https://github.com/ciekce/Stormphrax), who, along with JW, taught me many things about Chess Programming, and is an excellent c++ programmer
    - [@Alex2262](https://github.com/Alex2262), developer of [Altaire](https://github.com/Alex2262/AltairChessEngine)
    - [@Cj5716](https://github.com/cj5716/), general contributor to many engines
- The Stockfish Discord Server
- The [Sebastian Lague Chess-Challenge](https://github.com/seblague/Chess-challenge) Discord Server
	- Not a direct resource, but led to me finding many of the above mentioned resources
- [Rustic Chess Blog](https://rustic-chess.org/), currently a WIP but it has excellent explanations for the techniques it does explain.
- Many others

