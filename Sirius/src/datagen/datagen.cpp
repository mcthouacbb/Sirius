#include "datagen.h"

#include "../board.h"
#include "../movegen.h"
#include "../search.h"
#include "viriformat.h"
#include <atomic>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <random>

namespace datagen
{

constexpr u32 BATCH_SIZE = 128;
std::atomic_bool stop = false;

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
        for (i32 i = 0; i < 8; i++)
        {
            MoveList moves;
            genMoves<MoveGenType::LEGAL>(board, moves);
            std::uniform_int_distribution<i32> dist(0, moves.size() - 1);
            i32 idx = dist(gen);
            board.makeMove(moves[idx]);

            if (gameResult(board) != GameResult::NON_TERMINAL)
                goto retry;
        }
        return board;
    retry:;
    }
}

viriformat::Game runGame(std::mt19937& gen, const Config& config)
{
    constexpr i32 WIN_ADJ_THRESHOLD = 2000;
    constexpr i32 WIN_ADJ_PLIES = 5;
    constexpr i32 DRAW_ADJ_MOVE_NUM = 50;
    constexpr i32 DRAW_ADJ_THRESHOLD = 7;
    constexpr i32 DRAW_ADJ_PLIES = 8;
    constexpr i32 MAX_OPENING_SCORE = 300;

    Board startpos = genOpening(gen);
    ColorArray<search::Search> searches = {search::Search(8), search::Search(8)};
    SearchLimits limits = {};
    limits.softNodes = config.softLimit;
    limits.maxNodes = config.hardLimit;
    limits.maxDepth = MAX_PLY;

    Board board = startpos;
    marlinformat::WDL wdl = marlinformat::WDL::DRAW;

    viriformat::Game game = {};

    i32 winPlies = 0;
    i32 drawPlies = 0;
    i32 lossPlies = 0;

    for (;;)
    {
        auto [score, move] = searches[board.sideToMove()].datagenSearch(limits, board);
        if (board.sideToMove() == Color::BLACK)
            score = -score;

        if (game.moves.size() == 0 && score > MAX_OPENING_SCORE)
        {
            searches[board.sideToMove()].newGame();
            board = startpos = genOpening(gen);
            continue;
        }

        if (isMateScore(score))
        {
            wdl = score > 0 ? marlinformat::WDL::WHITE_WIN : marlinformat::WDL::BLACK_WIN;
            break;
        }

        game.moves.push_back({viriformat::ViriMove(move), score});
        board.makeMove(move);

        if (score >= WIN_ADJ_THRESHOLD)
            winPlies++;
        else
            winPlies = 0;

        if (std::abs(score) < DRAW_ADJ_THRESHOLD && game.moves.size() >= DRAW_ADJ_MOVE_NUM * 2)
            drawPlies++;
        else
            drawPlies = 0;

        if (score <= -WIN_ADJ_THRESHOLD)
            lossPlies++;
        else
            lossPlies = 0;

        if (winPlies >= WIN_ADJ_PLIES)
        {
            wdl = marlinformat::WDL::WHITE_WIN;
            break;
        }
        else if (drawPlies >= DRAW_ADJ_PLIES)
        {
            wdl = marlinformat::WDL::DRAW;
            break;
        }
        else if (lossPlies >= WIN_ADJ_PLIES)
        {
            wdl = marlinformat::WDL::BLACK_WIN;
            break;
        }

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

std::string tmpFilename(i32 threadID)
{
    return "datagen_tmp" + std::to_string(threadID) + ".bin";
};

void datagenThread(u32 threadID, const Config& config, u32& gamesLeft, std::mutex& mutex)
{
    std::random_device rd;
    auto seed = rd();
    {
        std::unique_lock<std::mutex> lock(mutex);
        std::cout << "Thread " << threadID << " seed " << seed << std::endl;
    }
    std::mt19937 gen(seed);

    std::ofstream outFile(tmpFilename(threadID), std::ios::binary);

    u32 totalGames = 0;

    while (!stop)
    {
        u32 totalPositions = 0;
        auto startTime = std::chrono::steady_clock::now();

        {
            std::unique_lock<std::mutex> lock(mutex);
            if (stop)
                break;
            if (gamesLeft >= BATCH_SIZE)
                gamesLeft -= BATCH_SIZE;
            else
            {
                stop = true;
                gamesLeft = 0;
                break;
            }
        }

        totalGames += BATCH_SIZE;

        for (i32 i = 0; i < BATCH_SIZE; i++)
        {
            auto game = runGame(gen, config);
            game.write(outFile);

            totalPositions += game.moves.size() + 1;
        }

        auto currTime = std::chrono::steady_clock::now();
        float seconds =
            std::chrono::duration_cast<std::chrono::duration<float>>(currTime - startTime).count();

        std::unique_lock<std::mutex> lock(mutex);
        std::cout << "Thread " << threadID << " wrote " << BATCH_SIZE << " games in the last "
                  << seconds << "s, " << BATCH_SIZE / seconds << " games/s" << std::endl;
        std::cout << "    average positions/game: " << static_cast<float>(totalPositions) / BATCH_SIZE
                  << std::endl;
        std::cout << "    " << totalGames << " total games written" << std::endl;
    }

    std::unique_lock<std::mutex> lock(mutex);
    std::cout << "Thread " << threadID << " finished playing " << totalGames << " games" << std::endl;
}

extern "C" void signalHandler(int sig)
{
    if (sig != SIGINT)
        return;
    stop = true;
}

void runDatagen(Config config)
{
    config.numGames -= config.numGames % BATCH_SIZE;
    std::cout << "Generating " << config.numGames << " games with " << config.numThreads
              << " threads" << std::endl;
    std::cout << "Using " << config.hardLimit << " nodes hard limit and " << config.softLimit
              << " nodes soft limit" << std::endl;

    std::vector<std::thread> threads;
    std::mutex lock;
    stop = false;
    std::signal(SIGINT, signalHandler);

    u32 gamesLeft = config.numGames;

    for (u32 i = 0; i < config.numThreads; i++)
    {
        threads.push_back(std::thread(
            [i, &lock, &gamesLeft, &config]()
            {
                datagenThread(i, config, gamesLeft, lock);
            }));
    }

    for (auto& thread : threads)
        thread.join();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::ofstream outputFile(config.outputFilename, std::ios::binary);

    for (u32 i = 0; i < config.numThreads; i++)
    {
        std::ifstream tmpFile(tmpFilename(i), std::ios::binary);
        if (!tmpFile.is_open())
        {
            std::cout << "Could not open file " << tmpFilename(i) << " for writing to final output"
                      << std::endl;
            break;
        }
        std::cout << "Merging games written from thread " << i << std::endl;
        outputFile << tmpFile.rdbuf();
        std::cout << "Finished merging games written from thread " << i << std::endl;
        tmpFile.close();
        if (!std::filesystem::remove(tmpFilename(i)))
            std::cout << "Could not delete temporary file " << tmpFilename(i) << std::endl;
    }
}

}
