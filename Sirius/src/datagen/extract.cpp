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

f32 phaseScaleFactor(i32 phase)
{
    f32 oddScale = std::min(0.9f, -0.09793221f * (static_cast<f32>(phase) - 23.0f) + 0.189393939f);
    f32 p16 = std::abs(static_cast<f32>(phase) - 16.0f);
    f32 base =
        phase > 16 ? 1 - 0.875 * std::pow(p16 / 8.0f, 2) : 1 - 0.95 * std::pow(p16 / 16.0f, 2.0f);
    if (phase % 2 == 1 && phase > 12)
        return oddScale * base;
    return base;
}

}

i32 boardPhase(const Board& board)
{
    i32 phase = 4 * board.pieces(PieceType::QUEEN).popcount()
        + 2 * board.pieces(PieceType::ROOK).popcount()
        + (board.pieces(PieceType::BISHOP) | board.pieces(PieceType::KNIGHT)).popcount();
    return std::min(phase, 24);
}

bool filterPos(const Board& board, Move move, i32 score, marlinformat::WDL wdl)
{
    if (board.checkers().any())
        return true;
    if (!moveIsQuiet(board, move))
        return true;
    return false;
}

bool dropPosition(f32 keepProb, std::mt19937& gen)
{
    std::uniform_real_distribution<f32> dist(0.0, 1.0);
    f32 u = dist(gen);
    return u >= keepProb;
}

void extract(std::string dataFilename, std::string outputFilename, u32 maxGames, u32 ppg)
{
    std::random_device rd;
    auto seed = rd();
    std::mt19937 gen(seed);
    std::cout << "Using seed " << seed << std::endl;

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

    struct Position
    {
        marlinformat::PackedBoard board;
        i32 phase;
    };
    std::vector<Position> positions;
    for (auto game : games)
    {
        std::vector<Position> currPositions;
        auto [board, score, wdl] = marlinformat::unpackBoard(game.startpos);
        for (auto [viriMove, score] : game.moves)
        {
            Move move = viriMove.toMove();
            if (filterPos(board, move, score, wdl))
            {
                board.makeMove(move);
                continue;
            }

            marlinformat::PackedBoard packedBoard = marlinformat::packBoard(board, score, wdl);
            currPositions.push_back({packedBoard, boardPhase(board)});
            board.makeMove(move);
        }

        std::sample(currPositions.begin(), currPositions.end(), std::back_inserter(positions), ppg, gen);
    }

    std::cout << "Sampled " << positions.size() << " positions from " << games.size() << " games"
              << std::endl;
    std::cout << "Adjusting distribution" << std::endl;

    u32 phaseCounts[25] = {};
    for (const auto& pos : positions)
        phaseCounts[pos.phase]++;

    f32 phaseKeepProbs[25] = {};
    f32 phaseNormConst = 100.0f;
    for (i32 i = 0; i <= 24; i++)
    {
        f32 observed = static_cast<f32>(phaseCounts[i]) / static_cast<f32>(positions.size());
        // this is not actually the desired probability, but it still works
        f32 desired = dists::phaseScaleFactor(i);
        phaseKeepProbs[i] = desired / observed;
        phaseNormConst = std::min(phaseNormConst, observed / desired);
    }

    for (i32 i = 0; i <= 24; i++)
        phaseKeepProbs[i] = std::min(phaseKeepProbs[i] * phaseNormConst, 1.0f);

    for (i32 i = 0; i < positions.size(); i++)
    {
        const auto& pos = positions[i];
        if (dropPosition(phaseKeepProbs[pos.phase], gen))
        {
            positions[i] = std::move(positions.back());
            positions.pop_back();
            i--;
        }
    }

    std::cout << "Shuffling" << std::endl;
    std::shuffle(positions.begin(), positions.end(), gen);
    std::cout << "Writing to output file" << std::endl;
    for (const auto& pos : positions)
    {
        auto [board, score, wdl] = marlinformat::unpackBoard(pos.board);
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
        outputFile << ss.str();
    }
    outputFile.flush();

    std::cout << "Finished extracting " << positions.size() << " fens from " << games.size()
              << " games" << std::endl;
}

}
