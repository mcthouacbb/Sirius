#include <iostream>
#include <string>

#include "attacks.h"
#include "bench.h"
#include "cuckoo.h"
#include "eval/endgame.h"
#include "eval/eval.h"
#include "misc.h"
#include "search_params.h"
#include "sirius.h"
#include "uci/move.h"
#include "uci/uci.h"
#include "util/string_split.h"

int main(int argc, char** argv)
{
    attacks::init();
    cuckoo::init();
    search::init();
    eval::endgames::init();

    if (argc > 1 && std::string(argv[1]) == "bench")
    {
        std::unique_ptr<search::Search> bencher = std::make_unique<search::Search>();
        runBench(*bencher, BENCH_DEPTH);
        finalizeSuite();
        return 0;
    }

    for (;;)
    {
        std::string stuff;
        std::getline(std::cin, stuff);
        auto segments = splitBySpaces(stuff);
        auto fen = segments[0];
        fen += ' ' + segments[1];
        fen += ' ' + segments[2];
        fen += ' ' + segments[3];
        fen += ' ' + segments[4];
        fen += ' ' + segments[5];

        Board board;
        board.setToFen(fen);

        MoveList legals;
        genMoves<MoveGenType::LEGAL>(board, legals);
        Move move = uci::findMoveFromUCI(board, legals, segments[6].c_str()).move;
        if (move == Move::nullmove())
        {
            std::cout << "MOVE NOT FOUND" << std::endl;
        }
        std::cout << "\"true\" SEE score: " << fullyLegalSEE(board, move).score << std::endl;
        std::cout << "Current SEE score: " << seeExact(board, move) << std::endl;
    }

    return 0;

    std::string mode;
    std::getline(std::cin, mode);

#ifdef EXTERNAL_TUNE
    if (mode == "wfconfig")
    {
        search::printWeatherFactoryConfig();
    }
    else if (mode == "obconfig")
    {
        search::printOpenBenchConfig();
    }
    else
#endif
    {
        uci::UCI uci;
        uci::uci = &uci;
        uci.run(mode);
    }
    return 0;
}
