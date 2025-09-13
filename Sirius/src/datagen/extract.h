#pragma once

#include <cstdint>
#include <string>

namespace datagen
{

void extract(std::string dataFile, std::string outFile, uint32_t maxGames, uint32_t ppg);

}
