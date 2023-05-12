#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <cmath>
#include <cstring>
#include <cstdlib>

struct EvalData
{
	int phaseWeights[6];
	int materialMG[6];
	int materialEG[6];
	int psqtMG[6][32];
	int psqtEG[6][32];
};

constexpr size_t NUM_PARAMS = sizeof(EvalData) / sizeof(int);
constexpr double K_VAL = 0.96;

constexpr int PIECE_KING = 0;
constexpr int PIECE_QUEEN = 1;
constexpr int PIECE_ROOK = 2;
constexpr int PIECE_BISHOP = 3;
constexpr int PIECE_KNIGHT = 4;
constexpr int PIECE_PAWN = 5;

union EvalParams
{
	EvalData data;
	int params[NUM_PARAMS];
};

struct Pos
{
	const char* epd;
	int epdLen;
	double result;
};

int psqtIdx(int sq)
{
	int psqtX = sq & 0b111;
	int psqtY = sq & 0b111000;
	if (psqtX > 3)
		psqtX = 7 - psqtX;
	return psqtY / 2 + psqtX;
}

int evaluate(const char* epd, const EvalParams& params)
{
	constexpr int FLIP_Y = 0b111000;
	const EvalData& data = params.data;
	int totalPhase =
		data.phaseWeights[PIECE_QUEEN] * 2 +
		data.phaseWeights[PIECE_ROOK] * 4 +
		data.phaseWeights[PIECE_BISHOP] * 4 +
		data.phaseWeights[PIECE_KNIGHT] * 4 +
		data.phaseWeights[PIECE_PAWN] * 16;
	
	int sq = 56;
	int i = 0;
	
	int phase = totalPhase;
	
	int whiteMatMG = 0;
	int blackMatMG = 0;
	
	int whiteMatEG = 0;
	int blackMatEG = 0;
	
	int whitePsqtMG = 0;
	int blackPsqtMG = 0;
	
	int whitePsqtEG = 0;
	int blackPsqtEG = 0;
	
	while (epd[i] != ' ')
	{
		switch (epd[i++])
		{
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				sq += epd[i - 1] - '0';
				break;
			case 'k':
				blackPsqtMG += data.psqtMG[PIECE_KING][psqtIdx(sq)];
				blackPsqtEG += data.psqtEG[PIECE_KING][psqtIdx(sq)];
				sq++;
				// addPiece(sq++, Color::BLACK, PieceType::KING);
				break;
			case 'q':
				phase -= data.phaseWeights[PIECE_QUEEN];
				blackMatMG += data.materialMG[PIECE_QUEEN];
				blackMatEG += data.materialEG[PIECE_QUEEN];
				blackPsqtMG += data.psqtMG[PIECE_QUEEN][psqtIdx(sq)];
				blackPsqtEG += data.psqtEG[PIECE_QUEEN][psqtIdx(sq)];
				sq++;
				// addPiece(sq++, Color::BLACK, PieceType::QUEEN);
				break;
			case 'r':
				phase -= data.phaseWeights[PIECE_ROOK];
				blackMatMG += data.materialMG[PIECE_ROOK];
				blackMatEG += data.materialEG[PIECE_ROOK];
				blackPsqtMG += data.psqtMG[PIECE_ROOK][psqtIdx(sq)];
				blackPsqtEG += data.psqtEG[PIECE_ROOK][psqtIdx(sq)];
				sq++;
				// addPiece(sq++, Color::BLACK, PieceType::ROOK);
				break;
			case 'b':
				phase -= data.phaseWeights[PIECE_BISHOP];
				blackMatMG += data.materialMG[PIECE_BISHOP];
				blackMatEG += data.materialEG[PIECE_BISHOP];
				blackPsqtMG += data.psqtMG[PIECE_BISHOP][psqtIdx(sq)];
				blackPsqtEG += data.psqtEG[PIECE_BISHOP][psqtIdx(sq)];
				sq++;
				// addPiece(sq++, Color::BLACK, PieceType::BISHOP);
				break;
			case 'n':
				phase -= data.phaseWeights[PIECE_KNIGHT];
				blackMatMG += data.materialMG[PIECE_KNIGHT];
				blackMatEG += data.materialEG[PIECE_KNIGHT];
				blackPsqtMG += data.psqtMG[PIECE_KNIGHT][psqtIdx(sq)];
				blackPsqtEG += data.psqtEG[PIECE_KNIGHT][psqtIdx(sq)];
				sq++;
				// addPiece(sq++, Color::BLACK, PieceType::KNIGHT);
				break;
			case 'p':
				phase -= data.phaseWeights[PIECE_PAWN];
				blackMatMG += data.materialMG[PIECE_PAWN];
				blackMatEG += data.materialEG[PIECE_PAWN];
				blackPsqtMG += data.psqtMG[PIECE_PAWN][psqtIdx(sq)];
				blackPsqtEG += data.psqtEG[PIECE_PAWN][psqtIdx(sq)];
				sq++;
				// addPiece(sq++, Color::BLACK, PieceType::PAWN);
				break;
			case 'K':
				whitePsqtMG += data.psqtMG[PIECE_KING][psqtIdx(sq ^ FLIP_Y)];
				whitePsqtEG += data.psqtEG[PIECE_KING][psqtIdx(sq ^ FLIP_Y)];
				sq++;
				// addPiece(sq++, Color::WHITE, PieceType::KING);
				break;
			case 'Q':
				phase -= data.phaseWeights[PIECE_QUEEN];
				whiteMatMG += data.materialMG[PIECE_QUEEN];
				whiteMatEG += data.materialEG[PIECE_QUEEN];
				whitePsqtMG += data.psqtMG[PIECE_QUEEN][psqtIdx(sq ^ FLIP_Y)];
				whitePsqtEG += data.psqtEG[PIECE_QUEEN][psqtIdx(sq ^ FLIP_Y)];
				sq++;
				// addPiece(sq++, Color::WHITE, PieceType::QUEEN);
				break;
			case 'R':
				phase -= data.phaseWeights[PIECE_ROOK];
				whiteMatMG += data.materialMG[PIECE_ROOK];
				whiteMatEG += data.materialEG[PIECE_ROOK];
				whitePsqtMG += data.psqtMG[PIECE_ROOK][psqtIdx(sq ^ FLIP_Y)];
				whitePsqtEG += data.psqtEG[PIECE_ROOK][psqtIdx(sq ^ FLIP_Y)];
				sq++;
				// addPiece(sq++, Color::WHITE, PieceType::ROOK);
				break;
			case 'B':
				phase -= data.phaseWeights[PIECE_BISHOP];
				whiteMatMG += data.materialMG[PIECE_BISHOP];
				whiteMatEG += data.materialEG[PIECE_BISHOP];
				whitePsqtMG += data.psqtMG[PIECE_BISHOP][psqtIdx(sq ^ FLIP_Y)];
				whitePsqtEG += data.psqtEG[PIECE_BISHOP][psqtIdx(sq ^ FLIP_Y)];
				sq++;
				// addPiece(sq++, Color::WHITE, PieceType::BISHOP);
				break;
			case 'N':
				phase -= data.phaseWeights[PIECE_KNIGHT];
				whiteMatMG += data.materialMG[PIECE_KNIGHT];
				whiteMatEG += data.materialEG[PIECE_KNIGHT];
				whitePsqtMG += data.psqtMG[PIECE_KNIGHT][psqtIdx(sq ^ FLIP_Y)];
				whitePsqtEG += data.psqtEG[PIECE_KNIGHT][psqtIdx(sq ^ FLIP_Y)];
				sq++;
				// addPiece(sq++, Color::WHITE, PieceType::KNIGHT);
				break;
			case 'P':
				phase -= data.phaseWeights[PIECE_PAWN];
				whiteMatMG += data.materialMG[PIECE_PAWN];
				whiteMatEG += data.materialEG[PIECE_PAWN];
				whitePsqtMG += data.psqtMG[PIECE_PAWN][psqtIdx(sq ^ FLIP_Y)];
				whitePsqtEG += data.psqtEG[PIECE_PAWN][psqtIdx(sq ^ FLIP_Y)];
				sq++;
				// addPiece(sq++, Col	or::WHITE, PieceType::PAWN);
				break;
			case '/':
				sq -= 16;
				break;
		}
	}

	int evalMG = whiteMatMG + whitePsqtMG - (blackMatMG + blackPsqtMG);
	int evalEG = whiteMatEG + whitePsqtEG - (blackMatEG + blackPsqtEG);
	int phaseFactor = (phase * 256 + totalPhase / 2) / totalPhase;
	return (evalMG * (256 - phaseFactor) + evalEG * phaseFactor) / 256;
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

double sigmoid(double s, double k)
{
	return 1.0 / (1 + std::pow(10, -k * s / static_cast<double>(NUM_PARAMS)));
}

double error(const std::vector<Pos>& positions, const EvalParams& params, double kValue)
{
	if (isDegenerate(params))
		return 1000000.0;
	double result = 0.0;
	for (const auto& pos : positions)
	{
		int eval = evaluate(pos.epd, params);
		double term = pos.result - sigmoid(eval, kValue);
		// std::cout << term << std::endl;
		result += term * term;
	}
	// std::cout << result << std::endl;
	// std::cout << static_cast<double>(positions.size()) << std::endl;
	result /= static_cast<double>(positions.size());
	return result;
}

std::vector<Pos> parseEpdFile(const std::string& str)
{
	std::vector<Pos> positions;
	
	int lineStart = 0;
	int lineEnd = str.find('\n', 0);

	do
	{
		if (str[lineStart] == '\0')
			break;
		double result;
		int lastChar = lineEnd == -1 ? str.size() - 1 : lineEnd - 1;
		int len = lineEnd - lineStart;
		if (str[lastChar] == '0')
		{
			result = 1;
			len -= 3;
		}
		else if (str[lastChar] == '2')
		{
			result = 0.5;
			len -= 7;
		}
		else if (str[lastChar] == '1')
		{
			result = 0;
			len -= 3;
		}
		else
		{
			std::cout << str[lastChar] << std::endl;
			std::cout << lineEnd << std::endl;
			std::cout.write(&str[lastChar - 10], 50) << std::endl;
			throw std::runtime_error("Why?");
		}
		positions.push_back({str.c_str() + lineStart, len, result});
		lineStart = lineEnd + 1;
		lineEnd = str.find('\n', lineStart);
	}
	while (lineEnd != std::string::npos);

	return positions;
}

int numDigits(int num)
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

/*EvalParams defaultParams = {
	{
		// phase weights
		{0, 4, 2, 1, 1, 0},
		// material middlegame
		{0, 900, 500, 330, 320, 100},
		// material endgame
		{0, 900, 500, 330, 320, 100},
		// psqt middlegame
		{
			// king
			{
				-30,-40,-40,-50,
				-30,-40,-40,-50,
				-30,-40,-40,-50,
				-30,-40,-40,-50,
				-20,-30,-30,-40,
				-10,-20,-20,-20,
				 20, 20,  0,  0,
				 20, 30, 10,  0
			},
			// queen
			{
				-20,-10,-10, -5,
				-10,  0,  0,  0,
				-10,  0,  5,  5,
				 -5,  0,  5,  5,
				  0,  0,  5,  5,
				-10,  5,  5,  5,
				-10,  0,  5,  0,
				-20,-10,-10, -5
			},
			// rook
			{
				  0,  0,  0,  0,
				  5, 10, 10, 10,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				  0,  0,  0,  5
			},
			// bishop
			{
				-20,-10,-10,-10,
				-10,  0,  0,  0,
				-10,  0,  5, 10,
				-10,  5,  5, 10,
				-10,  0, 10, 10,
				-10, 10, 10, 10,
				-10,  5,  0,  0,
				-20,-10,-10,-10
			},
			// knight
			{
				-50,-40,-30,-30,
				-40,-20,  0,  0,
				-30,  0, 10, 15,
				-30,  5, 15, 20,
				-30,  0, 15, 20,
				-30,  5, 10, 15,
				-40,-20,  0,  5,
				-50,-40,-30,-30
			},
			// pawn
			{
				 0,  0,  0,  0,
				50, 50, 50, 50,
				10, 10, 20, 30,
				 5,  5, 10, 25,
				 0,  0,  0, 20,
				 5, -5,-10,  0,
				 5, 10, 10,-20,
				 0,  0,  0,  0
			}
		},
		// psqt endgame
		{
			// king
			{
				-50,-40,-30,-20,
				-30,-20,-10,  0,
				-30,-10, 20, 30,
				-30,-10, 30, 40,
				-30,-10, 30, 40,
				-30,-10, 20, 30,
				-30,-30,  0,  0,
				-50,-30,-30,-30
			},
			// queen
			{
				-20,-10,-10, -5,
				-10,  0,  0,  0,
				-10,  0,  5,  5,
				 -5,  0,  5,  5,
				  0,  0,  5,  5,
				-10,  5,  5,  5,
				-10,  0,  5,  0,
				-20,-10,-10, -5
			},
			// rook
			{
				  0,  0,  0,  0,
				  5, 10, 10, 10,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				 -5,  0,  0,  0,
				  0,  0,  0,  5
			},
			// bishop
			{
				-20,-10,-10,-10,
				-10,  0,  0,  0,
				-10,  0,  5, 10,
				-10,  5,  5, 10,
				-10,  0, 10, 10,
				-10, 10, 10, 10,
				-10,  5,  0,  0,
				-20,-10,-10,-10
			},
			// knight
			{
				-50,-40,-30,-30,
				-40,-20,  0,  0,
				-30,  0, 10, 15,
				-30,  5, 15, 20,
				-30,  0, 15, 20,
				-30,  5, 10, 15,
				-40,-20,  0,  5,
				-50,-40,-30,-30
			},
			// pawn
			{
				 0,  0,  0,  0,
				50, 50, 50, 50,
				10, 10, 20, 30,
				 5,  5, 10, 25,
				 0,  0,  0, 20,
				 5, -5,-10,  0,
				 5, 10, 10,-20,
				 0,  0,  0,  0
			}
		}
	}
};*/

EvalParams defaultParams =  {
	{
		{ 0, 27, 7, 1, 2, 0 },
		{ 0, 960, 405, 245, 225, 59 },
		{ 0, 921, 552, 321, 282, 149 },
		{
			{
				  51,   55, -135,   45, 
				-125, -115,  -23, -113, 
				 -99,    3,   55,  -66, 
				-125,  -26,  -94,  -34, 
				   0, -105,   55, -103, 
				 -74,   -2,   17,  -34, 
				  60,   51,    4,  -65, 
				  33,   53,  -14,   16, 
			},
			{
				  75,   85,   17,  -48, 
				  51,   18,   12,   75, 
				   4,   57,   74,    6, 
				 -24,  -26,  -10,   28, 
				  29,   14,   -8,   12, 
				  75,    6,    7,    3, 
				 -73,  -23,   16,   -9, 
				-115, -100,  -62,   -8, 
			},
			{
				   8,   90,   -1,    0, 
				  -3,   -8,   38,    9, 
				 -52,   95,   84,   95, 
				 -52,   -3,  -16,   41, 
				  -7,  -26,    1,  -45, 
				   6,  -27,   38,  -27, 
				-100,  -69,  -54,  -80, 
				 -47,  -24,  -18,  -36, 
			},
			{
				  75,  -87, -105,  -45, 
				 -93,  -31,  -65,  -60, 
				  51,    5,  -90,   15, 
				 -59,  -28,   19,  -22, 
				 -47,  -53,  -22,   -8, 
				 -41,   -4,   -7,  -16, 
				 -87,   -9,    3,  -41, 
				-115,   85,  -43,  -84, 
			},
			{
				-145,   31,   65,  -75, 
				  41,  -83,  -59,  -89, 
				  14,  -95,  -85,  -13, 
				  49,  -36,   34,  -18, 
				   1,    7,   11,  -28, 
				-115,   37,  -28,   20, 
				  55,   15,  -95,  -21, 
				 -68,  -52,   61,  -84, 
			},
			{
				   0,    0,    0,    0, 
				 140,   81,  145,   95, 
				 -43,  -38,   32,   26, 
				  27,   21,  -16,    9, 
				 -24,   -1,   -7,   17, 
				  -7,   11,   -4,   -7, 
				 -21,   30,   -2,  -16, 
				   0,    0,    0,    0, 
			},
		},
		{
			{
				 -81,    6,  -43,   23, 
				 -59,   29,   66,    0, 
				  18,   36,   24,    1, 
				 -14,   -4,    5,   17, 
				  10,    2,    4,   29, 
				 -39,  -22,   -3,    5, 
				 -22,  -27,   -2,  -13, 
				 -61,  -51,  -28,  -37, 
			},
			{
				  29,   61,   70,   60, 
				 -37,   34,   95,   17, 
				 -39,   67,   55,   95, 
				  90,   95,   81,  100, 
				 -77,    4,  100,   14, 
				 -73,  -40,   13,   27, 
				  38,   20,  -47,   95, 
				 -88, -105,  -44, -100, 
			},
			{
				  45,    9,   66,   61, 
				  27,   21,   43,   27, 
				  33,    2,   20,    2, 
				  28,    0,   39,    3, 
				 -20,   15,  -12,   21, 
				  11,   29,  -28,   26, 
				   1,  -26,  -44,   -4, 
				 -39,  -31,  -10,  -11, 
			},
			{
				 -18,   -3,   46,   13, 
				  70,  -20,  -15,   20, 
				   9,   -3,   22,   66, 
				 -20,  -29,   16,   31, 
				 -31,   59,   15,   48, 
				 -28,   20,   10,   17, 
				 -22,  -55,    6,   19, 
				  34,  -95,  -50,    1, 
			},
			{
				-139,   31,   30,  -38, 
				-115,   12,  -30,   36, 
				  -3,  -61,   81,   15, 
				 -36,   17,   -4,    3, 
				  18,   23,   -6,   43, 
				-110,   -2,  -23,  -16, 
				-114, -115,  -80,   -2, 
				 -15,  -42,  -46,  -52, 
			},
			{
				   0,    0,    0,    0, 
				 135,  114,  139,  145, 
				  32,   61,   69,   67, 
				  -7,  -13,   15,    3, 
				  -9,    5,  -10,   12, 
				  -4,   -2,  -28,   26, 
				  -5,   -8,  -14,  -23, 
				   0,    0,    0,    0, 
			},
		},
	}
};

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "Please arguments thx" << std::endl;
		return 0;
	}
	
	if (strcmp(argv[1], "evalpos") == 0)
	{
		const char* epd = argv[2];
		std::cout << evaluate(epd, defaultParams) << std::endl;
		return 0;
	}
	
	if (argc < 4)
	{
		std::cout << "Please arguments thx" << std::endl;
		return 0;
	}

	if (strcmp(argv[1], "optimize") == 0)
	{
		std::ifstream epdFile(argv[2]);
		
		std::string epdFileData(std::istreambuf_iterator<char>{epdFile}, std::istreambuf_iterator<char>());

		std::vector<Pos> positions = parseEpdFile(epdFileData);

		std::ofstream outFile(argv[3], std::ios::app);

		int start = argc >= 5 ? strtol(argv[4], nullptr, 10) : 0;

		if (!outFile.is_open())
		{
			std::cout << "Out file does not exist" << std::endl;
			return 0;
		}

		EvalParams params = localOptimize(defaultParams, positions, outFile, start);
		std::cout << std::endl;
		return 0;
	}
	
	if (strcmp(argv[1], "error") == 0)
	{
		std::ifstream epdFile(argv[2]);
		double k = std::strtod(argv[3], nullptr);
		std::string epdFileData(std::istreambuf_iterator<char>{epdFile}, std::istreambuf_iterator<char>());

		std::vector<Pos> positions = parseEpdFile(epdFileData);

		std::cout << error(positions, defaultParams, k) << std::endl;
		return 0;
	}
	
	if (strcmp(argv[1], "checkevals") == 0)
	{
		std::string epd = argv[2];
		std::string evals = argv[3];
		std::ifstream epdFile(epd);
		std::ifstream evalFile(evals);
		std::cout << epdFile.is_open() << ' ' << evalFile.is_open() << std::endl;

		std::string epdFileData(std::istreambuf_iterator<char>{epdFile}, std::istreambuf_iterator<char>());

		// std::cout << epdFileData << std::endl;
		
		std::vector<Pos> positions = parseEpdFile(epdFileData);

		int siriusEval;
		int passed = 0;
		for (int i = 0; i < positions.size(); i++)
		{
			int epdEval = evaluate(positions[i].epd, defaultParams);
			evalFile >> siriusEval;
			if (siriusEval != epdEval)
			{
				std::cout << "FAILED: " << i + 1 << std::endl;
				std::cout << "EXPECTED: " << siriusEval << " GOT: " << epdEval << std::endl;
				(std::cout << "POSITION: ").write(positions[i].epd, positions[i].epdLen) << std::endl;
			}
			else
			{
				passed++;
			}
			if (i % 1000 == 0)
				std::cout << "PASSED: " << passed << '/' << i + 1 << std::endl;
		}
		std::cout << "PASSED:" << passed << std::endl;
		return 0;
	}
}