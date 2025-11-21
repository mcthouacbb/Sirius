#pragma once

#include <cstdint>
#include <mutex>
#include <string>

#include "../defs.h"

namespace datagen
{

struct Config
{
    u32 softLimit;
    u32 hardLimit;
    u32 numGames;
    u32 numThreads;
    std::string outputFilename;
};

void runDatagen(Config config);

}
