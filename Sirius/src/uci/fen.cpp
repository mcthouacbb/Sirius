#include "fen.h"

namespace uci
{

bool isValidFen(const char* fen)
{
    std::string str(fen);

    usize space1 = str.find(' ', 0);
    if (space1 == std::string::npos)
        return false;

    usize space2 = str.find(' ', space1 + 1);
    if (space2 == std::string::npos)
        return false;

    usize space3 = str.find(' ', space2 + 1);
    if (space3 == std::string::npos)
        return false;

    bool hasCounters = true;

    usize space4 = str.find(' ', space3 + 1);
    if (space4 == std::string::npos)
    {
        hasCounters = false;
        space4 = str.size();
    }

    usize space5 = 0;
    if (hasCounters)
    {
        space5 = str.find(' ', space4 + 1);
        if (space5 == std::string::npos)
            return false;
    }

    std::string_view pieces = std::string_view(str).substr(0, space1);
    std::string_view stm = std::string_view(str).substr(space1 + 1, space2 - space1 - 1);
    std::string_view castle = std::string_view(str).substr(space2 + 1, space3 - space2 - 1);
    std::string_view ep = std::string_view(str).substr(space3 + 1, space4 - space3 - 1);

    if (stm.size() != 1)
        return false;

    if (stm[0] != 'w' && stm[0] != 'b')
        return false;

    if (ep != "-" && ep.size() != 2)
        return false;

    if (ep[0] != '-' && (ep[0] < 'a' || ep[0] > 'h' || ep[1] < '1' || ep[1] > 'h'))
        return false;

    i32 slashCount = 0;
    i32 square = 56;
    i32 whiteKingCount = 0;
    i32 blackKingCount = 0;
    for (char c : pieces)
    {
        switch (c)
        {
            default:
                return false;
            case '/':
                if (square != 64 - slashCount * 8)
                    return false;
                square -= 16;
                slashCount++;
                if (slashCount > 7)
                    return false;
                break;
            case 'K':
                whiteKingCount++;
                if (whiteKingCount > 1)
                    return false;
                square++;
                break;
            case 'k':
                blackKingCount++;
                if (blackKingCount > 1)
                    return false;
                square++;
                break;
            case 'P':
            case 'p':
            case 'N':
            case 'n':
            case 'B':
            case 'b':
            case 'R':
            case 'r':
            case 'Q':
            case 'q':
                square++;
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                square += c - '0';
                break;
        }
    }

    if (castle != "-")
    {
        if (castle.size() == 0)
            return false;
        for (char c : castle)
        {
            c = std::tolower(c);
            if (c != 'k' && c != 'q' && !(c >= 'a' && c <= 'h'))
            {
                return false;
            }
        }
    }

    if (square != 8)
        return false;

    if (hasCounters)
    {
        std::string_view hmc = std::string_view(str).substr(space4 + 1, space5 - space4 - 1);
        std::string_view fmc = std::string_view(str).substr(space5 + 1, str.size() - space5 - 1);

        switch (hmc.size())
        {
            case 1:
                if (hmc[0] < '0' || hmc[0] > '9')
                    return false;
                break;
            case 2:
                if (hmc[0] < '0' || hmc[0] > '9' || hmc[1] < '0' || hmc[1] > '9')
                    return false;
                break;
            case 3:
                if (hmc[0] < '0' || hmc[0] > '9' || hmc[1] < '0' || hmc[1] > '9' || hmc[2] < '0'
                    || hmc[2] > '9')
                    return false;
                break;
            default:
                return false;
        }
        if (fmc.size() < 1)
            return false;
        for (char c : fmc)
        {
            if (c < '0' || c > '9')
                return false;
        }
    }

    // TODO: Check position legality

    return true;
}

}
