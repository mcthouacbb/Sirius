#include <iostream>
#include <string>

#include "attacks.h"
#include "bench.h"
#include "comm/icomm.h"
#include "comm/uci.h"
#include "cuckoo.h"
#include "eval/endgame.h"
#include "eval/eval.h"
#include "search_params.h"
#include "sirius.h"

int main(int argc, char** argv)
{
    attacks::init();
    cuckoo::init();
    search::init();
    eval::endgames::init();

    if (argc > 1 && std::string(argv[1]) == "bench")
    {
        Board board;
        std::unique_ptr<search::Search> bencher = std::make_unique<search::Search>(board);
        runBench(*bencher, BENCH_DEPTH);
        return 0;
    }

    std::string mode;
    std::getline(std::cin, mode);

#ifdef EXTERNAL_TUNE
    else if (mode == "wfconfig")
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
        comm::UCI uci;
        comm::currComm = &uci;
        uci.run(mode);
    }
    return 0;
}
