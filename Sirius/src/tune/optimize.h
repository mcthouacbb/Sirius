#pragma once

#include "error.h"
#include <fstream>

namespace tune
{

EvalParams localOptimize(const EvalParams& initial, const std::vector<Pos>& positions, std::ofstream& outFile, int start)
{
	double bestError = error(positions, initial, K_VAL);
	EvalParams bestParams = initial;
	bool improved = true;
	int it = start;
	while (improved)
	{
		improved = false;
		for (int i = 0; i < NUM_PARAMS; i++)
		{
			EvalParams newParams = bestParams;
			newParams.params[i] += 5;
			double newError = error(positions, newParams, K_VAL);
			std::cout << "Delta: 5, New: " << newError << " best: " << bestError << '\n';
			if (newError < bestError)
			{
				bestError = newError;
				bestParams = newParams;
				improved = true;
				continue;
			}

			newParams.params[i] -= 4;
			newError = error(positions, newParams, K_VAL);
			std::cout << "Delta: 1, New: " << newError << " best: " << bestError << '\n';
			if (newError < bestError)
			{
				bestError = newError;
				bestParams = newParams;
				improved = true;
				continue;
			}

			newParams.params[i] -= 6;
			newError = error(positions, newParams, K_VAL);
			std::cout << "Delta: -5, New: " << newError << " best: " << bestError << '\n';
			if (newError < bestError)
			{
				bestError = newError;
				bestParams = newParams;
				improved = true;
				continue;
			}

			newParams.params[i] += 4;
			newError = error(positions, newParams, K_VAL);
			std::cout << "Delta: -1, New: " << newError << " best: " << bestError << '\n';
			if (newError < bestError)
			{
				bestError = newError;
				bestParams = newParams;
				improved = true;
				continue;
			}
		}
		std::cout << "Iteration: " << it << std::endl;
		outFile << "Iteration: " << it++ << std::endl;
		std::cout << "Error: " << bestError << std::endl;
		outFile << "Error: " << bestError << std::endl;
		printParams(bestParams, std::cout);
		printParams(bestParams, outFile);
	}
	return bestParams;
}


}