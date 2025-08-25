#include "extract.h"
#include "../move_ordering.h"
#include "viriformat.h"

#include <algorithm>
#include <fstream>
#include <random>
#include <sstream>

namespace datagen
{

bool filterPos(const Board& board, Move move, int score, marlinformat::WDL wdl)
{
    if (board.checkers().any())
        return true;
    if (!moveIsQuiet(board, move))
        return true;
    return false;
}

void extract(std::string dataFilename, std::string outputFilename)
{
    std::ifstream inputFile(dataFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::app);
    std::vector<viriformat::Game> games;

    if (!inputFile.is_open())
        std::cout << "Could not open file " << dataFilename << std::endl;

    while (inputFile.peek() != EOF)
    {
        auto game = viriformat::Game::read(inputFile);
        games.push_back(game);
    }

    std::cout << "Finished loading " << games.size() << " games from " << dataFilename << std::endl;

    std::vector<std::string> lines;
    uint32_t extracted = 0;
    for (auto game : games)
    {
        auto [board, score, wdl] = marlinformat::unpackBoard(game.startpos);
        for (auto [viriMove, score] : game.moves)
        {
            Move move = viriMove.toMove();
            if (filterPos(board, move, score, wdl))
            {
                board.makeMove(move);
                continue;
            }

            extracted++;
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
            lines.push_back(ss.str());
            board.makeMove(move);
        }
    }
    std::cout << "Shuffling" << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(lines.begin(), lines.end(), gen);
    std::cout << "Writing to output file" << std::endl;
    for (const auto& line : lines)
        outputFile << line;
    outputFile.flush();

    std::cout << "Finished extracting " << extracted << " fens from " << games.size() << " games"
              << std::endl;
}

}
