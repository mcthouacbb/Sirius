#pragma once

#include <cstdint>
#include <mutex>
#include <string>

namespace datagen
{

struct Config
{
    uint32_t softLimit;
    uint32_t hardLimit;
    uint32_t numGames;
    uint32_t numThreads;
    std::string outputFilename;
};

void runDatagen(Config config);

}
