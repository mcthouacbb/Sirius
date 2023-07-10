#pragma once

#include "error.h"
#include <fstream>

namespace tune
{

EvalParams localOptimize(const EvalParams& initial, const std::vector<Pos>& positions, std::ofstream& outFile, int start);
void normalize(const EvalParams& params);


}