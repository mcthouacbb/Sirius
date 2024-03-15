#include "history.h"
#include "search_params.h"

namespace
{

template<int MAX_VAL, size_t N>
void fillHistTable(std::array<HistoryEntry<MAX_VAL>, N>& arr, int value)
{
    std::fill(arr.begin(), arr.end(), value);
}

template<typename T, size_t N>
void fillHistTable(std::array<T, N>& arr, int value)
{
    for (auto& elem : arr)
    {
        fillHistTable(elem, value);
    }
}

}

int historyBonus(int depth)
{
    // formula from berserk
    return std::min(search::maxHistBonus, search::histScaleQuadratic * depth * depth + search::histScaleLinear * depth - search::histBonusOffset);
}


void History::clear()
{
    fillHistTable(m_MainHist, 0);
    fillHistTable(m_ContHist, 0);
}

int History::getQuietStats(ExtMove move, std::span<const CHEntry* const> contHistEntries) const
{
    int score = getMainHist(move);
    for (auto entry : contHistEntries)
        if (entry)
            score += getContHist(entry, move);
    return score;
}

int History::getNoisyStats(ExtMove move) const
{
    return getCaptHist(move);
}

void History::updateQuietStats(ExtMove move, std::span<CHEntry*> contHistEntries, int bonus)
{
    updateMainHist(move, bonus);
    for (auto entry : contHistEntries)
        if (entry)
            updateContHist(entry, move, bonus);
}

void History::updateNoisyStats(ExtMove move, int bonus)
{
    updateCaptHist(move, bonus);
}

int History::getMainHist(ExtMove move) const
{
    return m_MainHist[static_cast<int>(getPieceColor(move.movingPiece()))][move.fromTo()];
}

int History::getContHist(const CHEntry* entry, ExtMove move) const
{
    return (*entry)[static_cast<int>(move.movingPiece())][move.dstPos()];
}

int History::getCaptHist(ExtMove move) const
{
    return m_CaptHist[static_cast<int>(move.capturedPiece())][static_cast<int>(move.movingPiece())][move.dstPos()];
}

void History::updateMainHist(ExtMove move, int bonus)
{
    m_MainHist[static_cast<int>(getPieceColor(move.movingPiece()))][move.fromTo()].update(bonus);
}

void History::updateContHist(CHEntry* entry, ExtMove move, int bonus)
{
    (*entry)[static_cast<int>(move.movingPiece())][move.dstPos()].update(bonus);
}

void History::updateCaptHist(ExtMove move, int bonus)
{
    m_CaptHist[static_cast<int>(move.capturedPiece())][static_cast<int>(move.movingPiece())][move.dstPos()].update(bonus);
}
