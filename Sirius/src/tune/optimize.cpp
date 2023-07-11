#include "optimize.h"

namespace tune
{

constexpr int NUM_THREADS = 3;

EvalParams localOptimize(const EvalParams& initial, const std::vector<Pos>& positions, std::ofstream& outFile, int start)
{
	std::vector<ErrorThread> threads(NUM_THREADS);
	double bestError = error(positions, initial, K_VAL, threads);
	EvalParams bestParams = initial;
	bool improved = true;
	int it = start;
	while (improved)
	{
		improved = false;
		for (uint32_t i = 0; i < NUM_PARAMS; i++)
		{
			bool improvedParam = false;
			do
			{
				improvedParam = false;
				EvalParams newParams = bestParams;
				newParams.params[i] += 5;
				double newError = error(positions, newParams, K_VAL, threads);
				std::cout << "Param: " << i << " Delta: 5, New: " << newError << " best: " << bestError << '\n';
				if (newError < bestError)
				{
					bestError = newError;
					bestParams = newParams;
					improved = true;
					improvedParam = true;
					continue;
				}

				newParams.params[i] -= 10;
				newError = error(positions, newParams, K_VAL, threads);
				std::cout << "Param: " << i << " Delta: -5, New: " << newError << " best: " << bestError << '\n';
				if (newError < bestError)
				{
					bestError = newError;
					bestParams = newParams;
					improved = true;
					improvedParam = true;
					continue;
				}

				newParams.params[i] += 4;
				newError = error(positions, newParams, K_VAL, threads);
				std::cout << "Param: " << i << " Delta: -1, New: " << newError << " best: " << bestError << '\n';
				if (newError < bestError)
				{
					bestError = newError;
					bestParams = newParams;
					improved = true;
					improvedParam = true;
					continue;
				}

				newParams.params[i] += 2;
				newError = error(positions, newParams, K_VAL, threads);
				std::cout << "Param: " << i << " Delta: 1, New: " << newError << " best: " << bestError << '\n';
				if (newError < bestError)
				{
					bestError = newError;
					bestParams = newParams;
					improved = true;
					improvedParam = true;
					continue;
				}

				newParams.params[i] -= 1;
			} while (improvedParam);
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

void normalize(const EvalParams& params)
{
	EvalParams paramsCopy = params;
	EvalData& data = paramsCopy.data;

	for (int i = 1; i < 6; i++)
	{
		int mgPsqtSum = 0;
		int egPsqtSum = 0;
		for (int j = 0; j < 32; j++)
		{
			mgPsqtSum += data.psqtMG[i][j];
			egPsqtSum += data.psqtEG[i][j];
		}
		std::cout << "Piece: " << i << " OLD: " << std::endl;
		std::cout << data.materialMG[i] << ' ' << mgPsqtSum << std::endl;
		std::cout << data.materialEG[i] << ' ' << egPsqtSum << std::endl;

		while (mgPsqtSum > 16)
		{
			mgPsqtSum -= 32;
			for (int j = 0; j < 32; j++)
			{
				data.psqtMG[i][j] -= 1;
			}
			data.materialMG[i] += 1;
		}
		while (mgPsqtSum < -16)
		{
			mgPsqtSum += 32;
			for (int j = 0; j < 32; j++)
			{
				data.psqtMG[i][j] += 1;
			}
			data.materialMG[i] -= 1;
		}

		while (egPsqtSum > 31)
		{
			egPsqtSum -= 32;
			for (int j = 0; j < 32; j++)
			{
				data.psqtEG[i][j] -= 1;
			}
			data.materialEG[i] += 1;
		}
		while (egPsqtSum < 0)
		{
			egPsqtSum += 32;
			for (int j = 0; j < 32; j++)
			{
				data.psqtEG[i][j] += 1;
			}
			data.materialEG[i] -= 1;
		}

		std::cout << "Piece: " << i << " NEW: " << std::endl;
		std::cout << data.materialMG[i] << ' ' << mgPsqtSum << std::endl;
		std::cout << data.materialEG[i] << ' ' << egPsqtSum << std::endl;
		std::cout << std::endl;
	}

	printParams(paramsCopy, std::cout);
}


}