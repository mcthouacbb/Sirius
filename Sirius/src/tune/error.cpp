#include "error.h"
#include "quiescence.h"

#include <chrono>

namespace tune
{

double error(const std::vector<Pos>& positions, const EvalParams& params, double kValue)
{
	if (isDegenerate(params))
		return 1000000.0;
	double result = 0.0;
	Board board;
	EvalCache cache = genCache(params);

	int maxNodes = 0;
	int totalNodes = 0;
	std::string_view maxEpd;
	for (const auto& pos : positions)
	{
		board.setToEpd(std::string_view(pos.epd, pos.epdLen));
		int nodes = 0;
		int eval = evaluate(board, params, cache);//qsearch(board, params, cache, nodes, -200000, 200000);
		totalNodes += nodes;
		if (nodes > maxNodes)
		{
			maxNodes = nodes;
			maxEpd = std::string_view(pos.epd, pos.epdLen);
		}
		double term = pos.result - sigmoid(eval, kValue);
		// std::cout << term << std::endl;

		result += term * term;
	}
	//std::cout << maxNodes << std::endl;
	//std::cout << maxEpd << std::endl;
	//std::cout << totalNodes << std::endl;
	//std::cout << result << std::endl;
	//std::cout << positions.size() << std::endl;
	result /= static_cast<double>(positions.size());
	return result;
}


}