#include "extract.h"
#include "viriformat.h"
#include <fstream>

namespace datagen
{

bool filterPos(const Board& board, Move move, int score, marlinformat::WDL wdl)
{
    if (board.checkers().any())
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
            outputFile << board.fenStr() << " | ";
            outputFile << score << "cp | ";
            switch (wdl)
            {
                case marlinformat::WDL::BLACK_WIN:
                    outputFile << "0.0";
                    break;
                case marlinformat::WDL::DRAW:
                    outputFile << "0.5";
                    break;
                case marlinformat::WDL::WHITE_WIN:
                    outputFile << "1.0";
                    break;
            }
            outputFile << '\n';
            board.makeMove(move);
        }
    }
    std::cout << "Finished extracting " << extracted << " fens from " << games.size() << " games"
              << std::endl;
}

}
