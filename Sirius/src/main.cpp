#include <iostream>
#include <string>

#include "attacks.h"
#include "comm/icomm.h"
#include "comm/cmdline.h"
#include "comm/uci.h"
#include "eval/eval.h"
#include "search_params.h"
#include "bench.h"
#include "sirius.h"

int main(int argc, char** argv)
{
    attacks::init();
    search::init();

    if (argc > 1 && std::string(argv[1]) == "bench")
    {
        Board board;
        std::unique_ptr<search::Search> bencher = std::make_unique<search::Search>(board);
        runBench(*bencher, BENCH_DEPTH);
        return 0;
    }

    std::string mode;
    std::getline(std::cin, mode);

    if (mode == "cmdline")
    {
        comm::CmdLine cmdLine;
        comm::currComm = &cmdLine;
        cmdLine.run();
    }
#ifdef EXTERNAL_TUNE
    else if (mode == "wfconfig")
    {
        search::printWeatherFactoryConfig();
    }
    else if (mode == "obconfig")
    {
        search::printOpenBenchConfig();
    }
#endif
    else
    {
        comm::UCI uci;
        comm::currComm = &uci;
        uci.run(mode);
    }
    return 0;
}
