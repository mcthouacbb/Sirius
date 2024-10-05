#include "misc.h"
#include "comm/move.h"
#include "movegen.h"
#include "move_ordering.h"
#include <chrono>
#include <algorithm>
#include <charconv>
#include <vector>
#include <fstream>

void printBoard(const Board& board)
{
    std::cout << board.stringRep() << std::endl;
    std::cout << "GamePly: " << board.gamePly() << std::endl;
    std::cout << "HalfMoveClock: " << board.halfMoveClock() << std::endl;
    std::cout << "CastlingRights: ";
    if (board.castlingRights().value() != 0)
    {
        if (board.castlingRights().has(CastlingRights::WHITE_KING_SIDE))
            std::cout << 'K';
        if (board.castlingRights().has(CastlingRights::WHITE_QUEEN_SIDE))
            std::cout << 'Q';
        if (board.castlingRights().has(CastlingRights::BLACK_KING_SIDE))
            std::cout << 'k';
        if (board.castlingRights().has(CastlingRights::BLACK_QUEEN_SIDE))
            std::cout << 'q';
    }
    else
    {
        std::cout << '-';
    }
    std::cout << std::endl;
    std::cout << "Side to move: " << (board.sideToMove() == Color::WHITE ? "WHITE" : "BLACK") << std::endl;
    if (board.epSquare() != -1)
        std::cout << "Ep square: " << static_cast<char>((board.epSquare() & 7) + 'a') << static_cast<char>((board.epSquare() >> 3) + '1') << std::endl;
    else
        std::cout << "Ep square: N/A" << std::endl;
    std::cout << "Fen: " <<  board.fenStr() << std::endl;

    std::cout << "Zobrist hash: " << board.zkey().value << std::endl;
    std::cout << "Pawn structure hash: " << board.pawnKey().value << std::endl;
}

template<bool print>
uint64_t perft(Board& board, int depth)
{
    if (depth == 0)
        return 1;
    MoveList moves;
    genMoves<MoveGenType::LEGAL>(board, moves);
    if (depth == 1 && !print)
        return moves.size();

    uint64_t count = 0;

    for (Move move : moves)
    {
        board.makeMove(move);
        uint64_t sub = perft<false>(board, depth - 1);
        if (print)
            std::cout << comm::convMoveToPCN(move) << ": " << sub << std::endl;
        count += sub;
        board.unmakeMove();
    }
    return count;
}

template uint64_t perft<true>(Board& board, int depth);
template uint64_t perft<false>(Board& board, int depth);

void testSAN(Board& board, int depth)
{
    MoveList moves;
    genMoves<MoveGenType::LEGAL>(board, moves);

    for (Move move : moves)
    {
        std::string str = comm::convMoveToSAN(board, moves, move);
        auto find = comm::findMoveFromSAN(board, moves, str.c_str());
        if (find.move != move)
        {
            std::cerr << board.stringRep() << std::endl;
            std::cout << str << ' ' << comm::convMoveToPCN(move) << std::endl;
            std::cerr << "No match " << std::endl;
            exit(1);
        }

        if (find.len != static_cast<int>(str.length()))
        {
            std::cerr << board.stringRep() << std::endl;
            std::cerr << str << ' ' << comm::convMoveToPCN(move) << std::endl;
            std::cerr << "String wrong" << std::endl;
            exit(1);
        }
    }

    if (depth == 0)
        return;

    for (Move move : moves)
    {
        board.makeMove(move);
        testSAN(board, depth - 1);
        board.unmakeMove();
    }
}

void testKeyAfter(Board& board, int depth)
{
    if (depth == 0)
        return;

    MoveList moves;
    genMoves<MoveGenType::LEGAL>(board, moves);

    for (Move move : moves)
    {
        ZKey keyAfter = board.keyAfter(move);
        board.makeMove(move);
        if (keyAfter != board.zkey())
            throw std::runtime_error("key does not match");
        testKeyAfter(board, depth - 1);
        board.unmakeMove();
    }
}

void testNoisyGen(Board& board, int depth)
{
    if (depth == 0)
    {
        MoveList moves;
        genMoves<MoveGenType::NOISY_QUIET>(board, moves);

        MoveList noisies;
        genMoves<MoveGenType::NOISY>(board, noisies);

        int numFound = 0;
        for (Move move : moves)
        {
            bool found = std::find(noisies.begin(), noisies.end(), move) != noisies.end();
            numFound += found;

            if (moveIsQuiet(board, move) && found)
                throw std::runtime_error("Not noisy");
            if (!moveIsQuiet(board, move) && !found)
                throw std::runtime_error("Noisy not found");
        }
        if (numFound != noisies.size())
            throw std::runtime_error("More noisies than should be");
        return;
    }
    MoveList moves;
    genMoves<MoveGenType::LEGAL>(board, moves);

    for (Move move : moves)
    {
        board.makeMove(move);
        testNoisyGen(board, depth - 1);
        board.unmakeMove();
    }
}

void testIsPseudoLegal(Board& board, int depth)
{
    if (depth == 0)
    {
        MoveList moves;
        genMoves<MoveGenType::NOISY_QUIET>(board, moves);

        for (Move move : moves)
        {
            if (!board.isPseudoLegal(move))
            {
                std::cout << board.fenStr() << std::endl;
                std::cout << comm::convMoveToPCN(move) << std::endl;
                throw std::runtime_error("bruh");
            }
        }

        for (int from = 0; from < 64; from++)
        {
            for (int to = 0; to < 64; to++)
            {
                Move move(Square(from), Square(to), MoveType::NONE);
                if (std::find(moves.begin(), moves.end(), move) == moves.end() && board.isPseudoLegal(move))
                    throw std::runtime_error("bruh2");

                move = Move(Square(from), Square(to), MoveType::ENPASSANT);
                if (std::find(moves.begin(), moves.end(), move) == moves.end() && board.isPseudoLegal(move))
                    throw std::runtime_error("bruh2");

                move = Move(Square(from), Square(to), MoveType::CASTLE);
                if (std::find(moves.begin(), moves.end(), move) == moves.end() && board.isPseudoLegal(move))
                    throw std::runtime_error("bruh2");

                move = Move(Square(from), Square(to), MoveType::PROMOTION, Promotion::KNIGHT);
                if (std::find(moves.begin(), moves.end(), move) == moves.end() && board.isPseudoLegal(move))
                    throw std::runtime_error("bruh2");

                move = Move(Square(from), Square(to), MoveType::PROMOTION, Promotion::BISHOP);
                if (std::find(moves.begin(), moves.end(), move) == moves.end() && board.isPseudoLegal(move))
                    throw std::runtime_error("bruh2");

                move = Move(Square(from), Square(to), MoveType::PROMOTION, Promotion::ROOK);
                if (std::find(moves.begin(), moves.end(), move) == moves.end() && board.isPseudoLegal(move))
                    throw std::runtime_error("bruh2");

                move = Move(Square(from), Square(to), MoveType::PROMOTION, Promotion::QUEEN);
                if (std::find(moves.begin(), moves.end(), move) == moves.end() && board.isPseudoLegal(move))
                    throw std::runtime_error("bruh2");
            }
        }
        return;
    }

    MoveList moves;
    genMoves<MoveGenType::LEGAL>(board, moves);

    for (Move move : moves)
    {
        board.makeMove(move);
        testIsPseudoLegal(board, depth - 1);
        board.unmakeMove();
    }
}

void testQuietGen(Board& board, int depth)
{
    if (depth == 0)
    {
        MoveList moves;
        genMoves<MoveGenType::NOISY_QUIET>(board, moves);

        MoveList quiets;
        genMoves<MoveGenType::QUIET>(board, quiets);

        int numFound = 0;
        for (Move move : moves)
        {
            bool found = std::find(quiets.begin(), quiets.end(), move) != quiets.end();
            numFound += found;

            if (moveIsQuiet(board, move) && !found)
                throw std::runtime_error("Not quiet");
            if (!moveIsQuiet(board, move) && found)
                throw std::runtime_error("Quiet not found");
        }
        if (numFound != quiets.size())
            throw std::runtime_error("More Quiets than should be");
        return;
    }
    MoveList moves;
    genMoves<MoveGenType::LEGAL>(board, moves);

    for (Move move : moves)
    {
        board.makeMove(move);
        testQuietGen(board, depth - 1);
        board.unmakeMove();
    }
}

void testSEE()
{
    std::ifstream file("res/see_tests.epd");

    std::string line;
    Board board;
    int failCount = 0;
    int passCount = 0;
    while (std::getline(file, line))
    {
        int sep1 = static_cast<int>(line.find(';', 0));
        board.setToEpd(std::string_view(line).substr(0, sep1));

        int moveStart = sep1 + 2;

        MoveList moves;
        genMoves<MoveGenType::LEGAL>(board, moves);

        auto find = comm::findMoveFromSAN(board, moves, line.c_str() + moveStart);
        if (find.result == comm::MoveStrFind::Result::INVALID)
        {
            throw std::runtime_error("Invalid move");
        }
        if (find.result == comm::MoveStrFind::Result::NOT_FOUND)
        {
            throw std::runtime_error("Move not found");
        }
        if (find.result == comm::MoveStrFind::Result::AMBIGUOUS)
        {
            throw std::runtime_error("Ambiguous move");
        }

        Move move = find.move;
        const char* strEnd = line.c_str() + moveStart + find.len;

        strEnd += 2;

        int value;
        auto [ptr, ec] = std::from_chars(strEnd, line.c_str() + line.size(), value);
        if (ec != std::errc())
        {
            std::cout << "invalid number parsing value" << std::endl;
            return;
        }

        bool fail = board.see(move, value + 1);
        bool pass = board.see(move, value);
        bool failed = false;
        if (fail)
        {
            std::cout << line << " Returned true on value + 1" << std::endl;
            failed = true;
        }
        if (!pass)
        {
            std::cout << line << " Returned false on value" << std::endl;
            failed = true;
        }
        failCount += failed;
        passCount += !failed;
    }
    std::cout << "Failed: " << failCount << std::endl;
    std::cout << "Passed: " << passCount << "/" << (failCount + passCount) << std::endl;
}

struct PerftTest
{
    std::string fen;
    std::array<uint64_t, 6> results;
};

void runTests(Board& board, bool fast)
{
    std::ifstream file("res/perft_tests.txt");
    if (file.is_open())
    {
        std::cout << "open" << std::endl;
    }
    else
    {
        std::cout << "closed" << std::endl;
    }
    std::string line;
    std::vector<PerftTest> tests;
    while (std::getline(file, line))
    {
        PerftTest test;
        std::fill(std::begin(test.results), std::end(test.results), UINT64_MAX);
        int i = 0;
        while (line[i] != ';')
            i++;
        test.fen = line.substr(0, i);

        std::from_chars(line.data() + i + 4, line.data() + line.size(), test.results[line[i + 2] - '1']);

        while (true)
        {
            size_t idx = line.find(';', i + 1);
            if (idx == std::string::npos)
                break;
            i = static_cast<int>(idx);
            std::from_chars(line.data() + i + 4, line.data() + line.size(), test.results[line[i + 2] - '1']);
        }
        tests.push_back(test);
    }

    uint32_t failCount = 0;
    uint64_t totalNodes = 0;

    auto t1 = std::chrono::steady_clock::now();
    for (size_t i = 0; i < tests.size(); i++)
    {
        const auto& test = tests[i];
        board.setToFen(test.fen);
        std::cout << "TEST: " << test.fen << std::endl;
        for (int j = 0; j < 6; j++)
        {
            if (test.results[j] == UINT64_MAX)
                continue;
            if (fast && test.results[j] > 100000000)
            {
                std::cout << "\tSkipped: depth " << j + 1 << std::endl;
                continue;
            }
            uint64_t nodes = perft<false>(board, j + 1);
            totalNodes += nodes;
            if (nodes == test.results[j])
            {
                std::cout << "\tPassed: depth " << j + 1 << std::endl;
            }
            else
            {
                std::cout << "\tFailed: depth " << j + 1 << ", Expected: " << test.results[j] << ", got: " << nodes << std::endl;
                failCount++;
            }
        }
    }
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "Failed: " << failCount << std::endl;
    std::cout << "Nodes: " << totalNodes << std::endl;
    std::cout << "Time: " << (t2 - t1).count() << std::endl;
}

void testSANFind(const Board& board, const MoveList& moveList, int len)
{
    static constexpr std::array<char, 25> chars = {
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
        '1', '2', '3', '4', '5', '6', '7', '8',
        'K', 'Q', 'R', 'B', 'N', 'q', 'r', 'n',
        'x'
    };
    static constexpr uint64_t charCount = 25;
    char* buf = new char[len + 1];
    buf[len] = '\0';

    uint64_t maxIdx = 1;

    for (int i = 0; i < len; i++)
    {
        maxIdx *= charCount;
    }
    for (uint64_t i = 0; i < maxIdx; i++)
    {
        uint64_t tmp = i;
        for (int j = 0; j < len; j++)
        {
            buf[j] = chars[tmp % charCount];
            tmp /= charCount;
        }

        comm::findMoveFromSAN(board, moveList, buf);
    }
    delete[] buf;
}
