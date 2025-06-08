#pragma once

#include "../board.h"
#include "../movegen.h"
#include "../search.h"
#include "../time_man.h"
#include "uci_option.h"


#include <sstream>
#include <unordered_map>

namespace uci
{

class UCI
{
public:
    UCI();
    ~UCI() = default;

    enum class Command
    {
        INVALID,
        UCI,
        IS_READY,
        NEW_GAME,
        POSITION,
        GO,
        STOP,
        SET_OPTION,
        QUIT,

        DBG_PRINT,
        PERFT,
        RUN_PERFT_TESTS,
        EVAL,
        BENCH
    };

    void run(std::string cmd);
    void reportSearchInfo(const SearchInfo& info) const;
    void reportBestMove(Move bestMove) const;

private:
    std::unique_lock<std::mutex> lockStdout() const;
    void calcLegalMoves();
    void setToFen(const char* fen, bool frc = false);
    void makeMove(Move move);

    void prettyPrintSearchInfo(const SearchInfo& info) const;
    void printUCISearchInfo(const SearchInfo& info) const;

    bool execCommand(const std::string& command);
    Command getCommand(const std::string& command) const;

    void uciCommand() const;
    void newGameCommand();
    void positionCommand(std::istringstream& stream);
    void goCommand(std::istringstream& stream);
    void setOptionCommand(std::istringstream& stream);
    void evalCommand();
    void perftCommand(std::istringstream& stream);
    void benchCommand();

    mutable std::mutex m_StdoutMutex;
    Board m_Board;
    MoveList m_LegalMoves;
    search::Search m_Search;

    std::unordered_map<std::string, UCIOption> m_Options;
};

extern UCI* uci;

}
