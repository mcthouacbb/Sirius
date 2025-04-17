#include <iostream>
#include <string>

#include "attacks.h"
#include "bench.h"
#include "cuckoo.h"
#include "eval/endgame.h"
#include "eval/eval.h"
#include "search_params.h"
#include "sirius.h"
#include "uci/uci.h"

int main(int argc, char** argv)
{
    attacks::init();
    cuckoo::init();
    search::init();
    //eval::endgames::init();

    if (argc > 1 && std::string(argv[1]) == "bench")
    {
        std::unique_ptr<search::Search> bencher = std::make_unique<search::Search>();
        runBench(*bencher, BENCH_DEPTH);
        return 0;
    }

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
