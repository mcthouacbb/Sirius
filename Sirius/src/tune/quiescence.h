#pragma once

#include "eval_params.h"

namespace tune
{

int qsearch(Board& board, const EvalParams& params, const EvalCache& cache, int& nodes, int alpha, int beta);


}