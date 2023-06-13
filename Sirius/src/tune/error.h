#pragma once

#include "quiescence.h"
#include "pos.h"
#include <vector>

namespace tune
{

inline double sigmoid(double s, double k)
{
	return 1.0 / (1 + std::pow(10, -k * s / static_cast<double>(NUM_PARAMS)));
}

double error(const std::vector<Pos>& positions, const EvalParams& params, double kValue);


}