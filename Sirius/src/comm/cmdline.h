#pragma once

#include "icomm.h"
#include "book.h"

namespace comm
{

class CmdLine : public IComm
{
public:
    CmdLine();
    virtual ~CmdLine() override = default;

    enum class Command
    {
        INVALID,
        SET_POSITION,
        MAKE_MOVE,
        UNDO_MOVE,
        PRINT_BOARD,
        STATIC_EVAL,
        SEARCH,
        RUN_TESTS,
        PERFT,
        BOOK,
        STOP,
        QUIT
    };

    void run();
    virtual void reportSearchInfo(const SearchInfo& info) const override;
    virtual void reportBestMove(Move bestMove) const override;
private:
    bool execCommand(const std::string& command);
    Command getCommand(const std::string& command) const;

    void setPositionCommand(std::istringstream& stream);
    void makeMoveCommand(std::istringstream& stream);
    void undoMoveCommand();
    void printBoardCommand();
    void staticEvalCommand();
    void searchCommand(std::istringstream& stream);
    void runTestsCommand();
    void runPerftCommand(std::istringstream& stream);
    void probeBookCommand();

    Book m_Book;
};

}
