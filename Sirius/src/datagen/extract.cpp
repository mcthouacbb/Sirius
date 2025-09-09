#include "extract.h"
#include "../move_ordering.h"
#include "viriformat.h"

#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>

namespace datagen
{

namespace dists
{

constexpr float phaseScaleFactor(int phase)
{
    float oddScale = std::min(0.9f, -0.09793221f * (static_cast<float>(phase) - 23.0f) + 0.189393939f);
    float p16sqr = (static_cast<float>(phase) - 16) * (static_cast<float>(phase) - 16);
    float base = phase > 16 ? 1 - p16sqr / 73.1428571 : 1 - p16sqr / 269.473684;
    if (phase % 2 == 1 && phase > 12)
        return oddScale * base;
    return base;
}

}

int boardPhase(const Board& board)
{
    int phase = 4 * board.pieces(PieceType::QUEEN).popcount()
        + 2 * board.pieces(PieceType::ROOK).popcount()
        + (board.pieces(PieceType::BISHOP) | board.pieces(PieceType::KNIGHT)).popcount();
    return std::min(phase, 24);
}

bool filterPos(const Board& board, Move move, int score, marlinformat::WDL wdl)
{
    if (board.checkers().any())
        return true;
    if (!moveIsQuiet(board, move))
        return true;
    return false;
}

bool dropPosition(float keepProb, std::mt19937& gen)
{
    std::uniform_real_distribution<float> dist(0.0, 1.0);
    float u = dist(gen);
    return u >= keepProb;
}

void extract(std::string dataFilename, std::string outputFilename, uint32_t maxGames, uint32_t ppg)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::ifstream inputFile(dataFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::app);
    std::vector<viriformat::Game> games;

    if (!inputFile.is_open())
        std::cout << "Could not open file " << dataFilename << std::endl;

    while (inputFile.peek() != EOF && games.size() < maxGames)
    {
        auto game = viriformat::Game::read(inputFile);
        games.push_back(game);
    }

    std::cout << "Finished loading " << games.size() << " games from " << dataFilename << std::endl;
    std::cout << "Sampling a maximum of " << ppg << " positions per game" << std::endl;

    struct LineEntry
    {
        std::string line;
        int phase;
    };
    std::vector<LineEntry> lines;
    for (auto game : games)
    {
        std::vector<LineEntry> positionLines;
        auto [board, score, wdl] = marlinformat::unpackBoard(game.startpos);
        for (auto [viriMove, score] : game.moves)
        {
            Move move = viriMove.toMove();
            if (filterPos(board, move, score, wdl))
            {
                board.makeMove(move);
                continue;
            }

            std::stringstream ss;
            ss << board.fenStr() << " | ";
            ss << score << "cp | ";
            switch (wdl)
            {
                case marlinformat::WDL::BLACK_WIN:
                    ss << "0.0";
                    break;
                case marlinformat::WDL::DRAW:
                    ss << "0.5";
                    break;
                case marlinformat::WDL::WHITE_WIN:
                    ss << "1.0";
                    break;
            }
            ss << '\n';
            positionLines.push_back({ss.str(), boardPhase(board)});
            board.makeMove(move);
        }

        std::sample(positionLines.begin(), positionLines.end(), std::back_inserter(lines), ppg, gen);
    }

    std::cout << "Sampled " << lines.size() << " positions from " << games.size() << " games"
              << std::endl;
    std::cout << "Adjusting distribution" << std::endl;

    uint32_t phaseCounts[25] = {};
    for (const auto& lineEntry : lines)
        phaseCounts[lineEntry.phase]++;

    float phaseKeepProbs[25] = {};
    float phaseNormConst = 100.0f;
    for (int i = 0; i <= 24; i++)
    {
        float observed = static_cast<float>(phaseCounts[i]) / static_cast<float>(lines.size());
        // this is not actually the desired probability, but it still works
        float desired = dists::phaseScaleFactor(i);
        phaseKeepProbs[i] = desired / observed;
        phaseNormConst = std::min(phaseNormConst, observed / desired);
    }

    for (int i = 0; i <= 24; i++)
        phaseKeepProbs[i] = std::min(phaseKeepProbs[i] * phaseNormConst, 1.0f);

    for (int i = 0; i < lines.size(); i++)
    {
        const auto& line = lines[i];
        if (dropPosition(phaseKeepProbs[line.phase], gen))
        {
            lines[i] = std::move(lines.back());
            lines.pop_back();
            i--;
        }
    }

    std::cout << "Shuffling" << std::endl;
    std::shuffle(lines.begin(), lines.end(), gen);
    std::cout << "Writing to output file" << std::endl;
    for (const auto& lineEntry : lines)
        outputFile << lineEntry.line;
    outputFile.flush();

    std::cout << "Finished extracting " << lines.size() << " fens from " << games.size() << " games"
              << std::endl;
}

}
