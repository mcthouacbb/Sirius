#include <string>
#include <algorithm>
#include <optional>

#include "../sirius.h"
#include "uci.h"
#include "fen.h"
#include "move.h"
#include "../eval/eval.h"
#include "../misc.h"
#include "../bench.h"

#include "../search_params.h"

namespace comm
{

UCI::UCI()
{
    const auto& hashCallback = [this](const UCIOption& option)
    {
        m_Search.setTTSize(static_cast<int>(option.intValue()));
    };
    const auto& threadsCallback = [this](const UCIOption& option)
    {
        m_Search.setThreads(static_cast<int>(option.intValue()));
    };
    m_Options = {
        {"Hash", UCIOption("Hash", {64, 64, 1, 65536}, hashCallback)},
        {"Threads", UCIOption("Threads", {1, 1, 1, 256}, threadsCallback)},
        {"MoveOverhead", UCIOption("MoveOverhead", {10, 10, 1, 100})},
        {"PrettyPrint", UCIOption("PrettyPrint", UCIOption::BoolData{true})}
    };
#ifdef EXTERNAL_TUNE
    for (auto& param : search::searchParams())
    {
        m_Options.insert({param.name, UCIOption(param.name, {param.value, param.defaultValue, param.min, param.max}, [&param](const UCIOption& option)
        {
            param.value = static_cast<int>(option.intValue());
            if (param.callback)
                param.callback();
        })});
    }
#endif
}

void UCI::run(std::string cmd)
{
    if (cmd == "uci")
        m_Options.at("PrettyPrint").setBoolValue(false);

    if (execCommand(cmd))
        return;

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
    if (m_Options.at("PrettyPrint").boolValue())
        prettyPrintSearchInfo(info);
    else
        printUCISearchInfo(info);
}

void UCI::prettyPrintSearchInfo(const SearchInfo& info) const
{
    std::cout << "  ";
    // depth/seldepth
    std::cout
        << std::right << std::setw(3) << std::setfill(' ') << info.depth
        << "/" << std::left << std::setw(3) << std::setfill(' ') << info.selDepth;
    std::cout << "  ";

    // time
    if (info.time < Duration(1000))
    {
        std::cout << "    " << std::right << std::setw(3) << std::setfill(' ') << info.time.count() << "ms";
    }
    else
    {
        std::cout
            << std::right << std::setw(4) << std::setfill(' ') << info.time.count() / 1000 << '.'
            << std::right << std::setw(2) << std::setfill('0') << info.time.count() / 10 % 100 << "s ";
    }
    std::cout << "  ";

    // nodes and nps
    std::cout << std::right << std::setw(7) << std::setfill(' ') << info.nodes / 1000 << "kn  ";
    uint64_t nps = info.nodes * 1000ULL / (info.time.count() < 1 ? 1 : info.time.count());
    std::cout << std::right << std::setw(7) << std::setfill(' ') << nps / 1000 << "kn/s";
    std::cout << "  ";

    // score
    std::cout << std::right << std::setw(5) << std::setfill(' ') << info.score << "cp  ";

    Board board;
    board.setToFen(m_Board.fenStr());

    for (const Move* move = info.pvBegin; move != info.pvEnd; move++)
    {
        MoveList moves;
        genMoves<MoveGenType::LEGAL>(board, moves);
        std::cout << comm::convMoveToSAN(board, moves, *move) << ' ';
        board.makeMove(*move);
    }

    std::cout << std::endl;
}

void UCI::printUCISearchInfo(const SearchInfo& info) const
{
    std::cout << "info depth " << info.depth;
    std::cout << " seldepth " << info.selDepth;
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
        std::cout << "cp " << info.score;
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
        case Command::PERFT:
            perftCommand(stream);
            break;
        case Command::RUN_PERFT_TESTS:
        {
            auto lock = lockStdout();
            runTests(m_Board, false);
            break;
        }
        case Command::EVAL:
            evalCommand();
            break;
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
    else if (command == "perft")
        return Command::PERFT;
    else if (command == "perfttests")
        return Command::RUN_PERFT_TESTS;
    else if (command == "eval")
        return Command::EVAL;
    else if (command == "bench")
        return Command::BENCH;

    return Command::INVALID;
}


void UCI::uciCommand() const
{
    auto lock = lockStdout();
    std::cout << "id name Sirius " << SIRIUS_VERSION_STRING << std::endl;
    std::cout << "id author mcthouacbb" << std::endl;
    for (const auto& option : m_Options)
    {
        std::cout << "option name " << option.first << " type ";
        switch (option.second.type())
        {
            case UCIOption::Type::INT:
            {
                std::cout << "spin default " << option.second.intData().defaultValue
                    << " min " << option.second.intData().minValue
                    << " max " << option.second.intData().maxValue
                    << std::endl;
                break;
            }
            case UCIOption::Type::BOOL:
            {
                std::cout << "check default " << std::boolalpha << option.second.boolValue()
                    << std::endl;
                break;
            }
            default:
                break;
        }
    }
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
                    MoveStrFind find = comm::findMoveFromPCN(m_LegalMoves, tok.c_str());
                    Move move = find.move;
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
                MoveStrFind find = comm::findMoveFromPCN(m_LegalMoves, tok.c_str());
                Move move = find.move;
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
    limits.overhead = Duration(m_Options["MoveOverhead"].intValue());
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
            int nodes;
            stream >> nodes;
            limits.maxNodes = nodes;
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
    m_Search.run(limits, m_Board);
}

// cursed function
int parseBool(std::string str)
{
    for (char& c : str)
        c = std::tolower(c);
    if (str == "true")
        return 1;
    if (str == "false")
        return 0;
    return -1;
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

    auto& option = m_Options[name];
    switch (option.type())
    {
        case UCIOption::Type::INT:
        {
            int value;
            stream >> value;
            option.setIntValue(value);
            break;
        }
        case UCIOption::Type::BOOL:
        {
            std::string str;
            stream >> str;

            int value = parseBool(str);
            if (value != -1)
                option.setBoolValue(static_cast<bool>(value));
        }
        default:
            break;
    }
}

void UCI::perftCommand(std::istringstream& stream)
{
    auto lock = lockStdout();
    uint32_t depth;
    stream >> depth;

    auto t1 = std::chrono::steady_clock::now();
    uint64_t result = perft<true>(m_Board, depth);
    auto t2 = std::chrono::steady_clock::now();
    std::cout << "Nodes: " << result << std::endl;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::duration<float>>(t2 - t1).count() << std::endl;
}

void UCI::evalCommand()
{
    auto lock = lockStdout();
    if (m_Board.checkers().any())
    {
        std::cout << "static eval: none(in check)" << std::endl;
        return;
    }
    int staticEval = eval::evaluateSingle(m_Board);
    std::cout << "static eval: " << staticEval << "cp" << std::endl;
}

void UCI::benchCommand()
{
    runBench(m_Search, BENCH_DEPTH);
}


}
