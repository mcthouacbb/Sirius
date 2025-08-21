#include "datagen.h"

#include "../board.h"
#include "../movegen.h"
#include "../search.h"
#include "viriformat.h"
#include <fstream>
#include <random>

namespace datagen
{

enum class GameResult
{
    MATED,
    DRAW,
    NON_TERMINAL
};

GameResult gameResult(const Board& board)
{
    MoveList moves;
    genMoves<MoveGenType::LEGAL>(board, moves);
    if (moves.size() == 0)
    {
        if (board.checkers().any())
            return GameResult::MATED;
        else
            return GameResult::DRAW;
    }

    if (board.isDraw(0))
        return GameResult::DRAW;

    return GameResult::NON_TERMINAL;
}

Board genOpening(std::mt19937& gen)
{
    for (;;)
    {
        Board board;
        for (int i = 0; i < 8; i++)
        {
            MoveList moves;
            genMoves<MoveGenType::LEGAL>(board, moves);
            std::uniform_int_distribution<int> dist(0, moves.size() - 1);
            int idx = dist(gen);
            board.makeMove(moves[idx]);

            if (gameResult(board) != GameResult::NON_TERMINAL)
                goto retry;
        }
        return board;
    retry:;
    }
}

viriformat::Game runGame(std::mt19937& gen)
{
    Board startpos = genOpening(gen);
    ColorArray<search::Search> searches = {search::Search(8), search::Search(8)};
    SearchLimits limits = {};
    limits.softNodes = 5000;
    limits.maxDepth = MAX_PLY;

    Board board = startpos;
    marlinformat::WDL wdl = marlinformat::WDL::DRAW;

    viriformat::Game game = {};

    for (;;)
    {
        auto [score, move] = searches[board.sideToMove()].datagenSearch(limits, board);
        if (board.sideToMove() == Color::BLACK)
            score = -score;
        game.moves.push_back({viriformat::ViriMove(move), score});
        board.makeMove(move);

        GameResult result = gameResult(board);
        if (result != GameResult::NON_TERMINAL)
        {
            if (result == GameResult::MATED)
            {
                if (board.sideToMove() == Color::WHITE)
                    wdl = marlinformat::WDL::BLACK_WIN;
                else
                    wdl = marlinformat::WDL::WHITE_WIN;
            }
            break;
        }
    }

    game.startpos = marlinformat::packBoard(startpos, 0, wdl);
    return game;
}

void runDatagen(uint32_t threadID, std::string filename, std::mutex& coutLock)
{
    std::random_device rd;
    auto seed = rd();
    std::cout << "Thread " << threadID << " seed " << seed << std::endl;
    std::mt19937 gen(seed);

    std::ofstream outFile(filename, std::ios::binary);

    auto startTime = std::chrono::steady_clock::now();
    auto prevTime = startTime;
    uint32_t totalGames = 0;
    uint32_t totalPositions = 0;

    for (;;)
    {
        auto game = runGame(gen);
        game.write(outFile);

        totalGames++;
        totalPositions += game.moves.size() + 1;
        if (totalGames % 128 == 0)
        {
            auto currTime = std::chrono::steady_clock::now();
            float seconds =
                std::chrono::duration_cast<std::chrono::duration<float>>(currTime - prevTime).count();
            std::unique_lock<std::mutex> lock(coutLock);
            std::cout << "Thread " << threadID << " wrote " << totalGames << " total games" << std::endl;
            std::cout << "    128 games in the last " << seconds << "s, " << 128.0 / seconds
                      << " games/s" << std::endl;
            std::cout << "    average positions/game: "
                      << static_cast<float>(totalPositions) / static_cast<float>(totalGames)
                      << std::endl;

            prevTime = currTime;
        }
    }
}

}
