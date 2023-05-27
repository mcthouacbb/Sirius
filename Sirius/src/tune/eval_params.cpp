#include "eval_params.h"

namespace tune
{

inline int numDigits(int num)
{
	int abse = std::abs(num);
	int digits = 1;
	while (abse / 10 != 0)
	{
		abse /= 10;
		digits++;
	}
	return digits + (num < 0);
}

void printParams(const EvalParams& params, std::ostream& os)
{
	const EvalData& data = params.data;
	os << "{\n";

	os << "\t{ " << data.phaseWeights[0];
	for (int i = 1; i < 6; i++)
	{
		os << ", " << data.phaseWeights[i];
	}
	os << " },\n";

	os << "\t{ " << data.materialMG[0];
	for (int i = 1; i < 6; i++)
	{
		os << ", " << data.materialMG[i];
	}
	os << " },\n";

	os << "\t{ " << data.materialEG[0];
	for (int i = 1; i < 6; i++)
	{
		os << ", " << data.materialEG[i];
	}
	os << " },\n";

	os << "\t{\n";
	for (int i = 0; i < 6; i++)
	{
		os << "\t\t{\n";
		for (int y = 0; y < 8; y++)
		{
			os << "\t\t\t";
			for (int x = 0; x < 4; x++)
			{
				for (int k = 0; k < 4 - numDigits(data.psqtMG[i][y * 4 + x]); k++)
				{
					os << ' ';
				}
				os << data.psqtMG[i][y * 4 + x] << ", ";
			}
			os << '\n';
		}
		os << "\t\t},\n";
	}
	os << "\t},\n";

	os << "\t{\n";
	for (int i = 0; i < 6; i++)
	{
		os << "\t\t{\n";
		for (int y = 0; y < 8; y++)
		{
			os << "\t\t\t";
			for (int x = 0; x < 4; x++)
			{
				for (int k = 0; k < 4 - numDigits(data.psqtEG[i][y * 4 + x]); k++)
				{
					os << ' ';
				}
				os << data.psqtEG[i][y * 4 + x] << ", ";
			}
			os << '\n';
		}
		os << "\t\t},\n";
	}
	os << "\t},\n";
	os << "}";
	os << std::endl;
}

bool isDegenerate(const EvalParams& params)
{
	const EvalData& data = params.data;
	if (data.phaseWeights[0] < 0 ||
		data.phaseWeights[1] < 0 ||
		data.phaseWeights[2] < 0 ||
		data.phaseWeights[3] < 0 ||
		data.phaseWeights[4] < 0 ||
		data.phaseWeights[5] < 0)
		return true;
	if (data.phaseWeights[0] + data.phaseWeights[1] +
		data.phaseWeights[2] + data.phaseWeights[3] +
		data.phaseWeights[4] + data.phaseWeights[5] <= 0)
		return true;
	return false;
}

EvalCache genCache(const EvalParams& params)
{
	EvalCache cache;
	cache.totalPhase =
		2 * params.data.phaseWeights[1] +
		4 * params.data.phaseWeights[2] +
		4 * params.data.phaseWeights[3] +
		4 * params.data.phaseWeights[4] +
		16 * params.data.phaseWeights[5];
	return cache;
}

inline int psqtIdx(int sq)
{
	int psqtX = sq & 0b111;
	int psqtY = sq & 0b111000;
	if (psqtX > 3)
		psqtX = 7 - psqtX;
	return psqtY / 2 + psqtX;
}

int evaluate(const Board& board, const EvalParams& params, const EvalCache& cache)
{
	constexpr int FLIP_Y = 0b111000;
	const EvalData& data = params.data;
	int phase = cache.totalPhase;
	int matMG[2] = {};
	int matEG[2] = {};
	int psqtMG[2] = {};
	int psqtEG[2] = {};
	for (int sq = 0; sq < 64; sq++)
	{
		Piece piece = board.getPieceAt(sq);
		if (piece)
		{
			int type = piece & PIECE_TYPE_MASK;
			int color = piece >> 3;
			bool isWhite = static_cast<Color>(color) == Color::WHITE;

			if (type != static_cast<int>(PieceType::KING))
			{
				matMG[color] += data.materialMG[type - 1];
				matEG[color] += data.materialEG[type - 1];
				phase -= data.phaseWeights[type - 1];
			}
			psqtMG[color] += data.psqtMG[type - 1][psqtIdx(sq ^ (isWhite ? FLIP_Y : 0))];
			psqtEG[color] += data.psqtEG[type - 1][psqtIdx(sq ^ (isWhite ? FLIP_Y : 0))];
		}
	}

	int evalMG =
		matMG[0] + psqtMG[0] -
		(matMG[1] + psqtMG[1]);

	int evalEG =
		matEG[0] + psqtEG[0] -
		(matEG[1] + psqtEG[1]);

	int phaseFactor = (phase * 256 + cache.totalPhase / 2) / cache.totalPhase;

	return (evalMG * (256 - phaseFactor) + evalEG * phaseFactor) / 256;
}


}