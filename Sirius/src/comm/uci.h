#pragma once

#include "icomm.h"
#include "uci_option.h"

#include <sstream>
#include <unordered_map>

namespace comm
{

class UCI : public IComm
{
public:
    UCI();
    virtual ~UCI() override = default;

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
        BENCH
    };


    virtual void run() override;
    virtual void reportSearchInfo(const SearchInfo& info) const override;
    virtual void reportBestMove(Move bestMove) const override;
private:
    bool execCommand(const std::string& command);
    Command getCommand(const std::string& command) const;

    void uciCommand() const;
    void newGameCommand();
    void positionCommand(std::istringstream& stream);
    void goCommand(std::istringstream& stream);
    void setOptionCommand(std::istringstream& stream);
    void benchCommand();

    std::unordered_map<std::string, UCIOption> m_Options;
};


}
