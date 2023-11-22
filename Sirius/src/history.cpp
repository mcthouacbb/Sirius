#include "history.h"

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
    return std::min(12 * depth * depth, 1600);
}


void History::clear()
{
    fillHistTable(m_MainHist, 0);
}

int History::getQuietStats(ExtMove move) const
{
    int score = getMainHist(move);
    return score;
}

void History::updateQuietStats(ExtMove move, int bonus)
{
    updateMainHist(move, bonus);
}

int History::getMainHist(ExtMove move) const
{
    return m_MainHist[static_cast<int>(getPieceColor(move.movingPiece()))][move.fromTo()];
}

void History::updateMainHist(ExtMove move, int bonus)
{
    m_MainHist[static_cast<int>(getPieceColor(move.movingPiece()))][move.fromTo()].update(bonus);
}
