#include "../sirius.h"
#include "cmdline.h"
#include "fen.h"
#include "move.h"
#include "../eval/eval.h"
#include "../misc.h"
#include "../movegen.h"

#include <fstream>
#include <sstream>

namespace comm
{

CmdLine::CmdLine()
{
    std::cout << "Sirius " << SIRIUS_VERSION_STRING << std::endl;

    std::ifstream openings("res/gaviota_trim.pgn");
    if constexpr (false && openings.is_open())
    {
        std::ostringstream sstr;
        sstr << openings.rdbuf();
        std::string pgnData = sstr.str();

        m_Book.loadFromPGN(pgnData.c_str());
    }
    else
    {
        std::cout << "No opening book loaded" << std::endl;
    }
}

void CmdLine::run()
{
    while (true)
    {
        std::string command;
        std::getline(std::cin, command);
        if (execCommand(command))
            return;
    }
}

void CmdLine::reportSearchInfo(const SearchInfo& info) const
{
    auto lock = lockStdout();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(info.time).count();
    std::cout << "Depth: " << info.depth << '\n';
    std::cout << "\tNodes: " << info.nodes << '\n';
    std::cout << "\tTime Searched: " << time << '\n';
    std::cout << "\tScore: ";
    if (isMateScore(info.score))
    {
        if (info.score < 0)
            std::cout << "Mated in " << info.score + SCORE_MATE << " plies\n";
        else
            std::cout << "Mate in " << SCORE_MATE - info.score << " plies\n";
    }
    else
    {
        std::cout << info.score << '\n';
    }

    std::cout << "PV: ";

    Board board;
    board.setToFen(m_Board.fenStr());

    std::deque<BoardState> states;

    for (const Move* move = info.pvBegin; move != info.pvEnd; move++)
    {
        MoveList moves;
        genMoves<MoveGenType::LEGAL>(board, moves);
        std::cout << comm::convMoveToSAN(board, moves, *move) << ' ';
        states.push_back({});
        board.makeMove(*move);
    }

    std::cout << std::endl;
}

void CmdLine::reportBestMove(Move bestMove) const
{
    auto lock = lockStdout();
    std::cout << "best move: " << comm::convMoveToSAN(m_Board, m_LegalMoves, bestMove) << std::endl;
}

bool CmdLine::execCommand(const std::string& command)
{
    std::istringstream stream(command);

    std::string commandName;

    stream >> commandName;

    Command comm = getCommand(commandName);

    switch (comm)
    {
        case Command::INVALID:
            std::cout << "Invalid Command: " << command << std::endl;
            break;
        case Command::SET_POSITION:
            setPositionCommand(stream);
            break;
        case Command::MAKE_MOVE:
            makeMoveCommand(stream);
            break;
        case Command::UNDO_MOVE:
            undoMoveCommand();
            break;
        case Command::PRINT_BOARD:
            printBoardCommand();
            break;
        case Command::STATIC_EVAL:
            staticEvalCommand();
            break;
        case Command::SEARCH:
            searchCommand(stream);
            break;
        case Command::RUN_TESTS:
            if (!m_Search.searching())
                runTestsCommand();
            break;
        case Command::PERFT:
            if (!m_Search.searching())
                runPerftCommand(stream);
            break;
        case Command::BOOK:
            probeBookCommand();
            break;
        case Command::STOP:
            if (m_Search.searching())
                m_Search.stop();
            break;
        case Command::QUIT:
            return true;
    }
    return false;
}

CmdLine::Command CmdLine::getCommand(const std::string& command) const
{
    if (command == "position")
        return Command::SET_POSITION;
    else if (command == "move")
        return Command::MAKE_MOVE;
    else if (command == "undo")
        return Command::UNDO_MOVE;
    else if (command == "print")
        return Command::PRINT_BOARD;
    else if (command == "eval")
        return Command::STATIC_EVAL;
    else if (command == "search")
        return Command::SEARCH;
    else if (command == "tests")
        return Command::RUN_TESTS;
    else if (command == "perft")
        return Command::PERFT;
    else if (command == "book")
        return Command::BOOK;
    else if (command == "stop")
        return Command::STOP;
    else if (command == "quit")
        return Command::QUIT;

    return Command::INVALID;
}

void CmdLine::setPositionCommand(std::istringstream& stream)
{
    std::string tok;
    stream >> tok;
    if (tok == "fen")
    {
        std::string str = stream.str();
        stream.ignore();
        const char* fen = str.c_str() + stream.tellg();
        std::cout << "fen: " << fen << std::endl;
        if (!comm::isValidFen(fen))
        {
            std::cout << "Invalid fen string" << std::endl;;
            return;
        }
        setToFen(fen);
    }
    else if (tok == "startpos")
    {
        setToFen(Board::defaultFen);
    }
    else
    {
        std::cout << "Invalid position" << std::endl;
    }
}

void CmdLine::makeMoveCommand(std::istringstream& stream)
{
    std::string move;
    stream >> move;
    MoveStrFind find = comm::findMoveFromSAN(m_Board, m_LegalMoves, move.c_str());
    if (find.result == comm::MoveStrFind::Result::INVALID)
    {
        auto lock = lockStdout();
        std::cout << "Invalid move string" << std::endl;
        return;
    }

    if (find.result == comm::MoveStrFind::Result::NOT_FOUND)
    {
        auto lock = lockStdout();
        std::cout << "Move not found" << std::endl;
        return;
    }

    if (find.result == comm::MoveStrFind::Result::AMBIGUOUS)
    {
        auto lock = lockStdout();
        std::cout << "Move is ambiguous" << std::endl;
        return;
    }

    makeMove(find.move);
}

void CmdLine::undoMoveCommand()
{
    if (m_PrevMoves.empty())
    {
        auto lock = lockStdout();
        std::cout << "No moves to undo" << std::endl;
        return;
    }

    unmakeMove();
}

void CmdLine::printBoardCommand()
{
    auto lock = lockStdout();
    printBoard(m_Board);
}

void CmdLine::staticEvalCommand()
{
    auto lock = lockStdout();
    std::cout << "not currently supported" << std::endl;
    //std::cout << "Eval: " << eval::evaluate(m_Board) << std::endl;
    //std::cout << "Phase: " << m_Board.psqtState().phase << std::endl;
    //PackedScore psqt = m_Board.psqtState().evaluate(m_Board);
    //std::cout << "Piece Square Tables: " << psqt.mg() << ' ' << psqt.eg() << std::endl;
}

void CmdLine::searchCommand(std::istringstream& stream)
{
    SearchLimits limits = {};
    limits.maxDepth = 1000;

    std::string tok;
    stream >> tok;
    if (tok == "infinite")
    {

    }
    else if (tok == "clock")
    {
        uint32_t time;
        stream >> time;
        limits.clock.enabled = true;
        limits.clock.timeLeft[static_cast<int>(m_Board.sideToMove())] = Duration(time);
    }
    else if (tok == "depth")
    {
        uint32_t depth;
        stream >> depth;
        limits.maxDepth = depth;
    }
    else if (tok == "time")
    {
        uint32_t millis;
        stream >> millis;
        limits.maxTime = Duration(millis);
    }
    else
    {
        auto lock = lockStdout();
        std::cout << "No search policy specified, defaulting to infinite" << std::endl;
    }

    m_Search.run(limits, m_Board);
}

void CmdLine::runTestsCommand()
{
    runTests(m_Board, false);
}

void CmdLine::runPerftCommand(std::istringstream& stream)
{
    uint32_t depth;
    stream >> depth;

    auto t1 = std::chrono::steady_clock::now();
    uint64_t result = perft<true>(m_Board, depth);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "Nodes: " << result << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::duration<float>>(t2 - t1).count() << std::endl;
}

void CmdLine::probeBookCommand()
{
    auto lock = lockStdout();
    const std::vector<BookEntry>* entries = m_Book.lookup(m_Board.zkey());

    if (!entries)
        std::cout << "No moves in book found" << std::endl;
    else
    {
        for (const auto& entry : *entries)
        {
            std::cout << comm::convMoveToSAN(m_Board, m_LegalMoves, entry.move) << ' ';
        }
        std::cout << std::endl;
    }
}



}
