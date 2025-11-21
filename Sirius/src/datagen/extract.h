#pragma once

#include <cstdint>
#include <string>

#include "../defs.h"

namespace datagen
{

void extract(std::string dataFile, std::string outFile, u32 maxGames, u32 ppg);

}
