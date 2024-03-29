#include <iostream>
#include <string>

#include "attacks.h"
#include "zobrist.h"
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
    zobrist::init();
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
    else if (mode == "uci")
    {
        comm::UCI uci;
        comm::currComm = &uci;
        uci.run();
    }
#ifdef EXTERNAL_TUNE
    else if (mode == "wfconfig")
    {
        search::printWeatherFactoryConfig();
    }
#endif
    else
    {
        std::cout << "Unrecognized mode" << std::endl;
    }
    return 0;
}
