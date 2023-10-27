#include <string>

#include "../sirius.h"
#include "uci.h"
#include "fen.h"
#include "move.h"
#include "../eval/eval.h"
#include "../misc.h"
#include "../bench.h"

namespace comm
{

UCI::UCI()
{

}

void UCI::run()
{
    uciCommand();

    while (true)
    {
        std::string command;
        std::getline(std::cin, command);
        if (execCommand(command))
            return;
    }
}

void UCI::reportSearchInfo(const SearchInfo& info) const
{
    auto lock = lockStdout();
    std::cout << "info depth " << info.depth;
    std::cout << " time " << info.time.count();
    std::cout << " nodes " << info.nodes;
    uint64_t nps = info.nodes * 1000ULL / (info.time.count() < 1 ? 1 : info.time.count());
    std::cout << " nps " << nps;
    std::cout << " score ";
    if (isMateScore(info.score))
    {
        if (info.score > 0)
        {
            std::cout << "mate " << ((SCORE_MATE - info.score) + 1) / 2;
        }
        else
        {
            std::cout << "mate -" << (info.score + SCORE_MATE) / 2;
        }
    }
    else
    {
        std::cout << " cp " << info.score;
    }

    std::cout << " pv ";
    for (const Move* move = info.pvBegin; move != info.pvEnd; move++)
    {
        std::cout << comm::convMoveToPCN(*move) << ' ';
    }
    std::cout << std::endl;
}

void UCI::reportBestMove(Move bestMove) const
{
    auto lock = lockStdout();
    std::cout << "bestmove " << comm::convMoveToPCN(bestMove) << std::endl;
}

bool UCI::execCommand(const std::string& command)
{
    std::istringstream stream(command);

    std::string tok;
    stream >> tok;
    Command comm = getCommand(tok);

    switch (comm)
    {
        case Command::INVALID:
            break;
        case Command::UCI:
            uciCommand();
            break;
        case Command::IS_READY:
        {
            auto lock = lockStdout();
            std::cout << "readyok" << std::endl;
            break;
        }
        case Command::NEW_GAME:
            if (!m_Search.searching())
                newGameCommand();
            break;
        case Command::POSITION:
            positionCommand(stream);
            break;
        case Command::GO:
            // cannot be sent while searching as it will block the main thread until the in progress search finishes
            goCommand(stream);
            break;
        case Command::STOP:
            if (m_Search.searching())
                m_Search.stop();
            break;
        case Command::SET_OPTION:
            setOptionCommand(stream);
            break;
        case Command::QUIT:
            return true;
        // non standard commands
        case Command::DBG_PRINT:
        {
            auto lock = lockStdout();
            printBoard(m_Board);
            break;
        }
        case Command::BENCH:
            if (!m_Search.searching())
                benchCommand();
            break;
    }
    return false;
}

UCI::Command UCI::getCommand(const std::string& command) const
{
    if (command == "uci")
        return Command::UCI;
    else if (command == "isready")
        return Command::IS_READY;
    else if (command == "ucinewgame")
        return Command::NEW_GAME;
    else if (command == "position")
        return Command::POSITION;
    else if (command == "go")
        return Command::GO;
    else if (command == "stop")
        return Command::STOP;
    else if (command == "setoption")
        return Command::SET_OPTION;
    else if (command == "quit")
        return Command::QUIT;
    else if (command == "d")
        return Command::DBG_PRINT;
    else if (command == "bench")
        return Command::BENCH;

    return Command::INVALID;
}


void UCI::uciCommand() const
{
    auto lock = lockStdout();
    std::cout << "id name Sirius " << SIRIUS_VERSION_STRING << std::endl;
    std::cout << "id author AspectOfTheNoob" << std::endl;
    std::cout << "option name Hash type spin default 64 min 1 max 2048" << std::endl;
    // lol
    std::cout << "option name Threads type spin default 1 min 1 max 1" << std::endl;
    std::cout << "uciok" << std::endl;
}

void UCI::newGameCommand()
{
    m_Search.newGame();
}

void UCI::positionCommand(std::istringstream& stream)
{
    std::string tok;
    stream >> tok;

    if (tok == "startpos")
    {
        setToFen(Board::defaultFen);
        if (stream)
        {
            stream >> tok;
            if (tok == "moves")
            {
                while (stream.tellg() != -1)
                {
                    stream >> tok;
                    MoveStrFind find = comm::findMoveFromPCN(m_LegalMoves, m_LegalMoves + m_MoveCount, tok.c_str());
                    Move move = *find.move;
                    makeMove(move);
                }
            }
        }
    }
    else if (tok == "fen")
    {
        std::string fen;
        stream >> fen;
        while (stream.tellg() != -1)
        {
            stream >> tok;
            if (tok == "moves")
                break;
            fen += ' ' + tok;
        }

        if (!comm::isValidFen(fen.c_str()))
            return;
        setToFen(fen.c_str());

        if (tok == "moves")
        {
            while (stream.tellg() != -1)
            {
                stream >> tok;
                MoveStrFind find = comm::findMoveFromPCN(m_LegalMoves, m_LegalMoves + m_MoveCount, tok.c_str());
                Move move = *find.move;
                makeMove(move);
            }
        }
    }
    else
        return;
}

void UCI::goCommand(std::istringstream& stream)
{
    std::string tok;
    SearchLimits limits = {};
    limits.maxDepth = 1000;
    while (stream.tellg() != -1)
    {
        stream >> tok;
        if (tok == "wtime")
        {
            int wtime;
            stream >> wtime;
            limits.clock.timeLeft[static_cast<int>(Color::WHITE)] = Duration(wtime);
            limits.clock.enabled = true;
        }
        else if (tok == "btime")
        {
            int btime;
            stream >> btime;
            limits.clock.timeLeft[static_cast<int>(Color::BLACK)] = Duration(btime);
            limits.clock.enabled = true;
        }
        else if (tok == "winc")
        {
            int winc;
            stream >> winc;
            limits.clock.increments[static_cast<int>(Color::WHITE)] = Duration(winc);
            limits.clock.enabled = true;
        }
        else if (tok == "binc")
        {
            int binc;
            stream >> binc;
            limits.clock.increments[static_cast<int>(Color::BLACK)] = Duration(binc);
            limits.clock.enabled = true;
        }
        else if (tok == "movestogo")
        {
            // todo
        }
        else if (tok == "depth")
        {
            int depth;
            stream >> depth;
            limits.maxDepth = depth;
        }
        else if (tok == "nodes")
        {
            // todo
        }
        else if (tok == "mate")
        {
            // todo
        }
        else if (tok == "movetime")
        {
            int time;
            stream >> time;
            limits.maxTime = Duration(time);
        }
        else if (tok == "infinite")
        {

        }
    }

    m_Search.run(limits, m_BoardStates);
}

void UCI::setOptionCommand(std::istringstream& stream)
{
    std::string tok, name;
    stream >> tok;
    if (tok != "name")
        return;
    stream >> name;
    stream >> tok;
    if (tok != "value")
        return;

    if (name == "Hash")
    {
        int mb;
        stream >> mb;
        m_Search.setTTSize(mb);
    }
    else if (name == "Threads")
    {
        // lol
    }
}

void UCI::benchCommand()
{
    runBench(m_Search, BENCH_DEPTH);
}


}
